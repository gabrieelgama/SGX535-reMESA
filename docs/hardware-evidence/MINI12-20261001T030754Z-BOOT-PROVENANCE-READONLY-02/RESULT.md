# Read-only boot-provenance capture 02: PASS

2026-10-01 03:07:54–03:08:59 UTC. Exit 0; stderr empty; initial and final
identity/stock-state guards passed. All 55 framed file payloads independently
match target size and SHA-256. No boot/service/module/PCI/VT changes, DRM open,
MMIO, ioctl or SGX fire. Incidental SSH/sudo accounting is not a graphics/boot
mutation. No files were written or extracted on the target by the script.

Capture 01 stopped before its script at sudo authentication. After the
operator authenticated locally and replied ready, this separate single capture
used the same read-only script. No credentials were transmitted as script input.
Both records are preserved. No further target contact followed this capture.

- [Approved scope and exact read paths](PLAN.md)
- [Exact script](capture.sh), [transport](run_capture.py), [command/exit/hash record](capture.json)
- [Raw stdout](stdout.txt), [stderr](stderr.txt), [readable transcript](transcript-without-base64.txt)
- [Decoded file provenance](artifact-manifest.json); payloads under `decoded-files/`
- [Offline conclusion and proof classes](../../phase8/first-load-stock-boot-observation.md)
- Local parser, tests, independent GNU cpio comparison and stock-dependency
  versioned-import checks are preserved under `offline-analysis/`.

Raw stdout: 70,717,268 bytes, SHA-256
`5fee2f01b32e0316e1d132278e213154a817a60517d28dd082c748fdaf4a2a8b`.
Script SHA-256:
`a2c298389454924c292f9a358b82da762de9d4ff4a5bca76f32b35fae8daab29`.
Stock initrd: 50,863,580 bytes, SHA-256
`f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341`.

The stock initrd has no gma500 module/alias; its udev trigger runs BEFORE
/conf/modules loading. Root eudev modalias coldplug is the supported loader
inference, not a recorded caller PID. GRUB saved stock entry and single stock
kernel/initrd pair are established. A future separate non-saving custom entry
and pre-udev preload image can now be designed; neither was created here.

**Gate B BLOCKED; whitelist `[]`; FIRST TRIANGLE NOT ATTEMPTED.**

## Final offline verification

215 repository tests (zero skips), 14 local analysis-helper tests, generator
check, three strict UBSan harnesses and git diff --check PASS. Two dry runs
retain SHA-256 `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` exits 1 as expected: `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.
[Exact checks](offline-analysis/final-checks/checks.json),
[summary](offline-analysis/final-checks/summary.json),
[independent observation assertions](offline-analysis/verify-boot-findings-result.json),
[candidate identity checks](offline-analysis/pinned-artifact-identities.json).
No production implementation or candidate binary changed.
