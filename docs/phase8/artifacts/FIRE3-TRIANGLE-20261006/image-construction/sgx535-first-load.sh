#!/bin/sh
# Fixed first-owner preload. Generated payload hashes; no caller-selected paths.
PATH=/sbin:/usr/sbin:/bin:/usr/bin
export PATH
sgx_setup_log() {
    umask 077
    exec >>/run/initramfs/sgx535-first-load.log 2>&1
}
sgx_uname() { /bin/uname "$1"; }
sgx_pci_paths() { printf '%s\n' /sys/bus/pci/devices/*; }
sgx_read_file() { /bin/cat "$1"; }
sgx_has_path() { [ -e "$1" ] || [ -L "$1" ]; }
sgx_listed() { /bin/grep -Eq "^$1 " /proc/modules; }
sgx_irq_present() { /bin/grep -Eq '[[:space:],]gma500([,[:space:]]|$)' /proc/interrupts; }
sgx_irq_owned() { /bin/grep -Eq '^[[:space:]]*16:.*[[:space:],]gma500([,[:space:]]|$)' /proc/interrupts; }
sgx_link_is() {
    actual=$(/bin/readlink -f "$1") || return 1
    expected=$(/bin/readlink -f "$2") || return 1
    [ "$actual" = "$expected" ]
}
sgx_hash_ok() {
    digest=$(/bin/sha256sum "$1") || return 1
    [ "${digest%% *}" = "$2" ]
}
sgx_insert_file() { /sbin/insmod "$1"; }
sgx_refuse() { printf 'SGX535-FIRSTLOAD HOLD: %s\n' "$1"; return 1; }
sgx_first_load_main() {
    sgx_setup_log || return 1
    printf 'SGX535-FIRSTLOAD BEGIN\n'
    release=$(sgx_uname -r) || return 1
    [ "$release" = '5.10.240-antix.1-486-smp' ] || { sgx_refuse kernel; return 1; }
    arch=$(sgx_uname -m) || return 1
    [ "$arch" = i686 ] || { sgx_refuse architecture; return 1; }
    machine=$(sgx_read_file /sys/class/dmi/id/product_name) || return 1
    machine=${machine%"${machine##*[! ]}"}
    [ "$machine" = "Inspiron 1210" ] || { sgx_refuse machine; return 1; }
    for pair in vendor:0x8086 device:0x8108 subsystem_vendor:0x1028 subsystem_device:0x02b1; do
        field=${pair%%:*}; expected=${pair#*:}
        value=$(sgx_read_file "/sys/bus/pci/devices/0000:00:02.0/$field") || return 1
        [ "$value" = "$expected" ] || { sgx_refuse pci-identity; return 1; }
    done
    paths=$(sgx_pci_paths) || return 1
    for pci_path in $paths; do
        vendor=$(sgx_read_file "$pci_path/vendor") || return 1
        device=$(sgx_read_file "$pci_path/device") || return 1
        case "$vendor:$device" in
            0x8086:0x0be0|0x8086:0x0be1|0x8086:0x0be2|0x8086:0x0be3|0x8086:0x0be4|0x8086:0x0be5|0x8086:0x0be6|0x8086:0x0be7|0x8086:0x0be8|0x8086:0x0be9|0x8086:0x0bea|0x8086:0x0beb|0x8086:0x0bec|0x8086:0x0bed|0x8086:0x0bee|0x8086:0x0bef|0x8086:0x4100|0x8086:0x4101|0x8086:0x4102|0x8086:0x4103|0x8086:0x4104|0x8086:0x4105|0x8086:0x4106|0x8086:0x4107|0x8086:0x4108|0x8086:0x8108|0x8086:0x8109)
                [ "$pci_path" = /sys/bus/pci/devices/0000:00:02.0 ] || { sgx_refuse additional-matching-device; return 1; } ;;
        esac
    done
    for resource in /sys/class/drm/card0 /sys/class/graphics/fb0; do
        sgx_has_path "$resource" && { sgx_refuse preexisting-display-resource; return 1; }
    done
    cmdline=$(sgx_read_file /proc/cmdline) || return 1
    for arg in $cmdline; do
        case "$arg" in
            module_blacklist=*|modprobe.blacklist=*|blacklist=*|nomodeset|break|break=*)
                sgx_refuse unexpected-loading-policy; return 1 ;;
        esac
    done
    sgx_read_file /proc/modules >/dev/null || return 1
    sgx_has_path /sys/bus/pci/devices/0000:00:02.0/driver && { sgx_refuse already-bound; return 1; }
    for name in drm syscopyarea sysfillrect sysimgblt fb_sys_fops cec drm_kms_helper video i2c_algo_bit sgx535_provenance gma500_gfx; do
        if sgx_has_path "/sys/module/$name" || sgx_listed "$name"; then
            sgx_refuse preexisting-module; return 1
        fi
    done
    sgx_irq_present
    irq_result=$?
    [ "$irq_result" = 1 ] || { sgx_refuse preexisting-or-unreadable-irq; return 1; }
    sgx_hash_ok /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/drm.ko d1d7bf85268e40bc0fe30ac98f5083cd06ba16f0d10fc38ff810f974f22229c3 || { sgx_refuse payload-drm; return 1; }
    sgx_hash_ok /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/video/fbdev/core/syscopyarea.ko 073ef524e5d71941b894c62e567672a2912059bccdf6b7bc4f84e550876d4a00 || { sgx_refuse payload-syscopyarea; return 1; }
    sgx_hash_ok /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/video/fbdev/core/sysfillrect.ko 2ba264e98a234ee5eb51fd34fa60bcb9ef56a85db4b640422da9aa229c3747ed || { sgx_refuse payload-sysfillrect; return 1; }
    sgx_hash_ok /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/video/fbdev/core/sysimgblt.ko c39c1a63a5f072333050cae22f9699c37f89358d956c81cab6bb1765ec1d0a39 || { sgx_refuse payload-sysimgblt; return 1; }
    sgx_hash_ok /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/video/fbdev/core/fb_sys_fops.ko e9a32aa2e359917fc61d056d84c931d10504ff228fee50c10566697265e37402 || { sgx_refuse payload-fb_sys_fops; return 1; }
    sgx_hash_ok /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/media/cec/core/cec.ko 33c9184040ca309997146c1cb2545026029f399f85cd011dbc6aed1b6e09d9e8 || { sgx_refuse payload-cec; return 1; }
    sgx_hash_ok /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/drm_kms_helper.ko 9757eb58238b9166485ed78f48a4004e6a31ab1e116aa6aa2eec9df7ade21076 || { sgx_refuse payload-drm_kms_helper; return 1; }
    sgx_hash_ok /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/acpi/video.ko 83d359cb34190ff9833ddf101d99c02a443677dce37c7e526243be0ecd0e6f8e || { sgx_refuse payload-video; return 1; }
    sgx_hash_ok /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/i2c/algos/i2c-algo-bit.ko 3179bdd4f98d3ea876531257f75db850608e2e983e7cb1626406e89df04de1b7 || { sgx_refuse payload-i2c_algo_bit; return 1; }
    sgx_hash_ok /usr/lib/sgx535-first-load/sgx535_provenance.ko 2e35e863d51dbc1d407feef08f08bb2edc6390af621a0f173a9b1a73ec76158a || { sgx_refuse payload-sgx535_provenance; return 1; }
    sgx_hash_ok /usr/lib/sgx535-first-load/gma500_gfx.ko c925caedebcc0699aef53227d29e64b47bf2833efd9d2e611cba276dd3a08ab3 || { sgx_refuse payload-gma500_gfx; return 1; }
    printf 'SGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE\n'
    sgx_insert_file /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/drm.ko || { sgx_refuse insertion-drm; return 1; }
    sgx_insert_file /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/video/fbdev/core/syscopyarea.ko || { sgx_refuse insertion-syscopyarea; return 1; }
    sgx_insert_file /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/video/fbdev/core/sysfillrect.ko || { sgx_refuse insertion-sysfillrect; return 1; }
    sgx_insert_file /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/video/fbdev/core/sysimgblt.ko || { sgx_refuse insertion-sysimgblt; return 1; }
    sgx_insert_file /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/video/fbdev/core/fb_sys_fops.ko || { sgx_refuse insertion-fb_sys_fops; return 1; }
    sgx_insert_file /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/media/cec/core/cec.ko || { sgx_refuse insertion-cec; return 1; }
    sgx_insert_file /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/drm_kms_helper.ko || { sgx_refuse insertion-drm_kms_helper; return 1; }
    sgx_insert_file /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/acpi/video.ko || { sgx_refuse insertion-video; return 1; }
    sgx_insert_file /usr/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/i2c/algos/i2c-algo-bit.ko || { sgx_refuse insertion-i2c_algo_bit; return 1; }
    sgx_insert_file /usr/lib/sgx535-first-load/sgx535_provenance.ko || { sgx_refuse insertion-sgx535_provenance; return 1; }
    state=$(sgx_read_file /sys/module/sgx535_provenance/initstate) || return 1
    [ "$state" = live ] || { sgx_refuse provenance-not-live; return 1; }
    sgx_hash_ok /sys/module/sgx535_provenance/notes/.note.gnu.build-id 6afbac1cf48a211135e2a4b4ffc30f96ace64f6d397071595bbc328966bb8af0 || { sgx_refuse provenance-identity; return 1; }
    sgx_insert_file /usr/lib/sgx535-first-load/gma500_gfx.ko || { sgx_refuse insertion-gma500_gfx; return 1; }
    state=$(sgx_read_file /sys/module/gma500_gfx/initstate) || return 1
    [ "$state" = live ] || { sgx_refuse module-not-live; return 1; }
    sgx_hash_ok /sys/module/gma500_gfx/notes/.note.gnu.build-id 4499399a9f06a35b924c770ec5f04b8e5a5170c4f744359c675d13ca89223644 || { sgx_refuse loaded-identity; return 1; }
    sgx_link_is /sys/bus/pci/devices/0000:00:02.0/driver /sys/bus/pci/drivers/gma500 || { sgx_refuse driver-binding; return 1; }
    sgx_link_is /sys/bus/pci/drivers/gma500/module /sys/module/gma500_gfx || { sgx_refuse driver-owner; return 1; }
    sgx_link_is /sys/class/drm/card0/device /sys/bus/pci/devices/0000:00:02.0 || { sgx_refuse drm-binding; return 1; }
    fb=$(sgx_read_file /sys/class/graphics/fb0/name) || return 1
    [ "$fb" = gma500drmfb ] || { sgx_refuse framebuffer; return 1; }
    irq=$(sgx_read_file /sys/bus/pci/devices/0000:00:02.0/irq) || return 1
    [ "$irq" = 16 ] || { sgx_refuse pci-irq; return 1; }
    sgx_irq_owned || { sgx_refuse handler-absent; return 1; }
    printf 'SGX535-FIRSTLOAD PASS: derivative first owner; boot may continue\n'
}
sgx_first_load_main
