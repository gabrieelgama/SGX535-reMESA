set -eu
stage=start
trap 'rc=$?; if [ "$rc" -ne 0 ]; then printf "RECOVERY_STOP stage=%s exit=%s\n" "$stage" "$rc" >&2; fi' EXIT
service=/etc/runit/runsvdir/default/slimski
pci=/sys/bus/pci/devices/0000:00:02.0
module=/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko
expected_sha=7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb
expected_note=040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3
stage=identity
test "$(uname -r)" = 5.10.240-antix.1-486-smp
test "$(uname -m)" = i686
test "$(cat "$pci/vendor")" = 0x8086
test "$(cat "$pci/device")" = 0x8108
test "$(sha256sum "$module" | awk '{print $1}')" = "$expected_sha"
stage=loaded_identity
test "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id | tr -d ' \n')" = "$expected_note"
test "$(awk '$1 == "gma500_gfx" {print $5}' /proc/modules)" = Live
stage=pci_drm_binding
test "$(readlink -f "$pci/driver")" = /sys/bus/pci/drivers/gma500
test -c /dev/dri/card0
test "$(readlink -f /sys/class/drm/card0/device)" = "$(readlink -f "$pci")"
stage=framebuffer
test "$(cat /sys/class/graphics/fb0/name)" = gma500drmfb
test "$(cat /sys/class/vtconsole/vtcon0/name)" = '(S) VGA+'
test "$(cat /sys/class/vtconsole/vtcon1/name)" = '(M) frame buffer device'
stage=service_down
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = down
! pgrep -x Xorg >/dev/null
stage=console_before
v0=$(cat /sys/class/vtconsole/vtcon0/bind)
v1=$(cat /sys/class/vtconsole/vtcon1/bind)
case "$v0:$v1" in
 0:1) printf 'CONSOLE_ALREADY_RESTORED\n' ;;
 1:0) stage=normal_console_rebind; printf '1\n' > /sys/class/vtconsole/vtcon1/bind ;;
 *) exit 1 ;;
esac
stage=console_binding
test "$(cat /sys/class/vtconsole/vtcon0/bind)" = 0
test "$(cat /sys/class/vtconsole/vtcon1/bind)" = 1
test "$(cat /sys/module/gma500_gfx/refcnt)" = 1
printf 'CONSOLE_RESTORE_PASS vtcon0=0 vtcon1=1 refcount=1\n'
