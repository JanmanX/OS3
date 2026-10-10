#!/usr/bin/env bash
#
# Boot the OS3 ISO headlessly with the sample disk attached, drive the file
# explorer with synthetic mouse clicks over QMP and check it against the tree
# on the disk image.
#
# What it actually proves:
#   - the AHCI driver reads the disk, and the FAT driver mounts the volume
#     through the MBR
#   - the root folder lists exactly what the image holds
#   - clicking a folder opens it, several levels deep, and Up walks back out
#   - long file names, an empty folder and an empty file are handled
#   - a folder bigger than one screen is paged with Prev / Next, every entry
#     appearing exactly once, and the buttons stop at either end
#   - clicking a file reads all of it: size and a checksum of every byte
#     match the file on the image, including a 200KiB file spanning
#     hundreds of clusters
#   - the window really is painted, and the preview shows file text
#
# The expected tree is read from the image itself with mtools, so the checks
# stay valid whatever tools/mkdisk.sh puts on it.
#
# Artifacts land in $OUT (default /tmp/os3-explorer-test):
#   root.png, notes.png, file.png, bigfile.png, photos-N.png, serial.log ...
#
# Exit status: 0 all checks passed, 1 a check failed, 2 bad usage.

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

OUT="${OS3_EXPLORER_TEST_OUT:-/tmp/os3-explorer-test}"
ISO="$ROOT/os.iso"
DISK="$ROOT/disk.img"		# attached with snapshot=on, see below
QEMU="${QEMU:-qemu-system-x86_64}"
MARKER="Explorer: running"
WAIT="${OS3_BOOT_WAIT:-}"
SETTLE="${OS3_BOOT_SETTLE:-}"
BUILD=0

usage() {
	cat <<'USAGE'
usage: tools/explorer-test.sh [options]

  -o, --out DIR      where to write artifacts (default /tmp/os3-explorer-test)
  -w, --wait SEC     seconds to wait for the explorer (default 25)
  -s, --settle SEC   extra seconds after the marker before the first click
  -i, --iso PATH     ISO to boot (default ./os.iso)
  -d, --disk PATH    disk image (default ./disk.img)
  -b, --build        run "make iso" and "make disk.img" first
  -h, --help         this message
USAGE
}

while [ $# -gt 0 ]; do
	case "$1" in
	-o|--out)	OUT="$2"; shift 2;;
	-w|--wait)	WAIT="$2"; shift 2;;
	-s|--settle)	SETTLE="$2"; shift 2;;
	-i|--iso)	ISO="$2"; shift 2;;
	-d|--disk)	DISK="$2"; shift 2;;
	-b|--build)	BUILD=1; shift;;
	-h|--help)	usage; exit 0;;
	*)		echo "explorer-test: unknown option $1" >&2; usage >&2; exit 2;;
	esac
done

mkdir -p "$OUT"

if [ "$BUILD" -eq 1 ]; then
	echo "== building"
	if ! { make -C "$ROOT" iso && make -C "$ROOT" disk.img; } >"$OUT/build.log" 2>&1; then
		tail -30 "$OUT/build.log" >&2
		echo "explorer-test: build failed, see $OUT/build.log" >&2
		exit 1
	fi
fi

[ -f "$ISO" ] || { echo "explorer-test: no ISO at $ISO (run 'make iso')" >&2; exit 1; }
[ -f "$DISK" ] || { echo "explorer-test: no disk at $DISK (run 'make disk.img')" >&2; exit 1; }

SOCK="$OUT/qmp.sock"
SERIAL="$OUT/serial.log"
rm -f "$SOCK" "$SERIAL" "$OUT"/*.png "$OUT"/*.ppm "$OUT/qemu.log"
rm -rf "$OUT/tree"

# The tree the explorer is expected to show, straight off the image. The
# partition starts 1MiB in. mtools balks at a geometry that is not a whole
# number of tracks, hence the skip.
export MTOOLS_SKIP_CHECK=1
mkdir -p "$OUT/tree"
if ! mcopy -i "$DISK@@1M" -s -n -m "::/" "$OUT/tree" 2>"$OUT/mtools.log"; then
	cat "$OUT/mtools.log" >&2
	echo "explorer-test: could not read the tree off $DISK" >&2
	exit 1
fi

# Same acceleration choices as gui-test.sh: KVM when available, otherwise TCG
# with the "max" CPU, which the kernel needs for 1GiB pages
ACCEL=(-machine q35 -cpu max)
SLOW=1

if [ -w /dev/kvm ] && [ "${OS3_NO_KVM:-0}" != "1" ]; then
	ACCEL=(-machine q35,accel=kvm -cpu host)
	SLOW=0
fi

if [ "$SLOW" -eq 1 ]; then
	: "${WAIT:=180}"
	: "${SETTLE:=6}"
else
	: "${WAIT:=25}"
	: "${SETTLE:=2}"
fi

export OS3_SLOW="$SLOW"

# snapshot=on: guest writes go to a throwaway overlay, and the image can be
# opened by two runs at once
"$QEMU" \
	-m 2G "${ACCEL[@]}" \
	-vga std -display none \
	-no-reboot -no-shutdown -boot d \
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
	kill -0 "$QPID" 2>/dev/null || { echo "explorer-test: QEMU died on startup" >&2; tail -20 "$OUT/qemu.log" >&2; exit 1; }
	sleep 0.2
done
[ -S "$SOCK" ] || { echo "explorer-test: QMP socket never appeared" >&2; exit 1; }

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
		echo "== the explorer never announced itself within ${WAIT}s" >&2
	else
		echo "== QEMU exited before the explorer started" >&2
	fi
	[ -s "$OUT/qemu.log" ] && { echo "== qemu said:"; sed 's/^/  /' "$OUT/qemu.log"; }
	echo "== serial (last 25 lines)"
	if [ -s "$SERIAL" ]; then tail -25 "$SERIAL"; else echo "  (empty)"; fi
	exit 1
fi
echo "== explorer up"

sleep "$SETTLE"

OUT="$OUT" SERIAL="$SERIAL" python3 -u - <<'PY'
import json, os, re, socket, sys, time

out = os.environ["OUT"]
serial_path = os.environ["SERIAL"]
tree = os.path.join(out, "tree")
SLOW = os.environ.get("OS3_SLOW", "0") == "1"
POLL = 40 if SLOW else 8          # seconds to wait for the guest to react
QUIET = 8 if SLOW else 1.5        # seconds to wait to see that nothing happens

failures = []


def check(ok, msg):
    print(("   ok   " if ok else "   FAIL ") + msg)
    if not ok:
        failures.append(msg)


# ---- the expected tree --------------------------------------------------

def fnv(data):
    h = 2166136261
    for b in data:
        h = ((h ^ b) * 16777619) & 0xFFFFFFFF
    return h


def expected_entries(path):
    """name -> is_dir for one folder of the image."""
    d = os.path.join(tree, path.lstrip("/"))
    return {n: os.path.isdir(os.path.join(d, n)) for n in os.listdir(d)}


def expected_file(path):
    with open(os.path.join(tree, path.lstrip("/")), "rb") as fh:
        data = fh.read()
    return len(data), fnv(data), data


# ---- QMP ----------------------------------------------------------------

s = socket.socket(socket.AF_UNIX)
s.settimeout(20)
try:
    s.connect(os.path.join(out, "qmp.sock"))
except OSError as e:
    print(f"explorer-test: cannot reach QMP: {e}", file=sys.stderr)
    sys.exit(1)

f = s.makefile("rw")
f.readline()


def cmd(execute, **args):
    f.write(json.dumps({"execute": execute, "arguments": args}) + "\n")
    f.flush()
    while True:
        line = f.readline()
        if not line:
            return {"error": {"desc": "monitor closed"}}
        msg = json.loads(line)
        if "return" in msg or "error" in msg:
            return msg


def screendump(name):
    png = os.path.join(out, name)
    res = cmd("screendump", filename=png, format="png")
    if "error" in res:
        ppm = png.replace(".png", ".ppm")
        if "error" in cmd("screendump", filename=ppm):
            return None
        try:
            from PIL import Image
            Image.open(ppm).save(png)
        except Exception:
            return None
    return png


# ---- what the kernel says -----------------------------------------------

def log_text():
    try:
        with open(serial_path, "r", errors="replace") as fh:
            return fh.read()
    except OSError:
        return ""


def events():
    """The serial log as a list of events, in order."""
    evs = []
    cur = None
    for line in log_text().splitlines():
        m = re.search(r"\]: (.*)$", line)
        text = m.group(1) if m else line
        d = re.match(r"explorer: dir (.*) page (\d+)/(\d+) entries (\d+)$", text)
        if d:
            cur = {"kind": "dir", "path": d.group(1), "page": int(d.group(2)),
                   "pages": int(d.group(3)), "total": int(d.group(4)),
                   "entries": []}
            evs.append(cur)
            continue
        e = re.match(r"explorer:   (.*)$", text)
        if e and cur:
            cur["entries"].append(e.group(1))
            continue
        cur = None
        fl = re.match(r"explorer: file (.*) size (\d+) read (\d+) fnv 0x(\w+)$", text)
        if fl:
            evs.append({"kind": "file", "path": fl.group(1),
                        "size": int(fl.group(2)), "read": int(fl.group(3)),
                        "fnv": int(fl.group(4), 16)})
            continue
        c = re.match(r"gui: click (-?\d+),(-?\d+)$", text)
        if c:
            evs.append({"kind": "click", "x": int(c.group(1)),
                        "y": int(c.group(2))})
            continue
        p = re.match(r"gui: pointer (-?\d+),(-?\d+) ", text)
        if p:
            evs.append({"kind": "pointer", "x": int(p.group(1)),
                        "y": int(p.group(2))})
    return evs


def of_kind(kind):
    return [e for e in events() if e["kind"] == kind]


def pointer():
    p = of_kind("pointer")
    return (p[-1]["x"], p[-1]["y"]) if p else None


def wait_for(pred, timeout):
    end = time.time() + timeout
    while time.time() < end:
        v = pred()
        if v:
            return v
        time.sleep(0.15)
    return None


# ---- layout, as the kernel announces it ---------------------------------

text = log_text()
buttons = {}
for m in re.finditer(r"explorer: button (\w+) (\d+),(\d+),(\d+),(\d+)", text):
    x, y, w, h = map(int, m.groups()[1:])
    buttons[m.group(1)] = (x + w // 2, y + h // 2)

m = re.search(r"explorer: rows (\d+),(\d+),(\d+),(\d+),(\d+)", text)
if not m or len(buttons) != 3:
    print("explorer-test: the kernel never announced the explorer layout",
          file=sys.stderr)
    sys.exit(1)
ROWS_X, ROWS_Y, ROWS_W, ROW_H, ROWS_PER_PAGE = map(int, m.groups())
print(f"== layout: buttons {buttons}, {ROWS_PER_PAGE} rows of {ROW_H}px "
      f"at {ROWS_X},{ROWS_Y}")

cmd("qmp_capabilities")


def move_to(x, y):
    """Walk the relative pointer to an exact spot, correcting from the log."""
    for _ in range(8):
        cur = pointer()
        if cur is None:
            time.sleep(0.5)
            continue
        dx, dy = x - cur[0], y - cur[1]
        if dx == 0 and dy == 0:
            return True
        while dx or dy:
            sx = max(-60, min(60, dx))
            sy = max(-60, min(60, dy))
            cmd("input-send-event", events=[
                {"type": "rel", "data": {"axis": "x", "value": sx}},
                {"type": "rel", "data": {"axis": "y", "value": sy}}])
            dx -= sx
            dy -= sy
        wait_for(lambda: pointer() != cur, POLL)
        time.sleep(0.3 if not SLOW else 1.5)
    return pointer() == (x, y)


def click_at(x, y):
    """Move there and press and release the left button. Returns whether the
    kernel registered a click, and how many events existed before it."""
    before = len(events())
    n_clicks = len(of_kind("click"))
    if not move_to(x, y):
        print(f"   (pointer stuck at {pointer()}, wanted {(x, y)})")
    cmd("input-send-event",
        events=[{"type": "btn", "data": {"down": True, "button": "left"}}])
    time.sleep(0.2 if not SLOW else 1)
    cmd("input-send-event",
        events=[{"type": "btn", "data": {"down": False, "button": "left"}}])
    ok = wait_for(lambda: len(of_kind("click")) > n_clicks, POLL)
    return bool(ok), before


def after(before, kind):
    """Events of one kind that appeared after a given point."""
    return [e for e in events()[before:] if e["kind"] == kind]


def current_dir():
    d = of_kind("dir")
    return d[-1] if d else None


def row_center(index):
    return ROWS_X + 80, ROWS_Y + index * ROW_H + ROW_H // 2


def click_entry(name):
    """Click a row on the page on show. Returns (clicked, before)."""
    d = current_dir()
    names = [e.rstrip("/") for e in d["entries"]]
    if name not in names:
        return False, len(events())
    x, y = row_center(names.index(name))
    return click_at(x, y)


def open_folder(name, expect_path):
    ok, before = click_entry(name)
    d = wait_for(lambda: after(before, "dir"), POLL)
    check(ok and bool(d) and d[-1]["path"] == expect_path,
          f"clicking '{name}' opens {expect_path}"
          + ("" if d else " (no listing arrived)"))
    return d[-1] if d else None


def open_file(name, path):
    ok, before = click_entry(name)
    fl = wait_for(lambda: after(before, "file"), POLL)
    size, h, _ = expected_file(path)
    if not fl:
        check(False, f"clicking '{name}' reads {path} (nothing was read)")
        return None
    fl = fl[-1]
    check(fl["path"] == path and fl["size"] == size and fl["read"] == size
          and fl["fnv"] == h,
          f"{path}: {fl['read']} of {size} bytes read, checksum "
          f"{'matches' if fl['fnv'] == h else 'DIFFERS'}")
    return fl


def same_listing(d, path):
    want = expected_entries(path)
    got = {e.rstrip("/"): e.endswith("/") for e in d["entries"]}
    return got == want


# ---- the walk -----------------------------------------------------------

print()
print("== checks")

state = cmd("query-status").get("return", {}).get("status", "unknown")
check(state == "running", f"guest is running (was '{state}')")
check("FAT: FAT32 volume" in text, "FAT driver mounted a FAT32 volume")
check("AHCI:" in text and "disk(s) ready" in text, "AHCI found a disk")

def count_color(img, rgb):
    """How many pixels are exactly this colour."""
    colors = img.getcolors(maxcolors=img.width * img.height) or []
    return sum(n for n, c in colors if c == rgb)


try:
    from PIL import Image
    pil = True
except ImportError:
    pil = False
    print("   skip  pixel checks (no PIL)")

root = current_dir()
check(root is not None and root["path"] == "/", "the explorer starts in /")
if root:
    check(same_listing(root, "/"),
          f"root lists exactly the {len(expected_entries('/'))} entries "
          f"on the image: {sorted(root['entries'])}")

shot = screendump("root.png")
check(shot is not None, "screenshot of the root captured")

if shot and pil:
    img = Image.open(shot).convert("RGB")
    ACCENT = (0x2F, 0x6F, 0xB8)
    check(count_color(img, ACCENT) > 2000,
          "the window title bar is painted")
    folder = (0xF2, 0xC2, 0x5C)
    check(count_color(img, folder) > 20,
          "folder markers are painted")

# --- down through the folders --------------------------------------------
print()
print("== opening folders")

d = open_folder("DOCS", "/DOCS")
if d:
    check(same_listing(d, "/DOCS"), f"/DOCS lists {sorted(d['entries'])}")
d = open_folder("notes", "/DOCS/notes")
if d:
    check(same_listing(d, "/DOCS/notes"),
          f"/DOCS/notes lists {len(d['entries'])} entries, "
          f"including the long and empty ones")
screendump("notes.png")

fl = open_file("A very long file name that needs several LFN entries.txt",
               "/DOCS/notes/A very long file name that needs several LFN entries.txt")
fl = open_file("empty.txt", "/DOCS/notes/empty.txt")
open_file("ideas.txt", "/DOCS/notes/ideas.txt")

shot = screendump("file.png")
if shot and pil:
    img = Image.open(shot).convert("RGB")
    PREVIEW = (0xB8, 0xD8, 0xB0)
    check(count_color(img, PREVIEW) > 30,
          "the preview shows the file's text")

d = open_folder("2026", "/DOCS/notes/2026")
if d:
    check(same_listing(d, "/DOCS/notes/2026"), f"…/2026 lists {sorted(d['entries'])}")
open_file("September Plans.txt", "/DOCS/notes/2026/September Plans.txt")
d = open_folder("archive", "/DOCS/notes/2026/archive")
if d:
    check(same_listing(d, "/DOCS/notes/2026/archive"),
          f"…/archive lists {sorted(d['entries'])}")
open_file("Q1 Review.txt", "/DOCS/notes/2026/archive/Q1 Review.txt")

# --- and back out --------------------------------------------------------
print()
print("== Up")

for expect in ["/DOCS/notes/2026", "/DOCS/notes", "/DOCS", "/"]:
    ok, before = click_at(*buttons["Up"])
    d = wait_for(lambda: after(before, "dir"), POLL)
    check(ok and bool(d) and d[-1]["path"] == expect, f"Up goes to {expect}")

n = len(events())
ok, _ = click_at(*buttons["Up"])
time.sleep(QUIET)
check(ok and not after(n, "dir"), "Up in the root does nothing")

# --- a big file ----------------------------------------------------------
print()
print("== a file spanning many clusters")

fl = open_file("bigfile.bin", "/bigfile.bin")
check(fl is not None and fl["read"] == 204800, "bigfile.bin read in full (200KiB)")
screendump("bigfile.png")
open_file("Welcome to OS3.txt", "/Welcome to OS3.txt")
open_file("README.TXT", "/README.TXT")

# --- paging --------------------------------------------------------------
print()
print("== paging through a folder bigger than the screen")

photos = expected_entries("/Photos")
seen = []

d = open_folder("Photos", "/Photos")
check(d is not None and d["total"] == len(photos),
      f"/Photos reports {len(photos)} entries "
      f"({d['total'] if d else '?'} reported)")

if d:
    pages = d["pages"]
    want_pages = -(-len(photos) // ROWS_PER_PAGE)
    check(pages == want_pages and pages > 1,
          f"it takes {want_pages} pages of {ROWS_PER_PAGE} rows (got {pages})")
    check(d["page"] == 1 and len(d["entries"]) == ROWS_PER_PAGE,
          "page 1 is full")
    seen += d["entries"]
    screendump("photos-1.png")

    for p in range(2, pages + 1):
        ok, before = click_at(*buttons["Next"])
        nd = wait_for(lambda: after(before, "dir"), POLL)
        nd = nd[-1] if nd else None
        check(ok and nd is not None and nd["page"] == p,
              f"Next shows page {p}")
        if nd:
            seen += nd["entries"]
            screendump(f"photos-{p}.png")

    check(sorted(seen) == sorted(photos),
          f"pages together hold each of the {len(photos)} photos once "
          f"({len(seen)} shown)")

    n = len(events())
    ok, _ = click_at(*buttons["Next"])
    time.sleep(QUIET)
    check(ok and not after(n, "dir"), "Next on the last page does nothing")

    ok, before = click_at(*buttons["Prev"])
    pd = wait_for(lambda: after(before, "dir"), POLL)
    check(ok and bool(pd) and pd[-1]["page"] == pages - 1,
          f"Prev goes back to page {pages - 1}")

    for p in range(pages - 2, 0, -1):
        ok, before = click_at(*buttons["Prev"])
        wait_for(lambda: after(before, "dir"), POLL)
    n = len(events())
    ok, _ = click_at(*buttons["Prev"])
    time.sleep(QUIET)
    check(ok and not after(n, "dir"), "Prev on the first page does nothing")

    # Opening a photo from a later page picks the right file
    ok, before = click_at(*buttons["Next"])
    wait_for(lambda: after(before, "dir"), POLL)
    cur = current_dir()
    name = next(e for e in cur["entries"] if e.endswith(".jpg"))
    open_file(name, "/Photos/" + name)

# --- the empty folder ----------------------------------------------------
print()
print("== an empty folder")

ok, before = click_at(*buttons["Up"])
wait_for(lambda: after(before, "dir"), POLL)
d = open_folder("Empty Folder", "/Empty Folder")
check(d is not None and d["total"] == 0 and d["entries"] == [],
      "the empty folder lists nothing")

n = len(events())
ok, _ = click_at(*row_center(0))
time.sleep(QUIET)
check(ok and not after(n, "dir") and not after(n, "file"),
      "clicking where a row would be does nothing")

ok, before = click_at(*buttons["Up"])
d = wait_for(lambda: after(before, "dir"), POLL)
check(bool(d) and d[-1]["path"] == "/", "Up leaves the empty folder")

screendump("final.png")

print()
check(cmd("query-status").get("return", {}).get("status") == "running",
      "the guest is still running after all of that")

print()
if failures:
    print(f"== {len(failures)} check(s) FAILED")
    sys.exit(1)
print("== all checks passed")
PY
RC=$?
echo "== artifacts in $OUT"
exit $RC
