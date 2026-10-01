# Offline i686 GCC toolchain qualification — 2026-09-30

**TOOLCHAIN: PASS for the observed provisioning and smoke obligations.**
**Kernel preparation: NOT PERFORMED. Replacement candidate: NOT BUILT.**
**Gate B BLOCKED; whitelist `[]`; no target contact or new SGX authorization.**

This is the qualification-stage record. Subsequent
[successor preparation/config/header/layout equivalence](antix-kbuild-successor-equivalence.md)
now PASSES in a separate output; no candidate has been built. The original
qualification evidence and failed preceding output remain unchanged.

This executes only the smallest toolchain step in the
[offline build plan](i386-gcc-offline-build-plan.md). That plan's preparation
and candidate rebuild are future implementation stages, not executed here.
No kernel configuration, generated reference, CRC, implementation, historical
evidence or preexisting unrelated file was changed. No stage/commit/push.

## Provisioning and provenance: CONFIRMED BY LOCAL EVIDENCE

The local host is Debian 13/trixie ARM64, with native GCC 14.2.0 already
installed. Provisioning used the plan's permitted **rootless relocatable
extraction** route because the current account is unprivileged. This is a
local isolated prefix, not a global dpkg installation or a full container:

```text
/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root
```

APT used fresh local lists/cache/status and a copied installed-package status,
with only `https://deb.debian.org/debian trixie main`, ARM64 and the Debian
archive keyring. `apt-get update` and download-only resolution exited 0.
Independent gpgv verification passed all three InRelease signatures. No
unauthenticated/trusted=yes option or package maintainer script was used.
All 25 downloaded archives matched signed-index SHA-256, size, package,
version and architecture before `dpkg-deb -x` into the prefix.

The exact [package/provenance manifest](artifacts/i386-gcc-toolchain-20260930/provisioned-packages.json)
records archive paths, hashes, source filenames and dependency fields.
[Initial installed-package inventory](artifacts/i386-gcc-toolchain-20260930/native-installed-packages.txt)
and runtime-library hashes record the native dependencies reused from the
local Debian host. Global `/var/lib/dpkg/status` remained byte-identical to
the starting snapshot. APT's three proposed OpenSSL upgrades were downloaded
and extracted locally only; no installed host package was upgraded.

| Package provisioned in local prefix | Version | Architecture |
| --- | --- | --- |
| `m4` | `1.4.19-8` | arm64 |
| `flex` | `2.6.4-8.2+b4` | arm64 |
| `openssl-provider-legacy` | `3.5.7-1~deb13u2` | arm64 |
| `libssl3t64` | `3.5.7-1~deb13u2` | arm64 |
| `bc` | `1.07.1-4+b1` | arm64 |
| `bison` | `2:3.8.2+dfsg-1+b2` | arm64 |
| `gcc-14-i686-linux-gnu-base` | `14.2.0-19cross1` | arm64 |
| `cpp-14-i686-linux-gnu` | `14.2.0-19cross1` | arm64 |
| `gcc-14-cross-base` | `14.2.0-19cross1` | all |
| `binutils-i686-linux-gnu` | `2.44-3` | arm64 |
| `libc6-i386-cross` | `2.41-11cross1` | all |
| `libgcc-s1-i386-cross` | `14.2.0-19cross1` | all |
| `libgomp1-i386-cross` | `14.2.0-19cross1` | all |
| `libitm1-i386-cross` | `14.2.0-19cross1` | all |
| `libatomic1-i386-cross` | `14.2.0-19cross1` | all |
| `libasan8-i386-cross` | `14.2.0-19cross1` | all |
| `libstdc++6-i386-cross` | `14.2.0-19cross1` | all |
| `libubsan1-i386-cross` | `14.2.0-19cross1` | all |
| `libquadmath0-i386-cross` | `14.2.0-19cross1` | all |
| `libgcc-14-dev-i386-cross` | `14.2.0-19cross1` | all |
| `gcc-14-i686-linux-gnu` | `14.2.0-19cross1` | arm64 |
| `linux-libc-dev-i386-cross` | `6.12.38-1cross1` | all |
| `libc6-dev-i386-cross` | `2.41-11cross1` | all |
| `libssl-dev` | `3.5.7-1~deb13u2` | arm64 |
| `openssl` | `3.5.7-1~deb13u2` | arm64 |

The cross `linux-libc-dev` package is for userspace compiler/link probes;
**it is not the Linux 5.10 kernel/module header input**. Future kernel builds
must use the retained exact antiX source/config/generated-reference checks.
Full signed-index files and .debs remain outside the repository in this
workspace; hashes, signed InRelease, keyring, package records, outputs and
smoke inputs are preserved in the repository
[offline evidence directory](artifacts/i386-gcc-toolchain-20260930/README.md).
The local path is part of this qualified relocation layout, not a claim that
arbitrarily moving the toolchain preserves qualification.

## Exact compiler roles and commands

Let `Q=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930` and
`T=$Q/root`. [environment.sh](artifacts/i386-gcc-toolchain-20260930/environment.sh)
records the actual environment, including PATH/native runtime-library paths,
BISON_PKGDATADIR, M4 and future explicit compiler role selections.

| Role | Qualified invocation / observed identity |
| --- | --- |
| Target CC | `$T/usr/bin/i686-linux-gnu-gcc-14 --sysroot=$T`; `-dumpmachine` = `i686-linux-gnu`, `-dumpfullversion` = `14.2.0`; banner Debian `14.2.0-19`, package version `14.2.0-19cross1` |
| Target binutils prefix | `$T/usr/bin/i686-linux-gnu-` |
| Target assembler | `$T/usr/bin/i686-linux-gnu-as`, GNU 2.44, configured target `i686-linux-gnu` |
| Target linker | `$T/usr/bin/i686-linux-gnu-ld.bfd`, GNU 2.44, `elf_i386` emulation supported |
| Native HOSTCC | `/usr/bin/gcc-14`, `aarch64-linux-gnu`, GCC 14.2.0 Debian `14.2.0-19` |
| Native build tools | Prefix bison 3.8.2, flex 2.6.4, m4 1.4.19, bc 1.07.1; installed native make 4.4.1 |

Twenty selected executable identities, including target GCC's cc1, were
verified as **ELF64 little-endian AArch64** with no missing dynamic libraries.
Their x86 target is established separately by compiler configuration/output.
[Executable hashes](artifacts/i386-gcc-toolchain-20260930/executable-identities.json)
include resolved symlink paths and ldd output; compiler search directories,
sysroot and program resolution are in
[tool identities](artifacts/i386-gcc-toolchain-20260930/tool-identities.json).
GCC resolved its cc1/as/ld/libgcc from the isolated prefix. No Clang, ARM
multilib or AArch64 `-m32` compilation was used.

**Required rootless routing:** GCC's compiled default sysroot reports `/`;
extracted cross libc linker scripts contain absolute `/usr/i686-linux-gnu`
paths. The explicit `--sysroot=$T` successfully directs them into this prefix.
Do not omit it or point it at the Mini 12. Native OpenSSL probes need both
`-I$T/usr/include` and `-I$T/usr/include/aarch64-linux-gnu`, plus
`-L$T/usr/lib/aarch64-linux-gnu` and the recorded native runtime-library path.
These are ordinary local search paths, not edits to compiler/header metadata.

## Observed smoke tests

Exact argv/stdout/stderr are retained in
[smoke-commands.json](artifacts/i386-gcc-toolchain-20260930/smoke-commands.json),
[native build-tool smoke](artifacts/i386-gcc-toolchain-20260930/native-build-tool-smoke.json)
and [native SSL smoke](artifacts/i386-gcc-toolchain-20260930/native-ssl-smoke.json).

```sh
# Executed in $Q/smoke with the recorded local tool environment.
"$T/usr/bin/i686-linux-gnu-gcc-14" --sysroot="$T" -m32 -march=i486   -ffreestanding -fno-pie -O2 -Wall -Wextra -Werror -v   -c target.c -o target.o
"$T/usr/bin/i686-linux-gnu-ld.bfd" -m elf_i386 -r   target.o -o target-linked.o
"$T/usr/bin/i686-linux-gnu-readelf" -h target.o
"$T/usr/bin/i686-linux-gnu-readelf" -h target-linked.o
"$T/usr/bin/i686-linux-gnu-gcc-14" --sysroot="$T" -m32 -march=i486   -Wl,-t libc-probe.c -o libc-probe
/bin/sh /tmp/sgx535-antix-source-6/source/scripts/cc-can-link.sh   "$T/usr/bin/i686-linux-gnu-gcc-14" --sysroot="$T" -m32 -march=i486
/usr/bin/make -f HOSTCC-smoke.mk HOSTCC=/usr/bin/gcc-14 all
```

- Target compile asserts 32-bit pointer/long. Object and relocatable link:
  **ELF32, little-endian, Intel 80386/e_machine=3**. PASS.
- Target libc link and the retained `cc-can-link.sh` pass. Target link tracing
  uses the extracted i386 crt/libgcc/libc inputs. No x86 executable was run.
- Native helper and the real retained `scripts/basic/fixdep.c`, built via a
  smoke Makefile selecting HOSTCC, are **ELF64 AArch64** and execute locally.
  fixdep generated the expected dependency command. PASS. This is not a full
  Kbuild preparation invocation or proof about every future generated helper.
- Native bison/flex generation (using extracted data/m4), HOSTCC compilation
  and generated parser/scanner execution pass; bc returns the expected 5.
- Native OpenSSL header/link/runtime probe passes and loads prefix libcrypto
  3.5.7. No target compilation receives these ARM64 include/library paths.

Two smoke-command corrections are retained rather than hidden: extra
`-Wextra -Werror` initially rejected an existing fixdep signedness comparison;
the smoke was corrected to the **actual retained KBUILD_HOSTCFLAGS**, without
changing source. The first OpenSSL probe omitted its extracted multiarch
include directory; adding the verified packaged search path passed. No
production guard or kernel configuration was weakened. A private command-list
assembly typo stopped before invoking any tool and did not alter artifacts.

## Important immutable inputs and observed hashes

- GCC archive: `c7a3bb2fc11ea6b656749cb31b4560d2265ba35b687665b1af1c8a7e33c1b72e`.
- Binutils archive: `9c0062347df833364717ed97615bccb5eda09e62e2cffc97ef711337cf728d93`.
- Signed InRelease: `0584fba32e13e0ab8285fb16c27adea1ec03a73669c18702821094fd6ca86675`.
- Target table unchanged: `faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca`.
- Target config unchanged: `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9`.
- Captured generated-header tar unchanged: `f899bce235cc04a3aa3c74f569fe2ba793a0b606676144ff762a57be2aa1f45c`.

Original-versus-target-table check still passes **222/222**, zero mismatches.
Old candidate remains unchanged/rejected with **154** target-table mismatches.
No replacement module hash, Build ID, vermagic or module_layout result exists
because no new module was built. Executable/output hashes are in the manifests;
they must not be confused with a candidate-module hash.

## Fresh repository verification and boundaries

- Scoped Python suite: **193 PASS**, 11.443 seconds, exit 0.
- Three native strict C/UBSan harnesses: fixed service, fixed IO, kernel
  contract; builds/runs exit 0, no sanitizer diagnostics.
- Initial-image generator `--check`: PASS.
- Two dry runs: identical SHA-256
  `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
- `--complete`: expected exit 1, `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.
- Signed package/hash checks and offline target-input checks: PASS.
- Documentation links/shell syntax, initial-file hash comparison and
  `git diff --check` are final verification gates recorded in the handoff.

**Not established:** prepared-kernel generated-header/config equivalence,
full module compilation/import CRC equality, bit-reproducible candidate,
safe hot transition/recovery or any runtime SGX property. Compiler banner
variance must still be reviewed during future preparation. Gate B remains
BLOCKED and no new target observation/action is authorized.

**SINGLE SMALLEST NEXT STEP — OFFLINE:** execute only the preparation stage
of the existing build plan in a fresh source/output tree using this qualified
local toolchain, captured configuration and immutable target references;
compare all generated configuration/layout inputs and stop on unexpected
changes. Candidate compilation may follow only within its authorized scope
and after those guards pass. Do not deploy.
