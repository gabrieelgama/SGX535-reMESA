# Successor antiX preparation/equivalence — 2026-09-30

**KERNEL PREPARATION: PASS. CONFIG/HEADER/LAYOUT EQUIVALENCE: PASS.**
**NEW CANDIDATE: NOT BUILT. TARGET CONTACT: NONE.**
**GATE B: BLOCKED. WHITELIST: `[]`.**

This executes only fresh-output preparation and complete comparison under the
[offline build plan](i386-gcc-offline-build-plan.md). No SGX implementation,
test predicate, CRC, target state or historical evidence changed. Candidate
compilation is a separately scoped next task, even after this gate passes.
The [previous failed preparation](antix-kbuild-preparation-equivalence.md)
remains failed evidence; it was not repaired or resumed in this task.

## Provenance and preservation

[New evidence index](artifacts/antix-kbuild-successor-20260930/README.md),
[inputs](artifacts/antix-kbuild-successor-20260930/inputs.json) and
[paths](artifacts/antix-kbuild-successor-20260930/paths.json) record exact local
identities. Initial `git status --short` and 1360 pre-existing file hashes were
captured before preparation.

| Authoritative input | SHA-256 |
| --- | --- |
| `linux-5.10.240-antix.1-486-smp_5.10.240-antix.1-486-smp.orig.tar.gz`, retained under `/tmp/sgx535-antix-source-6/` | `e5c5d7c6bdcc7a845589e5956cbf50a8d6198232df017f0398a2902371042e1d` |
| Exact retained source-package `-6.diff.gz` | `5c41ae456d77d9e0af181fe3153c4f80e89bacdd2b541ba15fbac0b297d69b03` |
| Capture-08 `build__config` | `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9` |
| Capture-08 qualified `build_Module_symvers` | `faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca` |
| Capture-08 generated-header archive | `f899bce235cc04a3aa3c74f569fe2ba793a0b606676144ff762a57be2aa1f45c` |
| Source/captured Makefile | `70b5c7688f9067bd13b8413a205dc73ba9f66676b3de34c4092d7ceb9c260493` |

The unchanged exact source reconstructed during the preceding pass was reused:

```text
SOURCE:
/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp

FAILED OUTPUT — immutable, never supplied to Kbuild this task:
/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/output

SUCCESSOR WORK:
/home/gama/sgx535-offline/antix-kbuild-successor-20260930

SUCCESSOR OUTPUT:
/home/gama/sgx535-offline/antix-kbuild-successor-20260930/output
```

Archive/diff/config/table hashes, recorded source-rule hashes and 20 qualified
executable hashes matched before use. The source is clean for an O= build;
its incompatible public table is absent. No cleanup or changes were made to
the failed output, prior logs or prior evidence. Before/after snapshots match
for **all 10569 failed-output entries**, including file bytes, symlink targets,
modes, inodes and modification times. Both snapshot SHA-256 values are
`1820bda1253d0d52365b887177efb8b1caa19a033840886bfd5307e40b8ae8d3`.

## Actual Kbuild execution

The already-qualified rootless toolchain is unchanged at
`/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root` (`T` below).
[Actual tool identities](artifacts/antix-kbuild-successor-20260930/actual-tool-identities.json)
and [environment](artifacts/antix-kbuild-successor-20260930/build-environment.json)
record observed version output and routing:

- Target `T/usr/bin/i686-linux-gnu-gcc-14 --sysroot=T`: GCC 14.2.0,
  `i686-linux-gnu`, banner Debian 14.2.0-19; package 14.2.0-19cross1.
- Target `T/usr/bin/i686-linux-gnu-as` and `...-ld.bfd`: GNU binutils 2.44.
- Native HOSTCC `/usr/bin/gcc-14`: GCC 14.2.0, `aarch64-linux-gnu`.
- `ARCH=x86`, absolute `CROSS_COMPILE=T/usr/bin/i686-linux-gnu-`,
  `LOCALVERSION=-486-smp`, `V=1`, recorded qualified native dependency flags,
  PATH/runtime-library/bison/m4 settings. Inherited build overrides cleared.

Fresh output initially contained **only** the byte-exact captured `.config`.
Normal commands, with full absolute arguments preserved in JSON:

```text
make -C SOURCE O=SUCCESSOR_OUTPUT <recorded qualified settings> olddefconfig
make -C SOURCE O=SUCCESSOR_OUTPUT <same settings> modules_prepare
```

`olddefconfig`: exit 0, 29.599 seconds, empty stderr.
`modules_prepare`: exit 0, 68.515 seconds, empty stderr; no interruption/retry.
Neither target nor diagnostic headers were copied into the successor.
Kbuild rebuilt its native helpers; no helper came from the failed output.

Actual helper ELF checks: fixdep, conf and modpost are ELF64 little-endian
AArch64; preparation `scripts/mod/empty.o` is ELF32 little-endian Intel 80386.
Verbose target commands use the qualified cross GCC with `-m32 -march=i486`.
No Clang substitution, ARM multilib or ARM compiler receiving `-m32` occurred.

## Previously interrupted header — CONFIRMED

Successor artifact:
`output/arch/x86/include/generated/asm/syscalls_32.h`, under successor work.

- Absent before Kbuild; generated once by the expected `syscalltbl.sh` rule
  from the exact source `syscall_32.tbl`, as recorded in verbose stdout and
  successor `.syscalls_32.h.cmd`.
- **22421 bytes, 828 lines**, includes syscall 440; byte-exact target header.
- SHA-256: `c4fcafae0822cf103d2822c0d842c6f8bb8f24b00bbb0633a3150ef36033d9fd`.
- Recorded successor inode 1672025/device 66325, mode 0664, mtime_ns
  1790807288023415987; distinct from the failed artifact's inode.
- The failed 11385-byte prefix/hash `24405cc6…` remains unchanged.

This closes the interrupted-header preparation blocker. It does not determine
the precise interruption cause from the previous pass, which remains UNKNOWN.

## Complete equivalence comparison and every non-exact difference

[Complete independent comparison](artifacts/antix-kbuild-successor-20260930/complete-equivalence.json)
starts again from the immutable capture-08 archive and configuration. It does
not reuse the preceding pass's comparison results as proof. Raw file hashes,
sizes, diffs, semantic values and classification logic are preserved.

| Item | Result / classification |
| --- | --- |
| 33 of 35 captured generated headers | **EXACT MATCH**, including syscall header, bounds.h, asm-offsets.h, timeconst.h, package.h, utsrelease.h, UAPI version and remaining x86 wrappers/UAPI headers |
| Full `.config`, 8788 enabled/disabled values | All equal except CONFIG_CC_VERSION_TEXT: **EXPECTED/DOCUMENTED CROSS-BUILD METADATA DIFFERENCE** |
| `auto.conf` and `autoconf.h` | Same single compiler-name banner difference: **EXPECTED/DOCUMENTED CROSS-BUILD METADATA DIFFERENCE** |
| `auto.conf.cmd` | Six selectors change: CC, LD, srctree, CC_VERSION_TEXT, NM, OBJCOPY. Dependency list/rest of content exact: **UNDERSTOOD NON-ABI/NON-LAYOUT DIFFERENCE** |
| `include/generated/compile.h` | Not produced by modules_prepare; init/Makefile generates full-kernel build identity for init/version.o: **UNDERSTOOD NON-ABI/NON-LAYOUT DIFFERENCE**. Not copied/spoofed |
| Extra `.syscalls_32.h.cmd`, `.unistd_{32,64,x32}.h.cmd` | Four normal Kbuild command/dependency records: **UNDERSTOOD NON-ABI/NON-LAYOUT DIFFERENCE** |
| Source Makefile; kernel.release/utsrelease.h | **EXACT MATCH**; release `5.10.240-antix.1-486-smp`, no suffix drift |
| Unexplained architecture/config/feature/mitigation/layout/header drift | **NONE** |

Comparison-only transformations are restricted to the literal reviewed banner
and six individually validated dependency selectors. Raw diffs remain visible;
no configuration, source or generated bytes were edited or normalized in place.
Filesystem owner/inode/timestamps are generation provenance, not content/layout
equivalence criteria. The newly generated module linker-script/command and its
exact source rule are retained; no captured target linker-script byte identity
is claimed where that reference was not collected.

Numeric GCC/AS/LD versions, X86_32, M486, SMP, MODVERSIONS and every other
configuration selection remain equal. Relevant generated bounds/offset constants
are byte-exact, rather than checked only for selected favorable fields.
Prepared config SHA-256:
`a9f866159ee08437f22c569b0f6864dd2d9cf8f4b777dd90d0397eba339f5ca4`.

**EQUIVALENCE: PASS** for the documented preparation/config/header/layout gate.
This is not proof of every possible structure layout, module ABI compatibility,
runtime behavior, hot-reload safety or target readiness. The future candidate
still requires all import/CRC/config/source/artifact checks.

## Fresh verification and task boundary

- Scoped Python discovery: **193 PASS**, 20.653 seconds, exit 0.
- Three strict native C/UBSan harnesses: all builds/runs exit 0, no diagnostics.
- Initial-image generator `--check`: PASS.
- Original module against captured target table: **222/222 PASS**, no missing
  imports or CRC mismatches; module_layout `0xb84efb99`.
- Two dry runs: identical SHA-256
  `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
- `--complete`: expected exit 1,
  `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`; no predicate weakened.
- Final immutable-input/prior-evidence checks, repository delta, documentation
  links/whitespace and `git diff --check` recorded in final-verification.json.

Only current Phase 8 documentation and new successor evidence are added/updated.
No implementation/test changes, candidate compilation, table CRC modification,
target contact, staging, commit or push. The qualified target table remains
unchanged in capture 08; it was **not yet installed** into successor output.
Build-plan step 6 and candidate compilation/qualification remain unexecuted.
The rejected candidate and failed preparation remain immutable evidence.

**SINGLE SMALLEST NEXT STEP — OFFLINE:** separately scope the first external
candidate build in this equivalence-qualified successor environment, installing
the unchanged captured target table as the normal Kbuild input and qualifying
every imported symbol/CRC and all source/config/artifact invariants afterward.
No deployment is authorized; hot-transition/IRQ recovery remains independently
blocked and outside this preparation task.
