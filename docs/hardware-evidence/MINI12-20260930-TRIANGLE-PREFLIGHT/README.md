# Frozen-triangle read-only target preflight, 2026-09-30

**TARGET PREFLIGHT: PASS for retained passive identity and display state.**
**ACTIVE SGX ACTION: NOT YET PERFORMED.** No DRM node was opened, module was
replaced, register was accessed, or GPU work was submitted. This does not
establish SGX readiness or module-replacement safety.

The single SSH connection used the retained pinned host key, dedicated
public-key identity, `BatchMode`, strict host-key checking, and unprivileged
`gama`. The exact executed read-only script is
[executed-preflight.sh](executed-preflight.sh), SHA-256
`52f4f1326b8d0af64754716cd1b63f1f9bced7971b8382b66f6b54324aaa3269`.
[mini12-frozen-preflight.sh](../../phase8/mini12-frozen-preflight.sh) is a
subsequent whitespace-only cleanup and was not executed on the target.
[capture.json](capture.json) records the exact SSH argv, local UTC interval,
exit 0, byte counts and hashes. [stdout.txt](stdout.txt) is the unedited
1,640-byte capture; [stderr.txt](stderr.txt) is empty. The host key provides
continuity with the earlier H0 session, not independent device attestation.

The [comparison](comparison.json) passed all 28 checks against the retained
[H0 baseline](../MINI12-20260927-H0/results.md):

| Domain | Live result | Baseline comparison |
| --- | --- | --- |
| Kernel and machine | `5.10.240-antix.1-486-smp`, `i686`; Inspiron 1210, board `0X605H`, BIOS `A02` | Match |
| PCI identity | `0000:00:02.0`, `8086:8108`, PCI rev `0x06`, subsystem `1028:02b1`, class `030000`, IRQ 16 | Match; PCI revision is not CORE_REVISION |
| PCI resources and power | Complete retained text resource table; runtime `active` | Exact resource match; runtime state match |
| Kernel driver | `gma500` bound; `gma500_gfx` loaded and `Live` | Match |
| Loaded module identity | sysfs GNU Build ID `d8dcb4d38b774ad64799d5e13aaedede069371f3` | Exact H0 match |
| Installed module | `/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko`; SHA-256 `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb` | Exact H0 match |
| DRM | `/dev/dri/card0` only, character device major 226/minor 0, bound to PCI `0000:00:02.0` | Match |
| Active display | `gma500drmfb`, 1280×800, 32 bpp, stride 8192; one Xorg process | Match; active-driver replacement still has accepted display risk |

CORE_ID and CORE_REVISION were **not reread**. Their retained separate
observations remain `0x01130000` and `0x00010201`. No target root credential,
display-service shutdown procedure, i386 one-shot client binary, or module
replacement operation was tested in this read-only pass. A host AArch64 client
binary cannot execute on this i686 target. The candidate kernel module remains
local at `/tmp/sgx535-antix-source-6/source/drivers/gpu/drm/gma500/gma500_gfx.ko`,
SHA-256 `934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`,
ELF32 i386 with matching `5.10.240-antix.1-486-smp SMP mod_unload modversions
486` vermagic. It was not transferred or loaded.

## Proposed active operation, not performed

Prepare a target-executable i386 client from the fixed source and verify its
hash and ABI. Verify an operator-controlled privilege and display-service
transition that can stop the single Xorg session, preserve the original
module, and replace `gma500_gfx` **without force**; any refused unload,
module-signature/symbol mismatch, lost binding, or unexpected display state
stops before the fixed ioctl. With the exact candidate module loaded and
PCI/card0 rebound to it, invoke only the root-only fixed ioctl with request
words `{1,1,0,0}` on `/dev/dri/card0`. Kernel action is sequence 1, at most
one TA fire and one raster fire, deadline `5×HZ`, maximum 300,000 status
samples, no automatic retry/reset. Preserve operation status, attributed
events, 32×32 color bytes, logs, and BO/USE/scene ownership on uncertain
execution. The exact active deployment procedure must be reviewed before its
first step. Current Gate B whitelist remains `[]`.
