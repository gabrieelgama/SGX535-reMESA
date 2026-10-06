# Cycle03 read-only capture scope

Before any boot, run one pinned SSH connection and noninteractive sudo root
Python capture derived byte-exact from cycle02 poststage-passive.py. Read uname,
/proc/version/cmdline/boot_id/taint/modules/interrupts/fb, DMI/PCI identity and
existing driver/module/DRM symlinks, loaded Build-ID note, fb/VT metadata and
card0 stat only. Read slimski status, Xorg process list and complete dmesg.
Read/hash the five stock files, GRUB environment via list only, both existing
experimental destinations, incoming-directory inventory, trusted parent
metadata, mountinfo and filesystem free-space/tool availability. No DRM open.

Expected: exact reviewed stock machine/kernel/driver/binding/services/health,
unchanged stock files/default, exact existing staged bytes AND original creation
device/inode receipts. A captured script PASS alone does not pass receipt checks.
Any failed root command, stderr, missing/changed identity, timeout, transport
failure or receipt mismatch aborts before experimental selection; no retry.

Only after preflight PASS and operator menu guards, one manual experimental boot
is permitted. Its one passive capture also reads /run/initramfs/sgx535-first-load.log
and loaded derivative identity, retaining unprivileged boot identity before sudo.
Both experimental and stock-recovery captures use current-boot explicit local
sudo witness, same-boot uptime bounds and operator handoff/completion clocks.
Exactly one stock recovery boot follows the approved operator machine boundary.

All transport/capture commands here are read-only apart from ordinary system
logging intrinsic to SSH/sudo. No payload file transfer, restaging, filesystem
control writes, service/module/PCI/VT operations, MMIO, DRM open or SGX operation.
The only permitted device-state transition is the normal reviewed derivative
probe during one manual experimental boot, then the operator stock boot boundary.
