set -eu
service=/etc/runit/runsvdir/default/slimski
pci=/sys/bus/pci/devices/0000:00:02.0
expected_note=040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = down
test "$(cat /sys/class/vtconsole/vtcon0/bind)" = 0
test "$(cat /sys/class/vtconsole/vtcon1/bind)" = 1
test "$(cat /sys/module/gma500_gfx/refcnt)" = 1
test "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id | tr -d ' \n')" = "$expected_note"
test "$(readlink -f "$pci/driver")" = /sys/bus/pci/drivers/gma500
/usr/bin/sv start "$service"
status=$(/usr/bin/sv status "$service")
printf 'SERVICE %s\n' "$status"
test "${status%%:*}" = run
test "$(pgrep -xc slimski)" = 1
test "$(pgrep -xc Xorg)" = 1
printf 'RESTORE_PASS service-running original-module\n'
