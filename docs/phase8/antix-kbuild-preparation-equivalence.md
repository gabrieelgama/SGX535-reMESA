# Isolated antiX Kbuild preparation/equivalence — 2026-09-30

**CONFIG SEMANTICS: PASS except the documented compiler-name banner.**
**GENERATED-HEADER EQUIVALENCE: FAIL / HARD STOP.**
**NEW CANDIDATE: NOT BUILT. TARGET CONTACT: NONE.**
**Gate B: BLOCKED. Whitelist: `[]`.**

This executes only steps 1–5 of the
[offline build plan](i386-gcc-offline-build-plan.md), using the already
[qualified toolchain](i386-gcc-toolchain-qualification.md). The generated-header
guard caught truncated syscall definitions. No candidate compilation, table
substitution, target operation, staging, commit or push followed the failure.
The failed prepared output is retained unchanged.

## Inputs and isolated paths — CONFIRMED BY LOCAL EVIDENCE

[Evidence index](artifacts/antix-kbuild-preparation-20260930/README.md) and
[input hashes](artifacts/antix-kbuild-preparation-20260930/inputs.json) preserve
the actual inputs; all were checked before use and remain unchanged.

| Input | SHA-256 |
| --- | --- |
| Exact `linux-5.10.240-antix.1-486-smp_5.10.240-antix.1-486-smp.orig.tar.gz`, retained under `/tmp/sgx535-antix-source-6/` | `e5c5d7c6bdcc7a845589e5956cbf50a8d6198232df017f0398a2902371042e1d` |
| Retained source-package `-6.diff.gz` under `docs/phase4-2-data/` | `5c41ae456d77d9e0af181fe3153c4f80e89bacdd2b541ba15fbac0b297d69b03` |
| Capture-08 `build__config` | `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9` |
| Capture-08 target `build_Module_symvers` | `faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca` |
| Capture-08 `build_generated.tar` | `f899bce235cc04a3aa3c74f569fe2ba793a0b606676144ff762a57be2aa1f45c` |
| Capture-08 `build_Makefile` | `70b5c7688f9067bd13b8413a205dc73ba9f66676b3de34c4092d7ceb9c260493` |

Let `W=/home/gama/sgx535-offline/antix-kbuild-preparation-20260930`:

- Source: `W/source-unpack/linux-5.10.240-antix.1-486-smp`.
- Output: `W/output`.
- Work/logs: `W`; captured comparison headers: `W/captured-generated`.
- Toolchain root `T=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root`.

The original archive was extracted afresh after checking confined paths and
symlink targets. The exact package diff passed dry-run and applied with
`--batch --fuzz=0 -p1`, without rejects. The reconstructed Makefile byte-matches
capture 08. Package-generated old `Module.symvers` (`f4732fe6…`) was recorded,
then removed by normal `mrproper` **in this disposable source only**. Neither
the rejected candidate tree nor retained hardware evidence was changed.

## Actual tools and preparation commands

[Environment](artifacts/antix-kbuild-preparation-20260930/build-environment.json)
and command JSON/logs preserve full absolute argv and rootless native dependency
paths. Every make invocation selected:

```text
ARCH=x86
CROSS_COMPILE=T/usr/bin/i686-linux-gnu-
CC=T/usr/bin/i686-linux-gnu-gcc-14 --sysroot=T
LD=T/usr/bin/i686-linux-gnu-ld.bfd
AS=T/usr/bin/i686-linux-gnu-as
HOSTCC=/usr/bin/gcc-14
LOCALVERSION=-486-smp
V=1
```

`T` above denotes the absolute root, not a literal variable passed to make.
Native HOSTCFLAGS/HOSTLDFLAGS point to qualified ARM64 dependency headers and
libraries; they are not target compiler flags. Recorded PATH, runtime library,
bison data and m4 paths were used. No Clang, ARM multilib or AArch64 `-m32`.
Target GCC is 14.2.0/i686; GNU target assembler/linker are 2.44. Native HOSTCC
is GCC 14.2.0/AArch64. The actual cross compiler **banner** contains Debian
`14.2.0-19`; its **package version** is `14.2.0-19cross1`.

| Operation | Observed result |
| --- | --- |
| `make -C SOURCE ... mrproper` | Exit 0; fresh source only |
| Copy captured config to fresh `OUTPUT/.config` | Exact input bytes before configuration |
| `make -C SOURCE O=OUTPUT ... olddefconfig` | Exit 0; no stderr; only compiler-banner value changes |
| Initial `modules_prepare` invocation | Interrupted/incomplete; exit record unavailable, exact cause UNKNOWN |
| Resumed same `modules_prepare` | Exit 0; no stderr; skipped existing truncated syscall header |
| Diagnostic syscall generation into separate `W/syscall-diagnostic/` | Exit 0; byte-exact target header; failed output not replaced |

Actual preparation outputs establish compiler-role separation:
`scripts/basic/fixdep`, `scripts/kconfig/conf` and `scripts/mod/modpost` are
ELF64 little-endian AArch64; target `scripts/mod/empty.o` is ELF32 little-endian
Intel 80386. Verbose preparation compilation selects qualified target GCC with
`-m32 -march=i486`. These are preparation artifacts, not an SGX module build.

## Full comparison and every difference

[Raw comparison](artifacts/antix-kbuild-preparation-20260930/raw-generated-comparison.json),
[classifications](artifacts/antix-kbuild-preparation-20260930/classified-comparison.json)
and retained raw diffs cover all **35 regular files** in the captured generated
header tar, plus config/auto.conf/dependency/release/Makefile comparisons.
No meaningful difference was normalized away for a PASS.

| Compared item | Result / classification |
| --- | --- |
| `.config`, 8788 enabled/disabled assignments | One change: `CONFIG_CC_VERSION_TEXT`, expected cross compiler-name metadata |
| `auto.conf`, 7384 enabled assignments | Same single banner change |
| `autoconf.h`, 7384 configuration definitions | Same single banner change |
| `auto.conf.cmd` | Six tool/path/banner selectors differ: CC, LD, srctree, CC_VERSION_TEXT, NM, OBJCOPY. Kconfig dependency list unchanged. Understood non-layout dependency metadata |
| 32 of 35 captured generated headers | Byte-exact, including bounds.h, asm-offsets.h, timeconst.h, utsrelease.h, package.h, UAPI version and remaining x86 wrappers/UAPI headers |
| `arch/x86/include/generated/asm/syscalls_32.h` | **HARD STOP:** 11385-byte/429-line exact prefix of 22421-byte/828-line target; stops at syscall 239 instead of 440 |
| `include/generated/autoconf.h` | The documented banner difference, counted above |
| `include/generated/compile.h` | Absent: full-kernel build identity generated for init/version.o by init/Makefile, not modules_prepare. Understood phase-specific metadata; original identity is not copied or spoofed |
| Three additional `.unistd_{32,64,x32}.h.cmd` files | Kbuild dependency/command records not present in captured header tar; not new layout definitions |
| Source Makefile | Byte-exact target Makefile |
| Kernel release/localversion | Exact `5.10.240-antix.1-486-smp`; no duplicated suffix/dirty marker |

The configuration preserves X86_32, M486, SMP, MODVERSIONS, numeric GCC
140200, assembler 24400, linker 244000000 and all other feature/mitigation/
layout selections. Prepared `.config` SHA-256 is
`a9f866159ee08437f22c569b0f6864dd2d9cf8f4b777dd90d0397eba339f5ca4`.
All configuration differences remain visible in the evidence.

### Narrow diagnosis of the hard stop

**CONFIRMED FROM SOURCE/OUTPUT:** `syscalltbl.sh` writes directly through
`> "$out"`. Its x86 Makefile uses the file dependency/`if_changed` generation
rule. Resumed make did not invoke `syscalltbl.sh`; it accepted the existing
partial file. The failed header hash is
`24405cc6e6a1eee9eb958e88fad2ef63e16029d9066a8b863ba35aa2b63139e5`.

The same retained generator and syscall table, run with the recorded
`CONFIG_SHELL=/bin/sh` into a **separate diagnostic file**, reproduce the
target header exactly: SHA-256
`c4fcafae0822cf103d2822c0d842c6f8bb8f24b00bbb0633a3150ef36033d9fd`.
Thus the qualified source/generator can produce the expected bytes.

**INFERRED:** interruption during non-atomic generation left a stale partial
output subsequently skipped by Kbuild. **UNKNOWN:** the exact interruption
signal/cause. A zero resumed make exit does not establish complete generation.
The evidence comparison detected this failure before candidate compilation.

No repair/regeneration was applied to the failed output after the hard stop.
No captured table was copied into it: build-plan step 6 requires a passed
equivalence gate first. Source/output `Module.symvers` remain absent.

## Fresh verification and repository preservation

- Scoped Python discovery: **193 tests PASS**, 19.678 seconds, exit 0.
- Three strict GCC/UBSan C harnesses (fixed service, fixed IO, kernel contract):
  all builds/runs exit 0, no sanitizer diagnostics.
- Initial-image generator `--check`: exit 0.
- Two dry runs: both exit 0, SHA-256
  `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
- Original module against immutable target table: **222/222 PASS**, no missing
  imports or CRC mismatches; module_layout `0xb84efb99`.
- `--complete`: expected exit 1, `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.
- Initial working-tree inventory preserved; final input/working-tree hashes,
  documentation links, whitespace checks and `git diff --check` recorded in
  the evidence's final verification record.

Only Phase 8 checkpoint documentation and new preparation evidence change.
No implementation, test predicate, historical evidence or existing unrelated
file changes. The old rejected candidate remains unchanged. No replacement
module hash/Build ID/vermagic/import verification exists because it was not
built. Hot-transition/recovery remains independently blocked.

**SINGLE SMALLEST NEXT STEP — OFFLINE:** regenerate the interrupted syscall
header through normal Kbuild in an isolated successor output, preserving this
failed output; repeat the complete config/header/layout comparison. Do not
compile a candidate until that gate passes within a separately scoped build.
