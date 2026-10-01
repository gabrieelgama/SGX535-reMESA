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
hold() { printf 'HOLD: %s\n' "$1" >&2; exit 30; }
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = down || hold service-not-down
test "$(note)" = "$expected_note" || hold original-module-changed-before-unbind
test "$(driver)" = /sys/bus/pci/drivers/gma500 || hold pci-driver-changed-before-unbind
test "$(v1)" = 1 || hold vtcon1-not-bound-before-unbind
test "$(refcnt)" = 1 || hold refcount-not-one-before-unbind
if printf '0\n' > "$bind"; then unbind_rc=0; else unbind_rc=$?; fi
u_v1=$(v1)
u_v0=$(v0)
u_ref=$(refcnt)
u_note=$(note)
u_driver=$(driver)
printf 'UNBIND write_rc=%s vtcon1=%s vtcon0=%s refcount=%s build_note=%s pci_driver=%s\n' \
    "$unbind_rc" "$u_v1" "$u_v0" "$u_ref" "$u_note" "$u_driver"
test "$u_note" = "$expected_note" || hold original-module-changed-after-unbind
test "$u_driver" = /sys/bus/pci/drivers/gma500 || hold pci-driver-changed-after-unbind
case "$u_v1" in
    0)
        if [ "$unbind_rc" = 0 ] && [ "$u_v0" = 1 ] && [ "$u_ref" = 0 ]; then
            result=RELEASE_OBSERVED
        else
            result=RELEASE_NOT_PROVEN
        fi
        if printf '1\n' > "$bind"; then rebind_rc=0; else rebind_rc=$?; fi
        r_v1=$(v1)
        r_v0=$(v0)
        r_ref=$(refcnt)
        r_note=$(note)
        r_driver=$(driver)
        printf 'REBIND write_rc=%s vtcon1=%s vtcon0=%s refcount=%s build_note=%s pci_driver=%s\n' \
            "$rebind_rc" "$r_v1" "$r_v0" "$r_ref" "$r_note" "$r_driver"
        test "$rebind_rc" = 0 && test "$r_v1" = 1 &&
            test "$r_v0" = 0 && test "$r_ref" = 1 &&
            test "$r_note" = "$expected_note" &&
            test "$r_driver" = /sys/bus/pci/drivers/gma500 || hold rebind-not-verified
        ;;
    1)
        test "$u_v0" = 0 && test "$u_ref" = 1 || hold unbind-contradictory
        result=NO_EFFECT
        ;;
    *) hold unbind-binding-unknown ;;
esac
test "$(cat /sys/class/graphics/fb0/name 2>/dev/null)" = gma500drmfb || hold framebuffer-changed
test "$(readlink -f /sys/class/drm/card0/device 2>/dev/null)" = "$(readlink -f "$pci")" || hold drm-device-changed
printf 'CONSOLE_TEST_RESULT=%s ORIGINAL_STATE_READY_FOR_SERVICE_RESTORE\n' "$result"
