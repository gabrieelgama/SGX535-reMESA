# Authorized post-reset/build-input observation — capture 08

**READONLY_CAPTURE_PASS**, 2026-09-30 19:10:31–19:10:40 UTC. One pinned SSH
invocation after the operator locally authenticated sudo. Noninteractive sudo
succeeded; no password was transmitted or recorded. Exit 0, stderr empty.
Exact commands/read paths and STOP guards were recorded in [PLAN.md](PLAN.md)
and [capture.sh](capture.sh) before execution. Raw stdout/stderr and transport
hashes are in [capture.json](capture.json); earlier captures are unchanged.

## Independently observed normal state

- Kernel `5.10.240-antix.1-486-smp`, i686; boot ID
  `8ae37532-19d4-4ec7-9c29-a791acfd889f`, kernel build #6.
- Original `gma500_gfx` Live, refcount 2; installed SHA-256
  `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb`;
  loaded Build ID `d8dcb4d38b774ad64799d5e13aaedede069371f3`.
- PCI 8086:8108 at 0000:00:02.0 bound to gma500; card0 associated with that
  device. Only node metadata was read; DRM was not opened.
- gma500drmfb 1280×800, 32 bpp, stride 8192; vtcon0=0, vtcon1=1.
- Root `sv status` reports slimski running; runsv→slimski→Xorg confirmed.
- Retained P/O/E taint baseline 12289; no additional taint bits. The final
  guards retained the same boot/kernel/module/binding/VT/display state.

## Installed build provenance and offline findings

The build link resolves to `/usr/src/linux-headers-5.10.240-antix.1-486-smp`.
No source link exists. dpkg records installed i386 image and header packages
at identical version `5.10.240-antix.1-486-smp-6`, and records ownership of the
original module, boot config/image, Module.symvers and generated headers.
Running `/proc/version` and generated `compile.h` agree on GCC 14.2.0-19,
Binutils 2.44, builder demo@antix1 and the #6 build timestamp.

[Artifact manifest](artifact-manifest.json) records all 11 locally decoded
artifacts. Ten individual files have byte counts and SHA-256 verified against
target stdout; the streamed generated-header tar has a locally computed hash.
The archive has 42 regular-file/directory members confined to the two requested
include/generated trees. No target output file was created.

- Target table SHA-256:
  `faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca`.
- [Reference-table checker](reference-table-check.json): PASS, all 222
  original-module imports covered, zero mismatches; module_layout b84efb99.
- [Candidate/table comparison](candidate-target-table-check.json): all 228
  rejected-candidate imports covered, including all nine candidate-only
  imports; 154 CRC mismatches in the rejected candidate. This is coverage of
  the required target CRCs, **not** candidate ABI PASS.
- Boot config equals header .config and retained public package .config:
  SHA-256 `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9`.
  All 7,384 enabled assignments agree with auto.conf and autoconf.h after
  numeric normalization of two hex-typed zeroes. Makefile matches the retained
  public source. See [build-input check](build-input-check.json).
- Old public table has 25,195 entries versus target 23,920; 17,520 shared
  export CRCs differ. The identical recorded config does **not** explain this
  discrepancy. How that public table was generated remains UNKNOWN.

The exported-CRC provenance and candidate-only coverage blockers are closed.
The rejected artifact is unchanged. No rebuild was performed: local GCC 14.2
is AArch64-only; an i386-targeting GCC build route preserving the captured
GCC configuration is not present. The previous Clang configuration must not
silently replace it. The capture is not a full prepared header-package export
(all include/config stubs, scripts and host helpers were not requested); these
must be prepared consistently from the retained source/config during a
qualified offline build, or supplied by the exact header package.

Full scoped suite: **193 tests PASS**; ten module-version tests PASS. Three
strict host C/UBSan harnesses pass without diagnostics (fixed service, fixed
IO, kernel contract); [record](ubsan-verification.json). Two dry
runs match SHA-256 `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` remains exit 1, PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE.

**No graphics/service/module/sysfs/MMIO/DRM/ioctl/SGX change occurred.** No
remote build, deployment, reset/reboot, staging, commit or push. Gate B remains
BLOCKED, whitelist `[]`; no Attempt 04. Smallest next step: **OFFLINE** —
establish the i386 GCC build route, prepare an isolated build from the retained
source and captured configuration/table, then verify every resulting import.
Hot-transition/recovery qualification remains separate and BLOCKED.
