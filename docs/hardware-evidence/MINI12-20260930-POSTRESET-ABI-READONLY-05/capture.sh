#!/bin/sh
# Read-only; all output streams back to the local evidence recorder.
set -eu
stage=identity
trap 'rc=$?; if [ "$rc" -ne 0 ]; then printf "READONLY_STOP stage=%s exit=%s\n" "$stage" "$rc" >&2; fi' EXIT
expected_release=5.10.240-antix.1-486-smp
expected_sha=7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb
expected_note=040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3
pci=/sys/bus/pci/devices/0000:00:02.0
service=/etc/runit/runsvdir/default/slimski
release=$(uname -r)
arch=$(uname -m)
boot_id=$(cat /proc/sys/kernel/random/boot_id)
printf 'kernel_release=%s\nmachine_arch=%s\nboot_id=%s\n' "$release" "$arch" "$boot_id"
uname -v
cat /proc/version
printf 'kernel_taint=%s\n' "$(cat /proc/sys/kernel/tainted)"
printf 'product_name=%s\n' "$(cat /sys/class/dmi/id/product_name)"
test "$release" = "$expected_release"
test "$arch" = i686
# Exact raw DMI bytes and P/O/E taint baseline are retained target evidence.
test "$(cat /sys/class/dmi/id/product_name)" = 'Inspiron 1210   '
test "$(cat /proc/sys/kernel/tainted)" = 12289
module=/lib/modules/$release/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko
stage=module_identity
printf 'MODULE_BEGIN\n'
awk '$1 == "gma500_gfx" {print; found=1} END {if (!found) exit 1}' /proc/modules
printf 'module_initstate=%s\n' "$(cat /sys/module/gma500_gfx/initstate)"
note=$(od -An -tx1 /sys/module/gma500_gfx/notes/.note.gnu.build-id | tr -d ' \n')
printf 'loaded_build_note=%s\n' "$note"
sha256sum "$module"
/usr/sbin/modinfo "$module"
printf 'MODULE_END\n'
test "$note" = "$expected_note"
test "$(sha256sum "$module" | awk '{print $1}')" = "$expected_sha"
test "$(cat /sys/module/gma500_gfx/initstate)" = live
awk '$1 == "gma500_gfx" && $5 == "Live" {found=1} END {exit !found}' /proc/modules
stage=display_state
printf 'pci_vendor=%s\npci_device=%s\npci_driver=%s\n' "$(cat "$pci/vendor")" "$(cat "$pci/device")" "$(readlink -f "$pci/driver")"
printf 'drm_card_device=%s\n' "$(readlink -f /sys/class/drm/card0/device)"
printf 'drm_card_node=%s\n' "$(stat -c '%F:%t:%T' /dev/dri/card0)"
printf 'proc_fb=%s\n' "$(cat /proc/fb)"
for field in name virtual_size bits_per_pixel stride; do
    printf 'fb_%s=%s\n' "$field" "$(cat "/sys/class/graphics/fb0/$field")"
done
for console in vtcon0 vtcon1; do
    printf '%s_name=%s\n%s_bind=%s\n' "$console" "$(cat "/sys/class/vtconsole/$console/name")" "$console" "$(cat "/sys/class/vtconsole/$console/bind")"
done
service_status=$(/usr/bin/sv status "$service")
printf 'service_status=%s\n' "$service_status"
ps -eo pid=,ppid=,comm=,args= | awk '$3 == "slimski" || $3 == "Xorg" || ($3 == "runsv" && $0 ~ /slimski/)'
test "$(cat "$pci/vendor")" = 0x8086
test "$(cat "$pci/device")" = 0x8108
test "$(readlink -f "$pci/driver")" = /sys/bus/pci/drivers/gma500
test "$(readlink -f /sys/class/drm/card0/device)" = /sys/devices/pci0000:00/0000:00:02.0
test "$(stat -c '%F:%t:%T' /dev/dri/card0)" = 'character special file:e2:0'
test "$(cat /proc/fb)" = '0 gma500drmfb'
test "$(cat /sys/class/graphics/fb0/name)" = gma500drmfb
test "$(cat /sys/class/graphics/fb0/virtual_size)" = 1280,800
test "$(cat /sys/class/graphics/fb0/bits_per_pixel)" = 32
test "$(cat /sys/class/graphics/fb0/stride)" = 8192
test "$(cat /sys/class/vtconsole/vtcon0/bind)" = 0
test "$(cat /sys/class/vtconsole/vtcon1/bind)" = 1
case "$service_status" in run:*) ;; *) exit 1 ;; esac
test "$(ps -eo comm= | awk '$1 == "slimski" {n++} END {print n+0}')" = 1
test "$(ps -eo comm= | awk '$1 == "Xorg" {n++} END {print n+0}')" = 1
printf 'POSTRESET_STATE_PASS\n'

stage=build_inventory
capture_file() {
    label=$1
    file=$2
    if [ ! -e "$file" ]; then
        printf 'FILE_ABSENT %s %s\n' "$label" "$file"
    elif [ ! -f "$file" ] || [ ! -r "$file" ]; then
        printf 'FILE_UNREADABLE %s %s\n' "$label" "$file"
    else
        printf 'FILE_BEGIN %s %s\n' "$label" "$file"
        stat -c 'size=%s mtime=%y' "$file"
        sha256sum "$file"
        printf 'BASE64_BEGIN\n'
        base64 -w0 "$file"
        printf '\nBASE64_END\nFILE_END\n'
    fi
}
capture_generated() {
    label=$1
    root=$2
    if [ ! -d "$root/include/generated" ] && [ ! -d "$root/arch/x86/include/generated" ]; then
        printf 'GENERATED_ABSENT %s %s\n' "$label" "$root"
        return
    fi
    set --
    if [ -d "$root/include/generated" ]; then set -- "$@" include/generated; fi
    if [ -d "$root/arch/x86/include/generated" ]; then set -- "$@" arch/x86/include/generated; fi
    printf 'ARCHIVE_BEGIN %s %s\nBASE64_BEGIN\n' "$label" "$root"
    tar --format=gnu --numeric-owner -cf - -C "$root" "$@" | base64 -w0
    printf '\nBASE64_END\nARCHIVE_END\n'
}
printf 'BUILD_LINKS_BEGIN\n'
for name in build source; do
    path=/lib/modules/$release/$name
    if [ -L "$path" ] || [ -e "$path" ]; then
        ls -ld "$path"
        printf '%s_resolved=%s\n' "$name" "$(readlink -f "$path" || true)"
    else
        printf '%s_ABSENT\n' "$name"
    fi
done
printf 'BUILD_LINKS_END\n'
printf 'PACKAGE_METADATA_BEGIN\n'
dpkg-query -W -f='${binary:Package}\t${Version}\t${Architecture}\t${Status}\n' "linux-image-$release" "linux-headers-$release" 2>&1 || true
dpkg-query -S "$module" "/boot/config-$release" "/boot/vmlinuz-$release" 2>&1 || true
printf 'PACKAGE_METADATA_END\n'
capture_file boot-config "/boot/config-$release"
if [ -r "/boot/vmlinuz-$release" ]; then
    printf 'BOOT_IMAGE_HASH\n'
    sha256sum "/boot/vmlinuz-$release"
fi
seen=''
for name in build source; do
    path=/lib/modules/$release/$name
    root=$(readlink -f "$path" || true)
    if [ -z "$root" ] || [ ! -d "$root" ]; then continue; fi
    if [ "$root" = "$seen" ]; then printf 'BUNDLE_ALIAS %s %s\n' "$name" "$root"; continue; fi
    seen=$root
    printf 'BUNDLE_ROOT %s %s\n' "$name" "$root"
    for relative in Module.symvers .config include/config/auto.conf include/config/auto.conf.cmd include/config/kernel.release include/generated/autoconf.h include/generated/utsrelease.h include/generated/compile.h include/generated/utsversion.h Makefile; do
        label=$(printf '%s_%s' "$name" "$relative" | tr '/.' '__')
        capture_file "$label" "$root/$relative"
    done
    printf 'BUNDLE_PACKAGE_OWNERS_BEGIN\n'
    dpkg-query -S "$root/Module.symvers" "$root/.config" "$root/include/generated/autoconf.h" "$root/Makefile" 2>&1 || true
    printf 'BUNDLE_PACKAGE_OWNERS_END\n'
    capture_generated "${name}_generated" "$root"
done
stage=final_state
test "$(cat /proc/sys/kernel/random/boot_id)" = "$boot_id"
test "$(uname -r)" = "$release"
test "$(od -An -tx1 /sys/module/gma500_gfx/notes/.note.gnu.build-id | tr -d ' \n')" = "$expected_note"
test "$(cat /sys/module/gma500_gfx/initstate)" = live
test "$(readlink -f "$pci/driver")" = /sys/bus/pci/drivers/gma500
test "$(readlink -f /sys/class/drm/card0/device)" = /sys/devices/pci0000:00/0000:00:02.0
test "$(cat /sys/class/vtconsole/vtcon1/bind)" = 1
case "$(/usr/bin/sv status "$service")" in run:*) ;; *) exit 1 ;; esac
test "$(ps -eo comm= | awk '$1 == "Xorg" {n++} END {print n+0}')" = 1
printf 'READONLY_CAPTURE_PASS\n'
