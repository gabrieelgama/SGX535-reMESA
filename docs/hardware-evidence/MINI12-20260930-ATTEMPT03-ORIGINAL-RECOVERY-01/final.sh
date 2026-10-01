set -eu
service=/etc/runit/runsvdir/default/slimski
pci=/sys/bus/pci/devices/0000:00:02.0
module=/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko
expected_note=040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3
test "$(uname -r)" = 5.10.240-antix.1-486-smp
test "$(uname -m)" = i686
test "$(cat "$pci/vendor")" = 0x8086
test "$(cat "$pci/device")" = 0x8108
test "$(readlink -f "$pci/driver")" = /sys/bus/pci/drivers/gma500
test "$(readlink -f /sys/class/drm/card0/device)" = "$(readlink -f "$pci")"
test "$(cat /sys/class/graphics/fb0/name)" = gma500drmfb
test "$(cat /sys/class/vtconsole/vtcon0/name)" = '(S) VGA+'
test "$(cat /sys/class/vtconsole/vtcon0/bind)" = 0
test "$(cat /sys/class/vtconsole/vtcon1/name)" = '(M) frame buffer device'
test "$(cat /sys/class/vtconsole/vtcon1/bind)" = 1
test "$(cat /sys/module/gma500_gfx/refcnt)" = 2
test "$(awk '$1 == "gma500_gfx" {print $3 ":" $5}' /proc/modules)" = 2:Live
test "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id | tr -d ' \n')" = "$expected_note"
test "$(sha256sum "$module" | awk '{print $1}')" = 7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb
/usr/bin/sv status "$service"
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = run
test "$(pgrep -xc slimski)" = 1
test "$(pgrep -xc Xorg)" = 1
holders=''
for fd in /proc/[0-9]*/fd/*; do
    target=$(readlink "$fd" 2>/dev/null || true)
    case "$target" in
        /dev/dri/*|/dev/fb[0-9]*) holders="$holders $target" ;;
    esac
done
test "$holders" = ' /dev/dri/card0'
printf 'FINAL_PASS original-module refcount=2 vtcon0=0 vtcon1=1 holder=card0\n'
printf 'FRAMEBUFFER_SIZE '; cat /sys/class/graphics/fb0/virtual_size
printf 'DMESG_BEGIN\n'
dmesg
printf 'DMESG_END\n'
