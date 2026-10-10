#!/usr/bin/env bash
#
# Boot the OS3 ISO headlessly, wait for the compositor, push synthetic mouse
# movement at it over QMP and check that the pointer followed.
#
# What it actually proves:
#   - the compositor started and presented a frame
#   - IRQ 12 reaches the kernel and the packets decode into movement
#   - the pointer moves in the same direction the mouse was pushed
#   - the cursor pixels really are where the kernel says the pointer is
#   - button presses reach the kernel and show up on screen
#   - damage tracking repaints what the cursor moved off, leaving no trail
#   - a lone packet is composited without waiting for the one after it
#   - the pointer clamps to the screen edges and the arrow clips cleanly
#   - the framebuffer still matches the composed scene byte for byte
#
# Artifacts land in $OUT (default /tmp/os3-gui-test):
#   before.png    the desktop as first composited
#   after.png     the desktop once the pointer has been moved
#   pressed.png   with the left button held down
#   released.png  with it let go again
#   back.png      with the pointer moved back where it came from
#   nudge.png     after a single isolated movement packet
#   corner-br.png pointer pinned in the bottom-right corner
#   corner-tl.png pointer pinned in the top-left corner
#   serial.log  everything the kernel wrote through kprintf
#   qemu.log    QEMU's own stderr
#
# Exit status: 0 all checks passed, 1 a check failed, 2 bad usage.

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

OUT="${OS3_GUI_TEST_OUT:-/tmp/os3-gui-test}"
ISO="$ROOT/os.iso"
DISK="$ROOT/hdd.bin"		# attached with snapshot=on, see below
QEMU="${QEMU:-qemu-system-x86_64}"
MARKER="${OS3_GUI_MARKER:-Compositor: running}"
WAIT="${OS3_BOOT_WAIT:-}"
SETTLE="${OS3_BOOT_SETTLE:-}"
BUILD=0

usage() {
	cat <<'USAGE'
usage: tools/gui-test.sh [options]

  -o, --out DIR      where to write artifacts (default /tmp/os3-gui-test)
  -w, --wait SEC     seconds to wait for the compositor (default 25)
  -s, --settle SEC   extra seconds after the marker before the first shot
  -i, --iso PATH     ISO to boot (default ./os.iso)
  -b, --build        run "make iso" first
  -h, --help         this message
USAGE
}

while [ $# -gt 0 ]; do
	case "$1" in
	-o|--out)	OUT="$2"; shift 2;;
	-w|--wait)	WAIT="$2"; shift 2;;
	-s|--settle)	SETTLE="$2"; shift 2;;
	-i|--iso)	ISO="$2"; shift 2;;
	-b|--build)	BUILD=1; shift;;
	-h|--help)	usage; exit 0;;
	*)		echo "gui-test: unknown option $1" >&2; usage >&2; exit 2;;
	esac
done

mkdir -p "$OUT"

if [ "$BUILD" -eq 1 ]; then
	echo "== building"
	if ! make -C "$ROOT" iso >"$OUT/build.log" 2>&1; then
		tail -30 "$OUT/build.log" >&2
		echo "gui-test: build failed, see $OUT/build.log" >&2
		exit 1
	fi
fi

[ -f "$ISO" ] || { echo "gui-test: no ISO at $ISO (run 'make iso')" >&2; exit 1; }

SOCK="$OUT/qmp.sock"
SERIAL="$OUT/serial.log"
rm -f "$SOCK" "$SERIAL" "$OUT"/*.png "$OUT"/*.ppm "$OUT/qemu.log"

# KVM where available, plain TCG otherwise. Set OS3_NO_KVM=1 to force TCG,
# which is what a CI runner without /dev/kvm will be doing.
#
# The TCG CPU has to be "max": QEMU's default qemu64 model has no PDPE1GB, and
# the kernel checks for 1GiB pages before it has any way to tell you about it -
# boot.asm's error path writes to the VGA text buffer, which is invisible once
# GRUB has set a linear framebuffer. The symptom is a black screen and an
# empty serial log.
ACCEL=(-machine q35 -cpu max)
SLOW=1

if [ -w /dev/kvm ] && [ "${OS3_NO_KVM:-0}" != "1" ]; then
	ACCEL=(-machine q35,accel=kvm -cpu host)
	SLOW=0
fi

# Emulation is roughly an order of magnitude slower, so anything not asked
# for explicitly gets more room
if [ "$SLOW" -eq 1 ]; then
	: "${WAIT:=180}"
	: "${SETTLE:=6}"
else
	: "${WAIT:=25}"
	: "${SETTLE:=2}"
fi

export OS3_SLOW="$SLOW"

# snapshot=on: guest writes go to a throwaway overlay. hdd.bin is tracked in
# git, so a test must not be able to dirty it - and a shared read-only open is
# what lets two harness runs (or a CI matrix) use the image at the same time.
"$QEMU" \
	-m 2G "${ACCEL[@]}" \
	-vga std -display none \
	-no-reboot -no-shutdown \
	-drive id=disk,file="$DISK",if=none,format=raw,snapshot=on \
	-device ahci,id=ahci \
	-device ide-hd,drive=disk,bus=ahci.0 \
	-serial "file:$SERIAL" \
	-qmp "unix:$SOCK,server,nowait" \
	-cdrom "$ISO" \
	>"$OUT/qemu.log" 2>&1 &
QPID=$!

cleanup() {
	kill -TERM "$QPID" 2>/dev/null
	for _ in 1 2 3 4 5 6 7 8 9 10; do
		kill -0 "$QPID" 2>/dev/null || return
		sleep 0.2
	done
	kill -KILL "$QPID" 2>/dev/null
}
trap cleanup EXIT

for _ in $(seq 1 50); do
	[ -S "$SOCK" ] && break
	kill -0 "$QPID" 2>/dev/null || { echo "gui-test: QEMU died on startup" >&2; tail -20 "$OUT/qemu.log" >&2; exit 1; }
	sleep 0.2
done
[ -S "$SOCK" ] || { echo "gui-test: QMP socket never appeared" >&2; exit 1; }

echo "== waiting for: $MARKER"
BOOTED=0
DEADLINE=$(( $(date +%s) + WAIT ))
while [ "$(date +%s)" -lt "$DEADLINE" ]; do
	if [ -f "$SERIAL" ] && grep -qF "$MARKER" "$SERIAL" 2>/dev/null; then
		BOOTED=1
		break
	fi
	kill -0 "$QPID" 2>/dev/null || break
	sleep 0.3
done

if [ "$BOOTED" -ne 1 ]; then
	if kill -0 "$QPID" 2>/dev/null; then
		echo "== compositor never announced itself within ${WAIT}s" >&2
	else
		echo "== QEMU exited before the compositor started" >&2
	fi

	# QEMU's own complaint is usually the real story - a held image lock,
	# an unsupported CPU model - and it is not in the serial log
	if [ -s "$OUT/qemu.log" ]; then
		echo
		echo "== qemu said:"
		sed 's/^/  /' "$OUT/qemu.log"
	fi

	echo
	echo "== serial (last 25 lines)"
	if [ -s "$SERIAL" ]; then
		tail -25 "$SERIAL"
	else
		echo "  (empty - the kernel never reached serial_init)"
	fi
	exit 1
fi
echo "== compositor up"

sleep "$SETTLE"

# -u so the checks appear as they run: block-buffered stdout otherwise hides
# all progress until the very end, which is painful on a slow TCG run
OUT="$OUT" SERIAL="$SERIAL" python3 -u - <<'PY'
import json, os, re, socket, sys, time

out = os.environ["OUT"]
serial_path = os.environ["SERIAL"]
sock_path = os.path.join(out, "qmp.sock")

# How far to push the mouse, and in how many steps. Small steps keep every
# packet inside the 9-bit range a PS/2 mouse can report.
STEPS, STEP_X, STEP_Y = 40, 5, 3
CURSOR_W = 12

# Without KVM the guest takes far longer to drain its packets and compose the
# frames, so every pause the checks depend on gets stretched
SLOW = os.environ.get("OS3_SLOW", "0") == "1"
PAUSE = 10 if SLOW else 2
SHORT = 5 if SLOW else 1

failures = []


def check(ok, msg):
    print(("   ok   " if ok else "   FAIL ") + msg)
    if not ok:
        failures.append(msg)


s = socket.socket(socket.AF_UNIX)
s.settimeout(20)
try:
    s.connect(sock_path)
except OSError as e:
    print(f"gui-test: cannot reach QMP: {e}", file=sys.stderr)
    sys.exit(1)

f = s.makefile("rw")
f.readline()                                    # greeting


def cmd(execute, **args):
    f.write(json.dumps({"execute": execute, "arguments": args}) + "\n")
    f.flush()
    while True:
        line = f.readline()
        if not line:
            return {"error": {"desc": "monitor closed"}}
        msg = json.loads(line)
        if "return" in msg or "error" in msg:
            return msg                          # skip async events


def screendump(name):
    png = os.path.join(out, name)
    res = cmd("screendump", filename=png, format="png")
    if "error" in res:
        ppm = png.replace(".png", ".ppm")
        res = cmd("screendump", filename=ppm)
        if "error" in res:
            return None
        try:
            from PIL import Image
            Image.open(ppm).save(png)
        except Exception:
            return None
    return png


def pointer_log():
    """Every 'gui: pointer X,Y' the kernel has reported so far."""
    try:
        with open(serial_path, "r", errors="replace") as fh:
            text = fh.read()
    except OSError:
        return []
    return [(int(a), int(b))
            for a, b in re.findall(r"gui: pointer (-?\d+),(-?\d+)", text)]


cmd("qmp_capabilities")

state = cmd("query-status").get("return", {}).get("status", "unknown")
print(f"== vm status: {state}")

before = screendump("before.png")
start = pointer_log()
start_pos = start[-1] if start else None

print(f"== pointer before: {start_pos}")
print(f"== pushing the mouse {STEPS} x ({STEP_X},{STEP_Y})")

for _ in range(STEPS):
    res = cmd("input-send-event", events=[
        {"type": "rel", "data": {"axis": "x", "value": STEP_X}},
        {"type": "rel", "data": {"axis": "y", "value": STEP_Y}},
    ])
    if "error" in res:
        print(f"gui-test: input-send-event failed: {res['error']}",
              file=sys.stderr)
        sys.exit(1)

# Give the guest time to take the interrupts and composite the frames
time.sleep(PAUSE)

after = screendump("after.png")
moves = pointer_log()
end_pos = moves[-1] if moves else None

print(f"== pointer after:  {end_pos}")
print(f"== pointer reports: {len(moves)}")
print()
print("== checks")

check(state == "running", f"guest is running (was '{state}')")
check(before is not None and after is not None, "both screenshots captured")

new_reports = len(moves) - len(start)
check(new_reports > 0,
      f"kernel saw mouse movement ({new_reports} pointer reports after injection)")

if start_pos and end_pos:
    dx = end_pos[0] - start_pos[0]
    dy = end_pos[1] - start_pos[1]
    check(dx > 0, f"pointer moved right (dx={dx})")
    check(dy > 0, f"pointer moved down (dy={dy})")
else:
    check(False, "kernel reported pointer positions")

OUTLINE = (0x10, 0x12, 0x18)
FILL = (0xF2, 0xF4, 0xF8)
FILL_PRESSED = (0x5B, 0xC8, 0xFA)

pil = True
try:
    from PIL import Image
except ImportError:
    pil = False
    print("   skip  pixel checks (no PIL)")

if before and after and pil:
    b = Image.open(before).convert("RGB")
    a = Image.open(after).convert("RGB")

    check(b.tobytes() != a.tobytes(), "the composited screen changed")

    if end_pos:
        x, y = end_pos

        # The arrow's tip is its hotspot, so the reported position is
        # exactly where the outline pixel has to be.
        tip = a.getpixel((x, y)) if x < a.width and y < a.height else None
        body = (a.getpixel((x + 2, y + 3))
                if x + 2 < a.width and y + 3 < a.height else None)

        check(tip == OUTLINE,
              f"cursor outline is drawn at the reported spot "
              f"({x},{y}) = {tip}")
        check(body == FILL,
              f"cursor body is drawn just inside it "
              f"({x + 2},{y + 3}) = {body}")

# --- buttons ------------------------------------------------------------
print()
print("== pressing the left button")

cmd("input-send-event",
    events=[{"type": "btn", "data": {"down": True, "button": "left"}}])
time.sleep(SHORT)

pressed_png = screendump("pressed.png")
pressed_log = pointer_log()

cmd("input-send-event",
    events=[{"type": "btn", "data": {"down": False, "button": "left"}}])
time.sleep(SHORT)

released_png = screendump("released.png")

try:
    with open(serial_path, "r", errors="replace") as fh:
        button_reports = re.findall(r"gui: pointer -?\d+,-?\d+ buttons 0x(\w+)",
                                    fh.read())
except OSError:
    button_reports = []

check("1" in button_reports,
      f"kernel saw the left button go down (button states seen: "
      f"{sorted(set(button_reports))})")

if pressed_png and released_png and pil and end_pos:
    x, y = end_pos
    p_img = Image.open(pressed_png).convert("RGB")
    r_img = Image.open(released_png).convert("RGB")

    if x + 2 < p_img.width and y + 3 < p_img.height:
        check(p_img.getpixel((x + 2, y + 3)) == FILL_PRESSED,
              "cursor shows the pressed fill while the button is held")
        check(r_img.getpixel((x + 2, y + 3)) == FILL,
              "cursor returns to its normal fill on release")

# --- damage tracking ----------------------------------------------------
print()
print("== moving the pointer back")

for _ in range(STEPS):
    cmd("input-send-event", events=[
        {"type": "rel", "data": {"axis": "x", "value": -STEP_X}},
        {"type": "rel", "data": {"axis": "y", "value": -STEP_Y}},
    ])

time.sleep(PAUSE)

back_png = screendump("back.png")
back_log = pointer_log()
back_pos = back_log[-1] if back_log else None

print(f"== pointer back at: {back_pos}")

if back_pos and end_pos:
    check(back_pos[0] < end_pos[0] and back_pos[1] < end_pos[1],
          f"pointer moved back up and left (to {back_pos})")

if back_png and pil and end_pos:
    bimg = Image.open(back_png).convert("RGB")
    x, y = end_pos

    # Where the cursor used to be has to have been repainted with whatever
    # the compositor has underneath - if damage tracking missed it, the old
    # arrow would still be sitting there.
    if x < bimg.width and y < bimg.height:
        check(bimg.getpixel((x, y)) != OUTLINE,
              f"no cursor left behind at the old spot ({x},{y}) = "
              f"{bimg.getpixel((x, y))}")

    if back_pos:
        bx, by = back_pos
        if bx < bimg.width and by < bimg.height:
            check(bimg.getpixel((bx, by)) == OUTLINE,
                  f"cursor is at the new spot ({bx},{by}) = "
                  f"{bimg.getpixel((bx, by))}")

# --- a single isolated movement -----------------------------------------
# One packet with nothing following it is the case a lost wakeup would break:
# the kernel records the new position but never composites it, so the cursor
# would sit one step behind where the mouse stopped.
print()
print("== one isolated nudge")

cmd("input-send-event", events=[
    {"type": "rel", "data": {"axis": "x", "value": 9}},
    {"type": "rel", "data": {"axis": "y", "value": 7}},
])

time.sleep(PAUSE)

nudge_png = screendump("nudge.png")
nudge_log = pointer_log()
nudge_pos = nudge_log[-1] if nudge_log else None

print(f"== pointer after nudge: {nudge_pos}")

if back_pos and nudge_pos:
    check(nudge_pos != back_pos,
          f"the single packet was seen ({back_pos} -> {nudge_pos})")

if nudge_png and pil and nudge_pos:
    nimg = Image.open(nudge_png).convert("RGB")
    nx, ny = nudge_pos

    if nx < nimg.width and ny < nimg.height:
        check(nimg.getpixel((nx, ny)) == OUTLINE,
              f"cursor is composited at the final position with no further "
              f"events ({nx},{ny}) = {nimg.getpixel((nx, ny))}")

# --- the screen edges ---------------------------------------------------
# Clamping, cursor clipping and the present-rect arithmetic all meet at the
# boundaries, so drive the pointer hard into two opposite corners and check
# it ends up pinned exactly on the last pixel with the arrow still drawn.
def slam(dx, dy, steps=120):
    for _ in range(steps):
        cmd("input-send-event", events=[
            {"type": "rel", "data": {"axis": "x", "value": dx}},
            {"type": "rel", "data": {"axis": "y", "value": dy}},
        ])
    time.sleep(PAUSE)


print()
print("== slamming into the bottom-right corner")
slam(20, 20)

br_png = screendump("corner-br.png")
br_log = pointer_log()
br_pos = br_log[-1] if br_log else None
print(f"== pointer: {br_pos}")

if br_png and pil:
    img = Image.open(br_png).convert("RGB")
    w, h = img.width, img.height

    check(br_pos == (w - 1, h - 1),
          f"pointer clamps to the last pixel ({w - 1},{h - 1}), got {br_pos}")
    check(img.getpixel((w - 1, h - 1)) == OUTLINE,
          f"cursor tip is drawn in the corner = "
          f"{img.getpixel((w - 1, h - 1))}")

print()
print("== slamming into the top-left corner")
slam(-20, -20)

tl_png = screendump("corner-tl.png")
tl_log = pointer_log()
tl_pos = tl_log[-1] if tl_log else None
print(f"== pointer: {tl_pos}")

if tl_png and pil:
    img = Image.open(tl_png).convert("RGB")

    check(tl_pos == (0, 0), f"pointer clamps to the origin, got {tl_pos}")
    check(img.getpixel((0, 0)) == OUTLINE,
          f"cursor tip is drawn at the origin = {img.getpixel((0, 0))}")

    # The arrow overlaps the panel here, and the panel has to survive it:
    # the bar is still there to the right of the cursor.
    PANEL_BG = (0x14, 0x18, 0x22)
    check(img.getpixel((CURSOR_W + 4, 1)) in (PANEL_BG, (0xD8, 0xDE, 0xEA)),
          f"panel is intact beside the cursor = "
          f"{img.getpixel((CURSOR_W + 4, 1))}")

    # And nothing from the old corner survived
    if br_pos:
        check(img.getpixel(br_pos) != OUTLINE,
              f"no cursor left in the far corner {br_pos} = "
              f"{img.getpixel(br_pos)}")

# --- the framebuffer matches what we composed ---------------------------
# The whole point of the back buffer is that incremental presents keep the
# screen identical to the composed scene. If damage tracking ever missed a
# region the two would have drifted apart by now, so compare them outright.
print()
print("== comparing the composed scene against the framebuffer")

try:
    with open(serial_path, "r", errors="replace") as fh:
        serial_text = fh.read()
except OSError:
    serial_text = ""

fb = re.search(r"framebuffer at 0x([0-9a-f]+) size 0x([0-9a-f]+)", serial_text)
bb = re.search(r"back buffer at 0x([0-9a-f]+) size 0x([0-9a-f]+)", serial_text)

if not (fb and bb):
    check(False, "kernel reported both buffer addresses")
else:
    fb_addr, fb_size = int(fb.group(1), 16), int(fb.group(2), 16)
    bb_addr, bb_size = int(bb.group(1), 16), int(bb.group(2), 16)

    print(f"   framebuffer 0x{fb_addr:x} ({fb_size} bytes)")
    print(f"   back buffer 0x{bb_addr:x} ({bb_size} bytes)")

    check(fb_size == bb_size, "both buffers are the same size")

    front_bin = os.path.join(out, "front.bin")
    back_bin = os.path.join(out, "back.bin")

    r1 = cmd("pmemsave", val=fb_addr, size=fb_size, filename=front_bin)
    r2 = cmd("pmemsave", val=bb_addr, size=bb_size, filename=back_bin)

    if "error" in r1 or "error" in r2:
        print(f"   skip  pmemsave unavailable: "
              f"{r1.get('error') or r2.get('error')}")
    else:
        with open(front_bin, "rb") as fh:
            front_bytes = fh.read()
        with open(back_bin, "rb") as fh:
            back_bytes = fh.read()

        if front_bytes == back_bytes:
            check(True, "framebuffer is byte-identical to the composed scene")
        else:
            differing = sum(1 for a, b in zip(front_bytes, back_bytes)
                            if a != b)
            first = next((i for i, (a, b)
                          in enumerate(zip(front_bytes, back_bytes))
                          if a != b), None)
            px = first // 4 if first is not None else None
            where = (f"pixel {px % 720},{px // 720}"
                     if px is not None else "?")

            check(False,
                  f"framebuffer drifted from the composed scene: "
                  f"{differing} bytes differ, first at {where} - "
                  f"a present missed a damaged region")

# --- the interrupt handler kept up ---------------------------------------
try:
    with open(serial_path, "r", errors="replace") as fh:
        overrun_lines = re.findall(r"mouse drain overruns: (\d+)", fh.read())
except OSError:
    overrun_lines = []

print()
check(not overrun_lines,
      f"mouse interrupts never ran out of drain budget "
      f"(overrun reports: {overrun_lines or 'none'})")

cmd("quit")

print()
if failures:
    print(f"== {len(failures)} check(s) FAILED")
    sys.exit(1)

print("== all checks passed")
PY
RC=$?

echo
echo "== serial (last 12 lines of $SERIAL)"
tail -12 "$SERIAL" 2>/dev/null || echo "  (empty)"

echo
echo "== artifacts in $OUT"

exit "$RC"
