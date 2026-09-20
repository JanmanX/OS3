#!/usr/bin/env bash
#
# Boot the OS3 ISO headlessly, capture a framebuffer screenshot and the
# full serial log, then shut the machine down again.
#
# Artifacts land in $OUT (default /tmp/os3-test):
#   shot.png    framebuffer contents at the moment of capture
#   serial.log  everything the kernel wrote through kprintf
#   qemu.log    QEMU's own stderr
#
# Exit status: 0 capture succeeded, 1 capture failed, 2 bad usage.

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

OUT="${OS3_TEST_OUT:-/tmp/os3-test}"
ISO="$ROOT/os.iso"
DISK="$ROOT/hdd.bin"
QEMU="${QEMU:-qemu-system-x86_64}"
MARKER="${OS3_BOOT_MARKER:-Done initializing.}"
WAIT="${OS3_BOOT_WAIT:-25}"
SETTLE="${OS3_BOOT_SETTLE:-2}"
BUILD=0

usage() {
	cat <<'USAGE'
usage: tools/boot-test.sh [options]

  -o, --out DIR      where to write artifacts (default /tmp/os3-test)
  -w, --wait SEC     seconds to wait for the boot marker (default 25)
  -s, --settle SEC   extra seconds after the marker before capture (default 2)
  -m, --marker TEXT  serial text that means "booted" (default "Done initializing.")
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
	-m|--marker)	MARKER="$2"; shift 2;;
	-i|--iso)	ISO="$2"; shift 2;;
	-b|--build)	BUILD=1; shift;;
	-h|--help)	usage; exit 0;;
	*)		echo "boot-test: unknown option $1" >&2; usage >&2; exit 2;;
	esac
done

mkdir -p "$OUT"

if [ "$BUILD" -eq 1 ]; then
	echo "== building"
	if ! make -C "$ROOT" iso >"$OUT/build.log" 2>&1; then
		tail -30 "$OUT/build.log" >&2
		echo "boot-test: build failed, see $OUT/build.log" >&2
		exit 1
	fi
fi

[ -f "$ISO" ] || { echo "boot-test: no ISO at $ISO (run 'make iso')" >&2; exit 1; }

SOCK="$OUT/qmp.sock"
SERIAL="$OUT/serial.log"
rm -f "$SOCK" "$SERIAL" "$OUT/shot.png" "$OUT/shot.ppm" "$OUT/qemu.log"

# KVM where available, plain TCG otherwise
ACCEL=(-machine q35)
if [ -w /dev/kvm ]; then
	ACCEL=(-machine q35,accel=kvm -cpu host)
fi

# -no-reboot + -no-shutdown: a triple fault stops the VM instead of
# rebooting into a loop, so the faulted screen is still capturable.
"$QEMU" \
	-m 2G "${ACCEL[@]}" \
	-vga std -display none \
	-no-reboot -no-shutdown \
	-drive id=disk,file="$DISK",if=none,format=raw \
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

# Wait for the monitor socket
for _ in $(seq 1 50); do
	[ -S "$SOCK" ] && break
	kill -0 "$QPID" 2>/dev/null || { echo "boot-test: QEMU died on startup" >&2; tail -20 "$OUT/qemu.log" >&2; exit 1; }
	sleep 0.2
done
[ -S "$SOCK" ] || { echo "boot-test: QMP socket never appeared" >&2; exit 1; }

# Wait for the kernel to announce itself
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
[ "$BOOTED" -eq 1 ] && echo "== marker seen" || echo "== marker NOT seen within ${WAIT}s"

sleep "$SETTLE"

OUT="$OUT" python3 - <<'PY'
import json, os, socket, sys

out = os.environ["OUT"]
sock = os.path.join(out, "qmp.sock")

try:
    s = socket.socket(socket.AF_UNIX)
    s.settimeout(15)
    s.connect(sock)
except OSError as e:
    print(f"boot-test: cannot reach QMP: {e}", file=sys.stderr)
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


cmd("qmp_capabilities")

status = cmd("query-status").get("return", {})
state = status.get("status", "unknown")
print(f"== vm status: {state}")

png = os.path.join(out, "shot.png")
res = cmd("screendump", filename=png, format="png")

if "error" in res:
    # Older QEMU has no png encoder; fall back to PPM and convert
    ppm = os.path.join(out, "shot.ppm")
    res = cmd("screendump", filename=ppm)
    if "error" in res:
        print(f"boot-test: screendump failed: {res['error']}", file=sys.stderr)
        sys.exit(1)
    try:
        from PIL import Image
        Image.open(ppm).save(png)
    except Exception as e:
        print(f"boot-test: could not convert PPM: {e}", file=sys.stderr)
        sys.exit(1)

cmd("quit")

if state in ("shutdown", "internal-error", "guest-panic"):
    print(f"boot-test: guest is in state '{state}' - likely a fault", file=sys.stderr)
    sys.exit(1)
PY
RC=$?

echo
echo "== serial (last 20 lines of $SERIAL)"
if [ -s "$SERIAL" ]; then
	tail -20 "$SERIAL"
else
	echo "  (empty - kernel produced no serial output)"
	RC=1
fi

echo
if [ -s "$OUT/shot.png" ]; then
	echo "== screenshot: $OUT/shot.png"
else
	echo "== screenshot: MISSING" >&2
	RC=1
fi

[ "$BOOTED" -eq 1 ] || RC=1

exit "$RC"
