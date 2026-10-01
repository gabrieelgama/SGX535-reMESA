#!/bin/sh
# Read-only; all output streams back to the local evidence recorder.
set -eu
stage=identity
test "$(id -u)" = 0
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
service_rc=0
service_status=$(/usr/bin/sv status "$service") || service_rc=$?
printf 'service_status=%s\nservice_exit=%s\n' "$service_status" "$service_rc"
test "$service_rc" -eq 0
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


stage=boot_provenance
printf 'CMDLINE_BEGIN\n'
cat /proc/cmdline
printf 'CMDLINE_END\n'
expected_cmdline='BOOT_IMAGE=/boot/vmlinuz-5.10.240-antix.1-486-smp root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro quiet selinux=0'
test "$(cat /proc/cmdline)" = "$expected_cmdline"
printf 'MODALIAS=%s\n' "$(cat "$pci/modalias")"
cat /proc/interrupts
cat /proc/mounts
readlink -f /sbin/init

capture_file() {
    label=$1
    file=$2
    limit=${3:-8388608}
    if [ ! -e "$file" ]; then
        printf 'FILE_ABSENT %s %s\n' "$label" "$file"
    elif [ ! -f "$file" ] || [ ! -r "$file" ]; then
        printf 'FILE_UNREADABLE %s %s\n' "$label" "$file"
    else
        printf 'FILE_BEGIN %s %s\n' "$label" "$file"
        stat -c 'size=%s mtime=%y mode=%a' "$file"
        sha256sum "$file"
        size=$(stat -c %s "$file")
        if [ "$size" -gt "$limit" ]; then
            printf 'FILE_LIMIT %s\nFILE_END\n' "$limit"
        else
            printf 'BASE64_BEGIN\n'
            base64 -w0 "$file"
            printf '\nBASE64_END\nFILE_END\n'
        fi
    fi
}
capture_dir() {
    directory=$1
    if [ ! -d "$directory" ]; then printf 'DIR_ABSENT %s\n' "$directory"; return; fi
    printf 'DIR_BEGIN %s\n' "$directory"
    find "$directory" -maxdepth 1 \( -type f -o -type l \) -print | LC_ALL=C sort | while IFS= read -r file; do
        label=$(printf '%s' "$file" | tr '/.' '__')
        capture_file "$label" "$file"
    done
    printf 'DIR_END\n'
}
printf 'BOOT_DIRECTORY_BEGIN\n'
ls -ld /boot /boot/grub /boot/syslinux /boot/extlinux /boot/loader /etc/grub.d 2>&1 || true
find /boot -maxdepth 2 -type f -printf '%p\t%s\t%TY-%Tm-%Td %TH:%TM:%TS\n' | LC_ALL=C sort
printf 'BOOT_DIRECTORY_END\n'
for file in /boot/grub/grub.cfg /boot/grub/grubenv /etc/default/grub /boot/syslinux/syslinux.cfg /boot/extlinux/extlinux.conf /boot/loader/loader.conf; do
    label=$(printf '%s' "$file" | tr '/.' '__')
    capture_file "$label" "$file"
done
capture_dir /etc/grub.d
capture_dir /boot/loader/entries
printf 'KERNEL_IMAGE_METADATA\n'
stat -c 'size=%s mtime=%y mode=%a' "/boot/vmlinuz-$release"
sha256sum "/boot/vmlinuz-$release"
# Associated release image is not relabelled as actual selected initrd until
# offline boot-entry/cmdline analysis establishes the relationship.
capture_file release_initrd "/boot/initrd.img-$release" 268435456
printf 'BOOT_SYMLINKS_BEGIN\n'
for path in /vmlinuz /initrd.img /vmlinuz.old /initrd.img.old /boot/vmlinuz /boot/initrd.img; do
    if [ -e "$path" ] || [ -L "$path" ]; then ls -ld "$path"; readlink -f "$path" || true; else printf 'LINK_ABSENT %s\n' "$path"; fi
done
printf 'BOOT_SYMLINKS_END\n'
printf 'PACKAGE_VERSIONS_BEGIN\n'
dpkg-query -W -f='${binary:Package}\t${Version}\t${Architecture}\t${Status}\n' "linux-image-$release" grub-pc grub-common grub2-common initramfs-tools initramfs-tools-core kmod udev eudev runit runit-init openssh-server 2>&1 || true
dpkg-query -S "/boot/vmlinuz-$release" "$module" /usr/share/initramfs-tools/init /usr/lib/udev/rules.d/80-drivers.rules /lib/udev/rules.d/80-drivers.rules 2>&1 || true
printf 'PACKAGE_VERSIONS_END\n'

stage=module_boot_policy
for file in /etc/modules /etc/initramfs-tools/initramfs.conf /etc/initramfs-tools/modules /etc/default/kmod /etc/default/udev /etc/default/networking /etc/init.d/kmod /etc/init.d/udev /etc/init.d/networking /etc/init.d/ssh /etc/runit/1 /usr/share/initramfs-tools/init /usr/share/initramfs-tools/hooks/framebuffer /usr/share/initramfs-tools/hooks/udev /usr/share/initramfs-tools/scripts/init-top/udev /usr/share/initramfs-tools/scripts/init-top/blacklist /etc/runit/runsvdir/default/slimski/run /run/initramfs/initramfs.debug /run/initramfs/initramfs.conf; do
    label=$(printf '%s' "$file" | tr '/.' '__')
    capture_file "$label" "$file"
done
for directory in /etc/modules-load.d /lib/modules-load.d /usr/lib/modules-load.d /etc/modprobe.d /lib/modprobe.d /usr/lib/modprobe.d /etc/initramfs-tools/conf.d /etc/initramfs-tools/hooks /etc/initramfs-tools/scripts/init-top /etc/initramfs-tools/scripts/init-premount /etc/initramfs-tools/scripts/init-bottom /etc/runit/1.d /etc/runit/runsvdir/default/ssh /etc/sv/ssh; do
    capture_dir "$directory"
done
for file in modules.alias modules.dep modules.softdep modules.builtin; do
    label=$(printf 'modules_%s' "$file" | tr '.' '_')
    capture_file "$label" "/lib/modules/$release/$file"
done
for directory in /etc/udev/rules.d /lib/udev/rules.d; do
    if [ ! -d "$directory" ]; then printf 'DIR_ABSENT %s\n' "$directory"; continue; fi
    find "$directory" -maxdepth 1 -type f -print | LC_ALL=C sort | while IFS= read -r file; do
        if grep -qE 'MODALIAS|kmod|modprobe|gma500|modules-load' "$file"; then
            label=$(printf '%s' "$file" | tr '/.' '__')
            capture_file "$label" "$file"
        fi
    done
done

stage=boot_logs_network
printf 'DMESG_BEGIN\n'
dmesg
printf 'DMESG_END\n'
for file in /var/log/boot /var/log/boot.log /var/log/kern.log /var/log/syslog /var/log/dmesg; do
    label=$(printf '%s' "$file" | tr '/.' '__')
    capture_file "$label" "$file"
done
printf 'NETWORK_BEGIN\n'
if command -v ip >/dev/null 2>&1; then ip -brief address; ip route; else cat /proc/net/route; fi
if command -v ss >/dev/null 2>&1; then ss -lntp; fi
ps -eo pid=,ppid=,comm=,args= | awk '$3 == "sshd" || $3 == "sshd-session" || $3 == "sshd-auth" || ($3 == "runsv" && $0 ~ /ssh/)'
printf 'NETWORK_END\n'

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
printf 'BOOT_PROVENANCE_READONLY_PASS\n'
