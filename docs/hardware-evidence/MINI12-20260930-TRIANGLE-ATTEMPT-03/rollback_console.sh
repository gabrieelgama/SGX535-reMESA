set -u
service=/etc/runit/runsvdir/default/slimski
pci=/sys/bus/pci/devices/0000:00:02.0
bind=/sys/class/vtconsole/vtcon1/bind
expected_note=040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3
hold() { printf 'HOLD: %s\n' "$1" >&2; exit 30; }
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = down || hold service-not-down
test "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id 2>/dev/null | tr -d ' \n')" = "$expected_note" || hold original-module-not-intact
test "$(readlink -f "$pci/driver" 2>/dev/null)" = /sys/bus/pci/drivers/gma500 || hold pci-driver-not-intact
before_v1=$(cat "$bind" 2>/dev/null || true)
before_ref=$(cat /sys/module/gma500_gfx/refcnt 2>/dev/null || true)
case "$before_v1:$before_ref" in
    0:0)
        if printf '1\n' > "$bind"; then write_rc=0; else write_rc=$?; fi
        printf 'REBIND_WRITE_RC=%s\n' "$write_rc"
        test "$write_rc" = 0 || hold rebind-write-failed
        ;;
    1:1) printf 'REBIND_NOT_NEEDED\n' ;;
    *) hold console-or-refcount-contradictory ;;
esac
test "$(cat /sys/class/vtconsole/vtcon0/bind)" = 0 || hold vtcon0-not-restored
test "$(cat "$bind")" = 1 || hold vtcon1-not-restored
test "$(cat /sys/module/gma500_gfx/refcnt)" = 1 || hold refcount-not-restored
test "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id | tr -d ' \n')" = "$expected_note" || hold original-module-changed
test "$(readlink -f "$pci/driver")" = /sys/bus/pci/drivers/gma500 || hold pci-driver-changed
printf 'ROLLBACK_CONSOLE_PASS original-module refcount=1 vtcon0=0 vtcon1=1\n'
