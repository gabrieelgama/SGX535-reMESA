# Exact future staging operation — before execution

The already authorized reviewed file mechanism is implemented literally:
root read-only full preflight again,then exclusive private incoming directory
and two fixed files,then exclusive boot image,then exclusive custom.cfg.
Fixed bytes/hashes only; no target path/command supplied by userspace. All writes
are ordinary file writes,with no-follow directory descriptors,exclusive creates,
file+directory fsync,close/reopen complete readback and ownership/inode receipts.
Private files mode0600/dir0700 gama;boot files root:root0644,single link.
Read-only stock guards run again after stage; stock/default/env bytes cannot change.

Transport is pinned SSH with root sudo -n -p empty prompt python3 -I -B -S -c
quoted fixed reviewed program. Script is command argument;stdin is only the two
fixed binary payloads. There are no credentials anywhere; cached/uncached sudo
cannot deliver data to shell input. Passive subprocess stdin is DEVNULL.
Any mismatch/write/sync/identity error STOP/HOLD;no cleanup/retry/reboot.
No service,VT,module,PCI,DRM-open,SGX or bootloader-generation command is executed.
No software shutdown/reset command is introduced. Manual boot is separately gated.
