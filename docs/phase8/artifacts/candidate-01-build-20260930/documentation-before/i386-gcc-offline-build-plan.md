# Offline i386 GCC compatibility research and build plan

Date: 2026-09-30. **Research/documentation only; commands below are proposed,
not executed.** No Mini 12 contact, package installation, candidate rebuild,
deployment, module operation or SGX action occurred in this research turn.
Gate B remains **BLOCKED**, whitelist `[]`.

**Subsequent implementation update:** The [local toolchain qualification](i386-gcc-toolchain-qualification.md) now records successful rootless provisioning and cross/native smoke checks. The historical research statements below describe the earlier unprovisioned checkpoint. Kernel preparation and candidate rebuild remain unexecuted; the next OFFLINE step is isolated preparation/config/header comparison. Gate B remains BLOCKED.

## Decision and evidence classes

**A documented build route is established; an installed/verified toolchain and
a qualified replacement candidate are not yet established.** Debian publishes
an ARM64-hosted GCC 14.2 cross compiler for i686 and matching-version GNU
binutils. This removes uncertainty about whether such a host/target toolchain
exists. It does not prove that the future build preserves this kernel's ABI.
See [Debian GCC package](https://packages.debian.org/trixie/gcc-14-i686-linux-gnu)
and [binutils package](https://packages.debian.org/trixie/binutils-i686-linux-gnu).

All conclusions use these classes:

- **PROVEN BY REPOSITORY EVIDENCE:** demonstrated by retained bytes, source,
  tests or recorded target observations; observations remain dated snapshots.
- **PROVEN BY PUBLIC DOCUMENTATION:** the cited primary source establishes
  the tool/build behavior or published package availability.
- **REASONABLE INFERENCE:** a proposed application of those facts to this
  project; requires the stated future checks.
- **UNKNOWN:** not established; do not substitute a plausible value.

## Current inputs: PROVEN BY REPOSITORY EVIDENCE

[Capture 08](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/RESULT.md)
independently verified the original driver/display and exact running release
`5.10.240-antix.1-486-smp`, i686. Installed image and headers have identical
package version `5.10.240-antix.1-486-smp-6`. The headers' export table matches
all 222 original-module imports and supplies every one of the rejected
candidate's 228 imports, including nine candidate-only names.

Use its `artifacts/` directory as immutable evidence:

| Input | SHA-256 |
| --- | --- |
| `build_Module_symvers` | `faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca` |
| `boot-config` and `build__config` (identical) | `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9` |
| `build_include_generated_autoconf_h` | `df4c243870d720c7fbb9481f0572cfd1fced552a24652d7a90474a3eb420340b` |
| `build_generated.tar` | `f899bce235cc04a3aa3c74f569fe2ba793a0b606676144ff762a57be2aa1f45c` |

The [manifest](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifact-manifest.json)
contains the remaining hashes. Captured `.config`, `auto.conf` and
`autoconf.h` agree on all 7,384 enabled assignments, with two equivalent
hex-zero spellings. Relevant settings: `CONFIG_X86_32=y`, `CONFIG_M486=y`,
`CONFIG_SMP=y`, `CONFIG_MODVERSIONS=y`, `CONFIG_DRM_GMA500=m`, GCC version
140200, assembler version 24400, linker version 244000000. Configured local
version is empty; the running release nevertheless ends in `-486-smp`.
The captured Makefile has `EXTRAVERSION=-antix.1` and matches retained source.

The source package is retained through
`docs/phase4-2-data/linux-5.10.240-antix.1-486-smp_5.10.240-antix.1-486-smp-6.dsc`
and its diff. The large original archive and extracted old build are currently
under `/tmp/sgx535-antix-source-6/`; temporary paths are not durable evidence.
Before using the archive, verify its dsc SHA-256
`e5c5d7c6bdcc7a845589e5956cbf50a8d6198232df017f0398a2902371042e1d`
and the diff.gz SHA-256
`5c41ae456d77d9e0af181fe3153c4f80e89bacdd2b541ba15fbac0b297d69b03`.
No source-package redownload is needed while these verified bytes remain.

The capture includes configuration and generated x86/header trees, **not a
complete ready-to-run header package**: all `include/config` stubs, build
scripts and host executables were not exported. Prepare these locally from
the retained source with the selected toolchain; compare against the capture.
Do not assume i386 host tools from an installed header package run on ARM64.

## Cross-build conclusions

| Question | Classification and conclusion | Primary support |
| --- | --- | --- |
| Host versus target architecture | **PROVEN BY PUBLIC DOCUMENTATION:** Kbuild's target `ARCH` and native `HOSTCC` are separate. `CROSS_COMPILE` selects target tools. | [5.10 Kbuild](https://www.kernel.org/doc/html/v5.10/kbuild/kbuild.html), [5.10 Makefiles](https://www.kernel.org/doc/html/v5.10/kbuild/makefiles.html) |
| `ARCH=x86` versus 32-bit selection | **PROVEN BY PUBLIC DOCUMENTATION:** x86 is shared; `CONFIG_X86_32` selects its 32-bit branch and `-m32`. `ARCH` alone is not the bitness proof. **PROVEN BY REPOSITORY EVIDENCE:** this target config selects that branch. | [5.10.240 x86 Makefile](https://raw.githubusercontent.com/gregkh/linux/v5.10.240/arch/x86/Makefile) |
| i686 compiler versus 486 kernel | **PROVEN BY PUBLIC DOCUMENTATION:** `CONFIG_M486` contributes `-march=i486`. **REASONABLE INFERENCE:** the i686-target cross compiler can build this configured kernel; confirm actual flags, ELF and required features. Do not change the target to M686 or use `-march=native`. | [5.10.240 CPU flags](https://raw.githubusercontent.com/gregkh/linux/v5.10.240/arch/x86/Makefile_32.cpu) |
| Multilib versus cross compiler | **PROVEN BY PUBLIC DOCUMENTATION:** multilib supplies variants within a compiler target; x86 `-m32` specifies x86 code. **REASONABLE INFERENCE:** an AArch64-native GCC does not become x86 GCC by installing ARM multilib or adding `-m32`; use an x86-targeting compiler. | [GCC configure](https://gcc.gnu.org/install/configure.html), [GCC 14.2 x86 options](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/x86-Options.html) |
| Target CC and binutils | **PROVEN BY PUBLIC DOCUMENTATION:** GNU Kbuild derives gcc/ld/ar/nm/objcopy/etc. from the prefix, with explicit command-line overrides possible. Native HOSTCC remains distinct. | [5.10.240 top Makefile](https://raw.githubusercontent.com/gregkh/linux/v5.10.240/Makefile) |
| Linker qualification | **PROVEN BY PUBLIC DOCUMENTATION:** `ld -V`/`--verbose` lists supported emulations. **REASONABLE INFERENCE:** require GNU x86 ELF32 support and verify relocatable ELF32 output, not only the linker version string. | [GNU ld options](https://sourceware.org/binutils/docs/ld/Options.html), [Debian ARM64 binutils file list](https://packages.debian.org/trixie/arm64/binutils-i686-linux-gnu/filelist) |
| MODVERSIONS | **PROVEN BY PUBLIC DOCUMENTATION:** exported-symbol prototype CRCs form an ABI consistency check; modpost reads the export table. `modules_prepare` does not create that table. | [5.10 external modules](https://www.kernel.org/doc/html/v5.10/kbuild/modules.html) |
| Which table is consumed | **PROVEN BY REPOSITORY EVIDENCE**, corroborated publicly: selected in-tree modpost reads `vmlinux.symvers`; external `M=...` mode reads the prepared kernel's `Module.symvers`. Use the latter with the captured complete table. | [5.10.240 modpost Makefile](https://raw.githubusercontent.com/gregkh/linux/v5.10.240/scripts/Makefile.modpost) |
| Compiler/config drift | **PROVEN BY PUBLIC DOCUMENTATION:** compiler identity changes can trigger Kconfig regeneration; compiler family, versions and capability probes are configuration inputs. A cross compiler banner may differ from native GCC's. | [5.10.240 init/Kconfig](https://raw.githubusercontent.com/gregkh/linux/v5.10.240/init/Kconfig) |
| Target libc | **PROVEN BY REPOSITORY EVIDENCE:** retained `cc-can-link.sh` compiles/links a stdio program. **REASONABLE INFERENCE:** include cross libc development/startup files for those configuration probes. This does not mean the kernel module links against libc. | [public probe implementation](https://raw.githubusercontent.com/gregkh/linux/v5.10.240/scripts/cc-can-link.sh), [Debian cross libc](https://packages.debian.org/trixie/libc6-dev-i386-cross) |
| Reproducibility | **PROVEN BY PUBLIC DOCUMENTATION:** timestamps, user/host, paths, signing and possible layout-randomization inputs affect reproducibility. **UNKNOWN:** bit-identical output of the proposed build until it is performed twice with recorded inputs. | [5.10 reproducible builds](https://www.kernel.org/doc/html/v5.10/kbuild/reproducible-builds.html) |
| Module signatures | **PROVEN BY PUBLIC DOCUMENTATION:** signature policy is independent of CRC compatibility. **PROVEN BY REPOSITORY EVIDENCE:** prior original module carried E taint. Future policy must not be bypassed or inferred solely from that historical observation. | [5.10 module signing](https://www.kernel.org/doc/html/v5.10/admin-guide/module-signing.html) |

**UNKNOWN:** why the old public table differs. It has 25,195 entries versus
23,920 in the target table; 17,520 shared export CRCs differ. Public and
installed recorded `.config` bytes are identical. Changing a compiler banner
or selecting GCC now does not explain the provenance of that old table.

## Smallest proposed toolchain route

**PROVEN BY PUBLIC DOCUMENTATION:** Debian trixie publishes these ARM64-host
packages; these are x86-targeting tools, not i386 executables to emulate:

| Package | Published version | Role |
| --- | --- | --- |
| `gcc-14-i686-linux-gnu:arm64` | `14.2.0-19cross1` | Target C compiler |
| `cpp-14-i686-linux-gnu:arm64` | `14.2.0-19cross1` | Target preprocessor; dependency |
| `binutils-i686-linux-gnu:arm64` | `2.44-3` | Target assembler, BFD linker and object tools |
| `libc6-dev-i386-cross` | `2.41-11cross1` | Cross libc/link-test inputs, with its dependencies |

Sources: [GCC metadata](https://packages.debian.org/trixie/gcc-14-i686-linux-gnu),
[CPP metadata](https://packages.debian.org/trixie/cpp-14-i686-linux-gnu),
[binutils metadata](https://packages.debian.org/trixie/binutils-i686-linux-gnu),
[cross libc metadata](https://packages.debian.org/trixie/libc6-dev-i386-cross).
Resolve the complete dependency closure, including GCC cross support libraries
and ARM64 runtime libraries. Retained libgcc cross .debs alone are not a
compiler. Use native ARM64 make/GCC/bison/flex/bc and necessary host development
libraries for Kbuild helpers; the dsc records native libssl requirements.

Public download-page hashes, **not locally verified downloaded artifacts**:

- [GCC ARM64 .deb](https://packages.debian.org/trixie/arm64/gcc-14-i686-linux-gnu/download):
  `c7a3bb2fc11ea6b656749cb31b4560d2265ba35b687665b1af1c8a7e33c1b72e`.
- [Binutils ARM64 .deb](https://packages.debian.org/trixie/arm64/binutils-i686-linux-gnu/download):
  `9c0062347df833364717ed97615bccb5eda09e62e2cffc97ef711337cf728d93`.

**REASONABLE INFERENCE / preferred future implementation:** provision these
pinned packages and dependencies inside an isolated local Debian ARM64 build
environment, using authenticated Debian package metadata. Record all package
versions, hashes, executable identities, runtime dependencies and environment
identity. No Mini 12 access or device passthrough is needed. Do not silently
update to GCC 15 or use a compiler for another target. A relocatable extraction
instead of a package-managed environment is possible in principle, but its
program/library/specs paths need testing; it is not the preferred first route.
No package was downloaded or installed in this turn.

Qualification before kernel preparation, on that local host only:

```sh
# Resolve these paths from the installed package's file list first.
# The expected versioned name is a proposed path, not a verified local tool.
i686-linux-gnu-gcc-14 -dumpmachine
i686-linux-gnu-gcc-14 -dumpfullversion
i686-linux-gnu-gcc-14 -v
i686-linux-gnu-ld.bfd --version
i686-linux-gnu-ld.bfd -V
```

Require target `i686-linux-gnu`, GCC 14.2.0, GNU as/ld 2.44, native ARM64 host
executables, and no missing runtime dependency. Compile a local freestanding
32-bit object with `-m32 -march=i486`; inspect it with target readelf; perform
an ELF32 relocatable link; compile a native helper with HOSTCC and run only that
native helper. Do not execute generated x86 objects/binaries to test the GPU.
The Debian cross build is not the identical compiler binary used on the target;
its suitability is a **REASONABLE INFERENCE** until these checks/config
comparisons and the candidate verification below pass.

## Proposed isolated preparation and build

These steps are a future **OFFLINE** implementation plan. Each failure stops
that build; none permits deployment or changing Gate B.

1. Preserve the old candidate, old build table, Attempts 01–03, recovery and
   capture 08 unchanged. Hash source, patches, client/UAPI and target inputs.
   Create a new local workspace outside this repository's Git ancestry. Never
   run cleanup on the existing working tree or `/tmp/.../source` evidence.
2. Reconstruct the exact source package from verified original archive plus
   retained diff. The public table remains rejected evidence, never a build
   input. In the **new disposable source copy only**, clean package-generated
   build products if necessary, then use a fresh separate output directory.
3. Copy captured `build__config` into the new output `.config`. Keep captured
   headers as comparison references. Native Kbuild tools must be rebuilt for
   ARM64; do not retain stale i386 host executables or old `.cmd` files.
4. Prepare with consistent settings on every invocation. Example variables
   refer to local future paths; this block was not run:

```sh
# task_src: fresh exact antiX source; task_out: fresh local output directory.
# task_evidence: repository capture-08/artifacts; task_repo: repository root.
# task_cc/task_hostcc/task_cross: verified absolute executable paths/prefix.
cp "$task_evidence/build__config" "$task_out/.config"
make -C "$task_src" O="$task_out" ARCH=x86 \
  CROSS_COMPILE="$task_cross" CC="$task_cc" LD="${task_cross}ld.bfd" \
  HOSTCC="$task_hostcc" \
  LOCALVERSION=-486-smp olddefconfig
make -C "$task_src" O="$task_out" ARCH=x86 \
  CROSS_COMPILE="$task_cross" CC="$task_cc" LD="${task_cross}ld.bfd" \
  HOSTCC="$task_hostcc" \
  LOCALVERSION=-486-smp modules_prepare
```

The intended target prefix is `i686-linux-gnu-`; target CC is the verified
versioned GCC 14 binary and LD must be GNU BFD, not lld. No `LLVM=1` or inherited
Clang/LLVM tool overrides. Native HOSTCC may be the verified AArch64 GCC 14.
The `LOCALVERSION` command argument supplies the already observed suffix
without changing captured `CONFIG_LOCALVERSION`. Retained setlocalversion
behavior supports this derivation; **verify** the resulting release and stop
on a duplicated suffix, `+`, `-dirty`, or any other unexpected result.
Do not hand-edit release/generated headers to disguise a mismatch.

5. Compare full pre/post config, `auto.conf`, `autoconf.h`, generated x86
   headers and layout-related outputs against capture 08. The initial proposed
   allowance is only the actual `CONFIG_CC_VERSION_TEXT` banner difference,
   recorded explicitly because the cross executable/package name may differ.
   Numeric GCC/as/ld versions and all architecture, mitigation, layout,
   FORTIFY, SMP, module-version, CPU and driver selections must remain equal.
   Any other changed capability/config setting is **STOP for review**, not an
   instruction to disable that setting. Do not spoof compiler detection,
   timestamps, old builder identity or generated layout outputs.
6. Only after preparation passes, copy the full captured target table into
   `$task_out/Module.symvers` as an ordinary Kbuild input. Verify its hash and
   run the existing reference-table checker. `modules_prepare` is not asked
   to regenerate CRCs. An independently rebuilt whole kernel is unnecessary
   merely to obtain already-qualified CRCs, but the prepared headers/layout
   must still be correct. Never patch CRC entries in a `.ko` or `.mod.c`.
7. Copy the exact antiX gma500 source directory into a new external-module
   directory. Add the current fixed sources/UAPI and generated includes from
   `kernel/sgx535_frozen/` and `tools/psb-dri-re/`; use the three retained
   Makefile/IRQ/ioctl patches. Record the full resulting source manifest.
   The existing Makefile's `obj-$(CONFIG_DRM_GMA500)` resolves to obj-m because
   captured CONFIG_DRM_GMA500=m; preserve its complete object list.
8. Build only that external module against the prepared local base:

```sh
make -C "$task_src" O="$task_out" ARCH=x86 \
  CROSS_COMPILE="$task_cross" CC="$task_cc" LD="${task_cross}ld.bfd" \
  HOSTCC="$task_hostcc" \
  LOCALVERSION=-486-smp M="$task_mod" V=1 modules
```

**REASONABLE INFERENCE:** this is the smallest normal Kbuild route for the
existing integration; it has not been executed. Inspect the actual V=1 commands
and dependency files to confirm `-m32`, `-march=i486`, native host helpers,
GNU target assembler/linker, expected headers, and modpost `-e` consuming the
captured output-tree `Module.symvers`. Stop on undefined symbols, missing
version tables, CRC warnings, namespace errors, configuration drift or
unreviewed source changes. Do not use `KBUILD_MODPOST_WARN`, ignore errors,
remove versions, force-load, or import the old public `vmlinux.symvers`.
Do not run `modules_install`, `depmod`, insmod or the one-shot client.

## Required candidate checks

**PROVEN BY REPOSITORY EVIDENCE:** the checker parses ELF32 `__versions` and
can compare every import against a supplied table. Its two-module comparison
alone cannot qualify candidate-only symbols. These are proposed future calls:

```sh
python3 "$task_repo/tools/psb-dri-re/frozen_module_versions.py" \
  --check-symvers "$task_original_ko" "$task_evidence/build_Module_symvers"
python3 "$task_repo/tools/psb-dri-re/frozen_module_versions.py" \
  --check-symvers "$task_new_ko" "$task_evidence/build_Module_symvers"
```

In the second call the new artifact is the first/reference argument: the
checker checks **all imports in that artifact**, not only imports shared with
the original. Its JSON scope remains `REFERENCE_IMPORTS_ONLY` and
`candidate_imports_verified=false` by API design; record the first-argument
SHA/count and the independent table provenance explicitly. Do not relabel
those fields or infer qualification of an unexamined artifact.

Require zero missing imports, zero CRC mismatches, module_layout b84efb99,
ELF32 little-endian Intel 80386, exact vermagic, source/UAPI identity and
successful complete linking. Compare dependency lists and all newly added
imports; do not assume the new count remains 228. Preserve compiler commands,
headers/config diffs, `mod.c`, `__versions`, symbols, relocations, metadata,
build ID and SHA-256. Confirm table bytes unchanged before/after modpost.
A matching CRC table is a necessary ABI check, **not proof** of correct code,
all structure layouts, hardware readiness or safe hot replacement.

Run the existing scoped Python suite, strict C/UBSan harnesses, generator
`--check`, two frozen dry runs and `--complete`. Preserve fail-closed output.
For reproducibility, pin genuine build inputs, paths/environment and chosen
build metadata, then repeat the isolated build and compare bytes. Do not claim
bit identity with the old/original modules; source/toolchain differ. Record
signing status; do not disable target enforcement or fabricate a signature.

**Independent remaining blocker:** the original hot unload/reload lifecycle
emitted cleanup warnings and subsequently oopsed in IRQ setup. The retained
unload path lacks `drm_irq_uninstall`; stale IRQ state is an inference, not
proven causation. A successful ABI rebuild does not repair or qualify this
transition. No deployment/Attempt 04 is authorized.

## Public source register

Every row was accessed **2026-09-30** through the web tool. All are primary
project/package sources; no forum or secondary tutorial supports a conclusion.
URLs above and below were opened successfully unless explicitly noted.

| ID | Document/page title | Organization/project | URL | Access date | Exact supported claim |
| --- | --- | --- | --- | --- | --- |
| S1 | Building External Modules (5.10) | Linux kernel | https://www.kernel.org/doc/html/v5.10/kbuild/modules.html | 2026-09-30 | Prepared config/headers, M=, MODVERSIONS and Module.symvers; modules_prepare does not produce CRCs. |
| S2 | Kbuild (5.10) | Linux kernel | https://www.kernel.org/doc/html/v5.10/kbuild/kbuild.html | 2026-09-30 | ARCH, CROSS_COMPILE, O=/M= and modpost-warning controls. |
| S3 | Linux Kernel Makefiles (5.10) | Linux kernel | https://www.kernel.org/doc/html/v5.10/kbuild/makefiles.html | 2026-09-30 | Native HOSTCC and host-program build flags. |
| S4 | Makefile, v5.10.240 | Linux stable / Greg Kroah-Hartman maintainer mirror | https://raw.githubusercontent.com/gregkh/linux/v5.10.240/Makefile | 2026-09-30 | GNU target tool prefix, HOSTCC separation, external-module base/config behavior. |
| S5 | arch/x86/Makefile, v5.10.240 | Linux stable | https://raw.githubusercontent.com/gregkh/linux/v5.10.240/arch/x86/Makefile | 2026-09-30 | CONFIG_X86_32, UTS_MACHINE=i386, target -m32 and kernel x86 flags. |
| S6 | arch/x86/Makefile_32.cpu, v5.10.240 | Linux stable | https://raw.githubusercontent.com/gregkh/linux/v5.10.240/arch/x86/Makefile_32.cpu | 2026-09-30 | CONFIG_M486 selects -march=i486. |
| S7 | scripts/Makefile.modpost, v5.10.240 | Linux stable | https://raw.githubusercontent.com/gregkh/linux/v5.10.240/scripts/Makefile.modpost | 2026-09-30 | In-tree/external symdump paths and generation of module version metadata. |
| S8 | init/Kconfig, v5.10.240 | Linux stable | https://raw.githubusercontent.com/gregkh/linux/v5.10.240/init/Kconfig | 2026-09-30 | Compiler banner triggers regeneration; GCC/Clang/binutils version and capability inputs. |
| S9 | x86 Options, GCC 14.2 | GNU GCC | https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/x86-Options.html | 2026-09-30 | x86 -m32 data model and architecture options; not ARM ILP32. |
| S10 | Installing GCC: Configuration | GNU GCC | https://gcc.gnu.org/install/configure.html | 2026-09-30 | Build/host/target distinction, multilib variants and distributor version strings. |
| S11 | Options (LD) | GNU binutils / Sourceware | https://sourceware.org/binutils/docs/ld/Options.html | 2026-09-30 | Supported linker emulations can be listed with -V/--verbose. |
| S12 | gcc-14-i686-linux-gnu in trixie | Debian GCC maintainers | https://packages.debian.org/trixie/gcc-14-i686-linux-gnu | 2026-09-30 | ARM64 host package 14.2.0-19cross1 targeting i686 and dependency closure. |
| S13 | binutils-i686-linux-gnu in trixie | Debian | https://packages.debian.org/trixie/binutils-i686-linux-gnu | 2026-09-30 | ARM64-host GNU i686 assembler/linker package 2.44-3. |
| S14 | ARM64 binutils package file list | Debian | https://packages.debian.org/trixie/arm64/binutils-i686-linux-gnu/filelist | 2026-09-30 | Actual i686-prefixed as/ld.bfd/readelf/etc. paths in the package. |
| S15 | cpp-14-i686-linux-gnu in trixie | Debian | https://packages.debian.org/trixie/cpp-14-i686-linux-gnu | 2026-09-30 | ARM64-hosted cross preprocessor availability at matching cross package version. |
| S16 | libc6-dev-i386-cross in trixie | Debian Cross Toolchain Base Team | https://packages.debian.org/trixie/libc6-dev-i386-cross | 2026-09-30 | Cross libc headers/startup files, architecture all and runtime/header dependencies. |
| S17 | GCC ARM64 package download metadata | Debian | https://packages.debian.org/trixie/arm64/gcc-14-i686-linux-gnu/download | 2026-09-30 | Exact compiler package filename and published SHA-256, not a local installation test. |
| S18 | binutils ARM64 package download metadata | Debian | https://packages.debian.org/trixie/arm64/binutils-i686-linux-gnu/download | 2026-09-30 | Exact binutils package filename and published SHA-256. |
| S19 | scripts/cc-can-link.sh, v5.10.240 | Linux stable | https://raw.githubusercontent.com/gregkh/linux/v5.10.240/scripts/cc-can-link.sh | 2026-09-30 | Kconfig link probe includes stdio and links a target userspace test without executing it. |
| S20 | Reproducible builds (5.10) | Linux kernel | https://www.kernel.org/doc/html/v5.10/kbuild/reproducible-builds.html | 2026-09-30 | Timestamp/path/build-identity/signing/layout-seed reproducibility factors. |
| S21 | Kernel module signing facility (5.10) | Linux kernel | https://www.kernel.org/doc/html/v5.10/admin-guide/module-signing.html | 2026-09-30 | Signature enforcement is distinct from CRCs; do not bypass or infer current runtime policy. |

Consulted but not relied upon: current/latest Kbuild search results (the
versioned 5.10 pages above were used instead), GCC build/specific-installation
pages (no additional conclusion), GNU's ld manual landing page and a Debian
GCC manpage URL (tool errors), and the ARM64 GCC package file-list URL (client
challenge). No access-control bypass was attempted. The executable name must
therefore be verified from the retrieved package before running future commands.

## Verification and smallest next step

**Subsequent local execution, 2026-09-30:** the toolchain qualification passed.
Steps 1–5 were attempted in the [isolated preparation pass](antix-kbuild-preparation-equivalence.md).
Configuration has only the allowed cross compiler-name banner change, but
generated-header equivalence FAILED: interrupted `syscalls_32.h` is truncated
and was skipped by resumed Kbuild despite exit 0. Fresh diagnostic generation
matches the target; failed prepared output remains unchanged. No candidate
was compiled, and step 6 target-table installation was not reached.

**Current successor result:** [fresh normal Kbuild preparation](antix-kbuild-successor-equivalence.md)
and the complete documented config/header/layout comparison now PASS. The
syscall header was generated by Kbuild and matches target; failed output and
evidence remain unchanged. No candidate compiled or table installed into
successor output in that task. No target contact or implementation changes.

**Newest candidate-build continuation:** [candidate #1 source/patch guard](candidate-01-offline-build-qualification.md)
STOPPED before Kbuild: the Makefile patch requests a nonexistent final blank
line; the ioctl patch also fails its strict no-fuzz dry run. IRQ dry run passes.
No patch applied or input changed, no table installed and no candidate built.
Qualified preparation/toolchain state remains unchanged.

**Current SINGLE SMALLEST NEXT STEP: OFFLINE** — separately correct/review the
retained Makefile/ioctl integration patches against the qualified exact source,
requiring strict no-fuzz dry-run success. Then resume steps 6–8 and complete
candidate qualification with the unchanged captured table. No deployment is authorized. The
research-turn next step below is historical, superseded by these results.

This documentation turn reruns the relevant existing checks; results are
recorded in the accompanying [ChatGPT handoff](CHATGPT-HANDOFF-POST-ATTEMPT03.md).
No implementation, test predicates, CRCs or historical evidence are changed.

**SINGLE SMALLEST NEXT STEP: OFFLINE** — provision and qualify the pinned
ARM64-hosted i686 GCC 14.2/binutils 2.44 dependency-complete toolchain in an
isolated local build environment. Then carry out this preparation/build plan.
No target observation or target-state change is needed for that step.
