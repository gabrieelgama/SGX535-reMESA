# Read-only capture resumed after evidence-qualified guard correction

The operator authorized the read-only kernel-build observation and requested
another connection. Capture 04 successfully authenticated the pinned host,
observed the expected release/architecture, and stopped in its identity check.
Its raw output is preserved unchanged. This continuation changes no graphics
state and is not a triangle retry or a new risk acceptance.

## Why the old identity check falsely stopped

The old script compared DMI `product_name` with `Inspiron 1210` without its
three trailing spaces. Exact target bytes `Inspiron 1210   \n` were already
recorded in the retained passive preflight and the COREID DMI-fix evidence.
The new guard compares those exact bytes after shell newline removal; it does
not trim or wildcard-match another product.

The old zero-taint requirement was not the retained target baseline.
Capture 04 reports **12289 = 1 + 4096 + 8192**, the kernel's P/O/E flags.
Retained H0 logs show the `wl` out-of-tree/proprietary driver; the Attempt 03
log reports `Tainted: P           OE` before its cleanup warning. The exact
5.10 source `include/linux/kernel.h` and tainted-kernels documentation define
bits 0/12/13 as proprietary/out-of-tree/unsigned modules. This is pre-existing
baseline state, not the warning/oops/forced-operation bits. The corrected
guard requires exactly **12289**, rejecting every other value, including any
new forced load/unload, OOPS, warning, or soft-lockup bit. It does not assert
that the current specific taint source is `wl` without observing it.

The local fixture regression first failed with the unchanged old guards,
then passed after only these two corrected predicates. Four test methods
accept the exact retained identity and reject different DMI bytes, additional
taint bits, different release and different architecture. No target operation
is involved in those tests.

## Exact action / expected outputs / STOP boundaries

`capture.sh` contains all exact commands/read paths; `run_capture.py` uses the
unchanged retained pinned-key/pinned-host SSH command to `192.168.18.90:22`.
One invocation, no automatic retry, no sudo/password transmission. Apart from
the two corrected identity predicates and their comment, the script is
byte-identical to capture 04. Every target command remains read-only.

Read kernel/version/boot/DMI/taint/module identity, PCI/DRM-node metadata,
framebuffer/VT/service/process status. Require the exact original module
identity/hash, normal binding/display ownership and running slimski/Xorg
before any build-bundle read. Missing mandatory evidence, changed identity,
unexpected taint bits, binding/display mismatch, SSH failure, unexpected
stderr, timeout or changed final state STOP collection. No repair/escalation.

After state guards pass, resolve only verified-release build/source links;
read installed package provenance and readable config/Module.symvers/generated
headers/compiler metadata. Stream the explicitly listed files and generated
header archives to local capture only. Missing build inputs are recorded and
retain the ABI blocker. All hashes/archives are checked offline before use.

No service control, module load/unload, PCI/VT binding write, sysfs write,
MMIO, DRM open, ioctl, SGX work, reset, reboot, remote build or file installation
is permitted. Gate B remains BLOCKED; whitelist `[]`. Any recovered bundle
permits offline analysis only, not deployment. Prior captures remain unchanged.
