#!/bin/sh
# Read-only Mini 12 preflight for the single frozen first-triangle review.
# Run only through the retained pinned-key SSH connection as unprivileged gama.
set -eu

emit_file() {
    printf '%s=' "$1"
    cat "$2"
}

printf 'kernel_release=%s\n' "$(uname -r)"
printf 'machine_arch=%s\n' "$(uname -m)"
emit_file product_name /sys/class/dmi/id/product_name
emit_file board_name /sys/class/dmi/id/board_name
emit_file bios_version /sys/class/dmi/id/bios_version

device=/sys/bus/pci/devices/0000:00:02.0
for field in vendor device revision subsystem_vendor subsystem_device class irq; do
    emit_file "pci_$field" "$device/$field"
done
printf 'pci_driver=%s\n' "$(readlink -f "$device/driver")"
printf 'pci_runtime_status=%s\n' "$(cat "$device/power/runtime_status")"
printf 'pci_resource_start\n'
cat "$device/resource"
printf 'pci_resource_end\n'

printf 'loaded_gma500=' 
awk '$1 == "gma500_gfx" { print $1 ":" $3 ":" $5; found=1 } END { if (!found) exit 1 }' /proc/modules
printf 'loaded_gma500_build_note=' 
od -An -tx1 /sys/module/gma500_gfx/notes/.note.gnu.build-id | tr -d ' \n'
printf '\n'
module=/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko
printf 'installed_module_sha256=' 
sha256sum "$module"

printf 'drm_card_device=%s\n' "$(readlink -f /sys/class/drm/card0/device)"
printf 'drm_card_node=' 
stat -c '%F:%t:%T' /dev/dri/card0
printf 'drm_node_names=' 
find /dev/dri -maxdepth 1 -type c -printf '%f '
printf '\n'
printf 'proc_fb=' 
cat /proc/fb
emit_file fb_name /sys/class/graphics/fb0/name
emit_file fb_virtual_size /sys/class/graphics/fb0/virtual_size
emit_file fb_bits_per_pixel /sys/class/graphics/fb0/bits_per_pixel
emit_file fb_stride /sys/class/graphics/fb0/stride
printf 'xorg_count=' 
ps -eo comm= | awk '$1 == "Xorg" { count++ } END { print count+0 }'
