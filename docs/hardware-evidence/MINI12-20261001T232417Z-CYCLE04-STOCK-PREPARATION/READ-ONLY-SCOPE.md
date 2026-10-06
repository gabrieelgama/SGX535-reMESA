# Exact Cycle04 STOCK preparation read-only scope

Recorded before contact. The reviewed pinned SSH prefix/known-host identity is
reused. Exact command/source is saved in command.json and the pinned root/wrapper
files. Python stdin is /dev/null. Root runs once via sudo -n with no password.
No credential transport, shell stdin or retries.

The root source retains Cycle03 stock-existing-preflight.py: uname -r/-m;
sv status of slimski; pgrep -a Xorg; dmesg; grub-editenv list only; id -u/-g.
Read /proc/version, cmdline, boot_id, tainted, modules, interrupts, fb, mounts and
mountinfo. Read DMI product_name; PCI identity/irq; PCI/driver/module and card0
symlink targets; module initstate and loaded Build-ID note; framebuffer name/size;
VT bind values READ ONLY. lstat card0; never open it. Hash/read only the five
stock files, existing experimental image/custom.cfg and their creation identities;
inspect incoming directory contents/type, /boot parents/statvfs/access flags.
No writing to target files or controls.

Added clock evidence: /proc/uptime, /proc/stat, os.sysconf(SC_CLK_TCK), PID1,
current Xorg and slimski /proc/PID/stat; read the current time-namespace link and
offsets if present. These are process-start observations, not userspace-readiness
timestamps or a measured HDD boot duration. No reboot/timing experiment.

Expected: current original driver identity, target stock ownership/services/
health/default/files pass all original guards; automatic prefix/root/end identify
one boot; same-boot clocks do not regress; root child <=35s and full connection
<=40s. STOCK age may exceed120s; this check cannot qualify a historical cycle.

STOP on any failed identity/health/file/prerequisite, unexpected stderr, missing
record, clock regression, privilege failure or timeout. Preserve partial evidence;
no automatic retry, force, repair, service/module/PCI/VT change or reboot.
Gate B BLOCKED; SGX whitelist[]. No SGX command/ioctl/MMIO action.
