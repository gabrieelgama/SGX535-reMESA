# Renewed operator-requested read-only capture

The operator requests another try after the prior connection failures.
Before contact, this capture fixes one fresh connection to the original
reviewed/pinned endpoint 192.168.18.90:22, with the identical script and
STOP guards. No alternate address or automatic retry is permitted. Prior
captures are preserved. No deployment or SGX action is authorized.

# Authorized post-reset / kernel-build observation

Recorded before target contact, 2026-09-30. Authorization is the operator's
read-only evidence-collection request, not a Gate B deployment permission.

## Exact execution and read paths

The executed command list is the adjacent `capture.sh`. It is streamed to
`sh -s` as unprivileged `gama` using the same pinned-key, pinned-host SSH
connection used by the retained passive preflight. No sudo credential is
needed or transmitted. `run_capture.py` preserves script/stdout/stderr hashes,
exit status and UTC timestamps locally. There are no remote output files.

1. Read `uname -r/-m/-v`, `/proc/version`, boot ID, kernel taint, public DMI,
   `/proc/modules`, loaded module initstate/build-ID note and the installed
   original module's SHA-256 and `modinfo`. Inspect PCI identity/driver link,
   DRM-node metadata **with stat only**, fbdev parameters, VT-console binding
   files, `sv status` (status only) and public display process metadata.
2. Require the retained release/architecture/original-module identity, normal
   gma500 PCI/DRM binding, expected framebuffer/VT ownership, running slimski
   and one Xorg. Any material mismatch stops before build-bundle collection.
3. Resolve only `/lib/modules/<verified-release>/{build,source}`. Read/hash
   `/boot/{config-,vmlinuz-}<release>` when readable. Query the installed
   image/header package ownership/version using `dpkg-query` only.
4. For existing build/source directories, read/hash/capture the explicitly
   listed `Module.symvers`, `.config`, generated config/release/compiler
   metadata and Makefile. Stream archives of only `include/generated` and
   `arch/x86/include/generated` if readable. `tar -cf -` and `base64` write
   **only stdout**, never a target file. Missing inputs are recorded as
   ABSENT/UNREADABLE and never synthesized.
5. Check boot ID, release, module identity and core display/service bindings
   again at the end. A changed state stops with the partial capture preserved.

## Expected output and STOP conditions

Expected: release `5.10.240-antix.1-486-smp`, architecture `i686`, Inspiron
1210, original module Live with Build ID
`d8dcb4d38b774ad64799d5e13aaedede069371f3`, installed SHA-256
`7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb`, PCI
`8086:8108` bound to gma500, card0 associated with that PCI device,
gma500drmfb at 1280×800/32bpp/8192 stride, vtcon0=0/vtcon1=1, slimski running
and one Xorg. Refcount/PIDs/uptime are recorded dynamic values. Build paths
and bundle availability are observations, not assumed results.

STOP: SSH identity/authentication failure, unexpected script error/stderr,
missing mandatory state observation, any state/identity/binding mismatch,
nonzero kernel taint, changed boot ID or capture timeout. Preserve all bytes;
do not retry, repair, escalate privilege or continue target investigation.
Missing optional build files retain the ABI blocker but are not a display
state mismatch. Archive/individual-file hashes must verify offline; incomplete
or unattributable artifacts are rejected.

## Read-only confirmation

All target commands are metadata/status/file reads or stdout-only encoders.
No DRM node is opened. No MMIO/ioctl is used. No service control, module
load/unload, PCI/VT binding write, sysfs write, reset, reboot, file installation
or remote build occurs. Normal SSH/accounting and read access bookkeeping are
incidental to the authorized observation, not graphics-state operations.
Gate B remains BLOCKED; whitelist `[]`; Attempt 04 is prohibited.
