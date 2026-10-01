# Boot-provenance read-only observation 01 — privilege STOP

2026-10-01 03:03:49 UTC. **STOP before remote capture script execution.**
One pinned SSH connection reached the retained endpoint, but noninteractive
sudo returned exit1: `sudo: a password is required`. stdout is empty; stderr
is29 bytes. No credential bytes were sent, requested or recorded. The operator
was asked to authenticate sudo locally before any new capture; this record
will not be retried or overwritten.

## What was and was not established

- SSH continuity to the previously pinned server was observed.
- The root shell/capture script did not run. Dell identity, current kernel,
  module, display, command line and boot files were **not** freshly verified.
- No initramfs, bootloader, module-policy or log artifact was collected.
- Target boot/loading/fallback questions therefore retain their previous
  UNKNOWN classifications. Current stock state cannot be inferred from SSH
  reachability or authentication denial.
- No display/service/module/VT/PCI/boot changes, output-file creation,
  DRM open, MMIO/ioctl, SGX fire, reset/reboot, staging or deployment occurred.
  Incidental SSH/sudo logging is observation bookkeeping, not a graphics or
  boot-state operation.

`PLAN.md` and `capture.sh` froze read paths and STOP guards before contact.
`capture.json` records exact argv, UTC times, exit status and stream hashes.
No fallback privilege mechanism or automatic retry was attempted.

**First-load alternative/fallback UNKNOWN/BLOCKED. Gate B BLOCKED.
Whitelist[]; FIRST TRIANGLE NOT ATTEMPTED.**

Smallest prerequisite: operator-local `sudo -v`, with password entered only
locally. If the operator directs continuation, preserve this failed capture
and use a new evidence record for the same authorized read-only scope. No
module or SGX action follows from enabling read privilege.
