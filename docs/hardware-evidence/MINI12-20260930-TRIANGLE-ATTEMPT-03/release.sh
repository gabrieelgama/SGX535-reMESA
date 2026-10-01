set -u
service=/etc/runit/runsvdir/default/slimski
pci=/sys/bus/pci/devices/0000:00:02.0
bind=/sys/class/vtconsole/vtcon1/bind
expected_note=040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3
note() { od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id 2>/dev/null | tr -d ' \n'; }
driver() { readlink -f "$pci/driver" 2>/dev/null || true; }
refcnt() { cat /sys/module/gma500_gfx/refcnt 2>/dev/null || true; }
v1() { cat "$bind" 2>/dev/null || true; }
v0() { cat /sys/class/vtconsole/vtcon0/bind 2>/dev/null || true; }
stop() { printf 'STOP: %s\n' "$1" >&2; exit 20; }
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = down || stop service-not-down
test "$(note)" = "$expected_note" || stop original-module-changed
test "$(driver)" = /sys/bus/pci/drivers/gma500 || stop pci-driver-changed
test "$(v0)" = 0 && test "$(v1)" = 1 && test "$(refcnt)" = 1 || stop console-or-refcount-changed
if printf '0\n' > "$bind"; then write_rc=0; else write_rc=$?; fi
u_v1=$(v1)
u_v0=$(v0)
u_ref=$(refcnt)
u_note=$(note)
u_driver=$(driver)
printf 'UNBIND write_rc=%s vtcon1=%s vtcon0=%s refcount=%s build_note=%s pci_driver=%s\n' \
    "$write_rc" "$u_v1" "$u_v0" "$u_ref" "$u_note" "$u_driver"
test "$write_rc" = 0 && test "$u_v1" = 0 && test "$u_v0" = 1 &&
    test "$u_ref" = 0 && test "$u_note" = "$expected_note" &&
    test "$u_driver" = /sys/bus/pci/drivers/gma500 || stop console-release-not-verified
printf 'RELEASE_PASS original-module refcount=0 vtcon0=1 vtcon1=0\n'
