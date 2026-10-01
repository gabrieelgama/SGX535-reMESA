# Narrow read-only service-status diagnostic

Capture 05 verified the preserved original module/PCI/DRM/framebuffer/VT
identity, then stopped at `sv status`: `set -e` exited before its assignment
value was printed. The actual status message was therefore not retained.
This observation collects that missing service fact within the operator's
read-only post-reset-state task. No privilege escalation or target repair.

Exact commands are in `capture.sh`: verify the same observed boot ID/release;
read UID, PID 1 and public runsv/slimski/Xorg process metadata; read the
existing service symlink and supervise-directory/status metadata; execute
**status only** through the already specified `/usr/bin/sv status` command,
preserving its stdout/stderr and return code; read boot ID again.
No build inputs will be read by this diagnostic.

Any changed boot/release or script failure stops. Nonzero `sv status` is
captured, public process metadata retained, then propagated as STOP. Do not
convert it to a passing guard, restart a service, use sudo, or infer a
different init service is equivalent. The narrow diagnostic may reveal an
actual state mismatch; then stop the hardware branch after preserving it.

Every target operation is read-only status/metadata. No graphics FD opens,
ioctl, MMIO, service control, module operation, sysfs write, reset, reboot,
network configuration or file installation. One pinned SSH invocation;
no retry or alternative endpoint. Gate B BLOCKED; whitelist `[]`.
