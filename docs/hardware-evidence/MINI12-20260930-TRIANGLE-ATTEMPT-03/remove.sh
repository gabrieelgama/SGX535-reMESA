set -u
service=/etc/runit/runsvdir/default/slimski
pci=/sys/bus/pci/devices/0000:00:02.0
expected_note=040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3
stop() { printf 'STOP: %s\n' "$1" >&2; exit 21; }
hold() { printf 'HOLD: %s\n' "$1" >&2; exit 30; }
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = down || stop service-not-down
test "$(cat /sys/class/vtconsole/vtcon0/bind)" = 1 || stop vtcon0-not-bound
test "$(cat /sys/class/vtconsole/vtcon1/bind)" = 0 || stop vtcon1-still-bound
test "$(cat /sys/module/gma500_gfx/refcnt)" = 0 || stop module-refcount-not-zero
test "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id | tr -d ' \n')" = "$expected_note" || stop original-module-changed
test "$(readlink -f "$pci/driver")" = /sys/bus/pci/drivers/gma500 || stop pci-driver-changed
printf 'REMOVAL_POSSIBLE original-module-only nonforced\n'
if /usr/sbin/modprobe -r gma500_gfx; then remove_rc=0; else remove_rc=$?; fi
printf 'REMOVE_RC=%s\n' "$remove_rc"
if [ "$remove_rc" != 0 ]; then
    if [ -e /sys/module/gma500_gfx ] &&
       [ "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id 2>/dev/null | tr -d ' \n')" = "$expected_note" ] &&
       [ "$(readlink -f "$pci/driver" 2>/dev/null)" = /sys/bus/pci/drivers/gma500 ]; then
        stop normal-removal-refused-original-intact
    fi
    hold removal-effect-ambiguous
fi
test ! -e /sys/module/gma500_gfx || hold module-still-present
test ! -e "$pci/driver" || hold pci-still-bound
test ! -e /dev/dri/card0 || hold drm-card-still-present
if [ -e /sys/class/graphics/fb0/name ]; then
    test "$(cat /sys/class/graphics/fb0/name)" != gma500drmfb || hold original-fb-still-present
fi
printf 'REMOVE_PASS module-absent pci-unbound card0-absent\n'
