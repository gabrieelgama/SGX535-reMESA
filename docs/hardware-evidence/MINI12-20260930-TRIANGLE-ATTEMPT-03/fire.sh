set -u
service=/etc/runit/runsvdir/default/slimski
pci=/sys/bus/pci/devices/0000:00:02.0
client=/root/sgx535-frozen-seq1/frozen-triangle-one-shot-i386
color=/root/sgx535-frozen-seq1/attempt-03-color.bin
expected_note=040000001400000003000000474e5500d4cb8d750efa6eb401f82e6eaec7eb82ebdf9c3c
stop() { printf 'STOP: %s\n' "$1" >&2; exit 21; }
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = down || stop service-not-down
if pgrep -x slimski >/dev/null || pgrep -x Xorg >/dev/null; then stop graphical-process-present; fi
for fd in /proc/[0-9]*/fd/*; do
    target=$(readlink "$fd" 2>/dev/null || true)
    case "$target" in /dev/dri/*|/dev/fb[0-9]*) stop unexpected-graphics-holder;; esac
done
test "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id 2>/dev/null | tr -d ' \n')" = "$expected_note" || stop candidate-identity-changed
test "$(readlink -f "$pci/driver" 2>/dev/null)" = /sys/bus/pci/drivers/gma500 || stop pci-binding-changed
test "$(readlink -f /sys/class/drm/card0/device 2>/dev/null)" = "$(readlink -f "$pci")" || stop drm-binding-changed
test "$(sha256sum "$client" | awk '{print $1}')" = 758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf || stop client-hash-changed
test "$(stat -c '%U:%G:%a' "$client")" = root:root:700 || stop client-mode-changed
test ! -e "$color" || stop color-output-already-exists
printf 'FIRE_POSSIBLE sequence=1 one-client-invocation\n'
if "$client" --one-shot-sgx535-rev121 "$color"; then client_rc=0; else client_rc=$?; fi
printf 'CLIENT_EXIT=%s\n' "$client_rc"
if [ -e "$color" ]; then
    printf 'COLOR_FILE_SIZE='; wc -c < "$color"
    sha256sum "$color"
    printf 'COLOR_BYTES_BEGIN\n'
    od -An -tx1 -v "$color"
    printf 'COLOR_BYTES_END\n'
else
    printf 'COLOR_FILE_ABSENT\n'
fi
test "$client_rc" = 0 || exit 31
printf 'FIRE_CLIENT_RETURNED_SUCCESS\n'
