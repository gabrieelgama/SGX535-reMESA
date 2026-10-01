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
stage=hold_state
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = down
! pgrep -x slimski >/dev/null
! pgrep -x Xorg >/dev/null
test ! -d /sys/module/gma500_gfx
! awk '$1 == "gma500_gfx" {found=1} END {exit !found}' /proc/modules
test ! -e "$pci/driver"
test ! -e /dev/dri/card0
test ! -e /sys/class/drm/card0
test "$(cat /sys/class/vtconsole/vtcon0/bind)" = 1
test ! -e /sys/class/vtconsole/vtcon1/bind
holders=''
for fd in /proc/[0-9]*/fd/*; do
    target=$(readlink "$fd" 2>/dev/null || true)
    case "$target" in /dev/dri/*|/dev/fb[0-9]*) holders="$holders $target" ;; esac
done
test -z "$holders"
stage=load_preview
/usr/sbin/modprobe -n -v gma500_gfx
printf 'HOLD_PRECHECK_PASS original_sha=%s\n' "$expected_sha"
printf 'DMESG_BEGIN\n'
dmesg
printf 'DMESG_END\n'
