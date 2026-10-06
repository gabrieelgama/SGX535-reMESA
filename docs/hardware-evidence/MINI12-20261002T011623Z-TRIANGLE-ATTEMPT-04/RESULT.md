# Attempt04: STOP before ioctl

The operator authorized one exact off-screen frozen SGX attempt. The fresh
sameboot preflight passed56 guards. Exclusive staging/read-back of the static
i386 client passed with SHA-256
`758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf`.
The root-owned0700 directory/client are preserved at
`/root/sgx535-frozen-seq1-29e27f75`.

The sole execution connection returned `STOP BEFORE IOCTL`,
`client_invocation_started=false`, and `AssertionError()`. No client invocation,
DRM open, fixed ioctl, SGX service operation, TA/raster fire or color output ran.
There was no retry, hot operation, reset or reboot.

The new wrapper contains a proven canonical-path bug: it compares
`realpath(/sys/class/drm/card0/device)` with the uncanonicalized bus alias
`/sys/bus/pci/devices/0000:00:02.0`. Retained target output explicitly records
`/sys/devices/pci0000:00/0000:00:02.0` as the canonical DRM device. The qualified
hook already resolves both sides. The incorrect assertion must reject the valid
layout. The abbreviated error does not identify its line, so earlier runtime
checks cannot be independently attributed from this failed wrapper alone;
all corresponding predicates passed the preceding captured preflight.

The corrected wrapper resolves both sides. An offline check of the actual
assertion reproduces rejection of the valid retained mapping before the fix,
accepts that mapping after the fix and still rejects a different PCI device.
`execution-root-source-corrected-NOT-RUN.py` is prepared, compiled and retained.
It was NOT executed remotely. The original executed source/output is unchanged.

Gate B exact off-screen readiness remains PASS; the qualified artifacts and
first-owner proof are unchanged. The no-retry execution boundary now stops any
further connection/invocation under this attempt. The whitelist identifier is
`MINI12-SGX535-REV121-FROZEN-32x32-SEQ1`; it is not retry permission. Current target
was not rebooted or manipulated: last verified EXPERIMENTAL boot remains
`29e27f75-7c84-4537-9ab8-8138bc3eac1d`. No SGX failure recovery is needed because
execution never started. No post-stop physical display/health observation is claimed.
FIRST TRIANGLE: NOT ATTEMPTED. No completion or readback.

Minimum next authorization: fresh sameboot read-only checks, reuse and verify the
already exclusively staged client, then ONE corrected-wrapper invocation of the
same fixed ioctl. No restaging, SGX retry, hot replacement, reset or LCD handoff.
