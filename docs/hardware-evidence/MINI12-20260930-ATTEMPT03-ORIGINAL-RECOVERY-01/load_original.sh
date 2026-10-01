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
stage=normal_load_plan
test "$(/usr/sbin/modinfo -F depends "$module")" = drm,drm_kms_helper,video,i2c-algo-bit
expected_plan='insmod /lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/i2c/algos/i2c-algo-bit.ko
insmod /lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/drm_kms_helper.ko
insmod /lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko'
plan=$(/usr/sbin/modprobe -n -v gma500_gfx | sed 's/[[:space:]]*$//')
printf 'LOAD_PLAN_BEGIN\n%s\nLOAD_PLAN_END\n' "$plan"
test "$plan" = "$expected_plan"
for dep in /lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/i2c/algos/i2c-algo-bit.ko /lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/drm_kms_helper.ko; do
    test "$(/usr/sbin/modinfo -F vermagic "$dep" | sed 's/[[:space:]]*$//')" = '5.10.240-antix.1-486-smp SMP mod_unload modversions 486'
    sha256sum "$dep"
done
stage=normal_original_insert
/usr/sbin/modprobe gma500_gfx
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
stage=console_state
v0=$(cat /sys/class/vtconsole/vtcon0/bind)
v1=$(cat /sys/class/vtconsole/vtcon1/bind)
case "$v0:$v1" in 0:1|1:0) ;; *) exit 1 ;; esac
printf 'ORIGINAL_LOAD_PASS vtcon0=%s vtcon1=%s refcnt=%s\n' "$v0" "$v1" "$(cat /sys/module/gma500_gfx/refcnt)"
printf 'DMESG_BEGIN\n'
dmesg
printf 'DMESG_END\n'
