set -eu
service=/etc/runit/runsvdir/default/slimski
pci=/sys/bus/pci/devices/0000:00:02.0
expected_note=040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = run
test "$(cat /sys/class/vtconsole/vtcon1/bind)" = 1
test "$(cat /sys/module/gma500_gfx/refcnt)" = 2
test "$(readlink -f "$pci/driver")" = /sys/bus/pci/drivers/gma500
test "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id | tr -d ' \n')" = "$expected_note"
/usr/bin/sv stop "$service"
status=$(/usr/bin/sv status "$service")
printf 'SERVICE %s\n' "$status"
test "${status%%:*}" = down
if pgrep -x slimski >/dev/null || pgrep -x Xorg >/dev/null; then
    printf 'STOP: graphical process remains\n' >&2
    exit 20
fi
for fd in /proc/[0-9]*/fd/*; do
    target=$(readlink "$fd" 2>/dev/null || true)
    case "$target" in
        /dev/dri/*|/dev/fb[0-9]*) printf 'STOP: graphics holder %s\n' "$target" >&2; exit 21 ;;
    esac
done
test "$(cat /sys/class/vtconsole/vtcon0/bind)" = 0
test "$(cat /sys/class/vtconsole/vtcon1/bind)" = 1
test "$(cat /sys/module/gma500_gfx/refcnt)" = 1
test "$(awk '$1 == "gma500_gfx" {print $3 ":" $5}' /proc/modules)" = 1:Live
test "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id | tr -d ' \n')" = "$expected_note"
test "$(readlink -f "$pci/driver")" = /sys/bus/pci/drivers/gma500
test "$(cat /sys/class/graphics/fb0/name)" = gma500drmfb
test "$(readlink -f /sys/class/drm/card0/device)" = "$(readlink -f "$pci")"
printf 'STOP_STAGE_PASS original-module refcount=1 vtcon0=0 vtcon1=1 no-graphics-holders\n'
