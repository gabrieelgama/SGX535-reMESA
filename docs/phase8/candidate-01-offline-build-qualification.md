# Candidate #1 offline build — integration guard STOP

Date: 2026-09-30. **NEW CANDIDATE #1: NOT BUILT. ABI QUALIFICATION: NOT ESTABLISHED.**
**Target contact NONE; Gate B BLOCKED; whitelist `[]`.**

This task reached the source/patch qualification guard before compilation.
No implementation, patch, generated header or CRC was changed. No Kbuild or
modpost invocation occurred. The qualified successor remains valid; this
failure does not reopen preparation, toolchain qualification or Phase 7.

Evidence: [artifact index](artifacts/candidate-01-20260930/README.md).
External work directory: `/home/gama/sgx535-offline/candidate-01-20260930`.

## Proven blocker

**CONFIRMED-FROM-ARTIFACT / observed strict dry run:** the retained Makefile
integration patch cannot apply without relaxing the context guard or changing
an input. Its hunk requests three old lines starting at line 55:

```text
<blank line>
obj-$(CONFIG_DRM_GMA500) += gma500_gfx.o
<blank line>
```

The exact archive's Makefile has only 56 lines and ends immediately after the
newline terminating the declaration. There is no line 57. Archive member,
qualified source and fresh module copy are byte-identical: 1,113 bytes,
SHA-256 `0166a146c2b03ee7065384598e5ee55b870e7e37bfe4b03c75373469636aa6ad`.
Patch SHA-256:
`e0537bb0b72c1513de1c0ad6e9fa2c32706aeb0c2f747d22fe8f2511b2fc5467`.

The exact command, working in the fresh `module/` copy, was:

```sh
/usr/bin/patch --batch --fuzz=0 -p5 \
  -i /home/gama/sgx535-gfx/kernel/sgx535_frozen/patches/antix-fixed-makefile.patch \
  --dry-run
```

GNU patch 2.8 returned 1: `Hunk #1 FAILED at 55.` No patch was applied.
The remaining patches were inspected using read-only strict dry runs:

| Retained patch | Exit | Observation |
| --- | --- | --- |
| Makefile | 1 | Missing final blank context line, proven above. |
| IRQ | 0 | Strict dry run accepts the patch; it was not applied. |
| Ioctl | 1 | Both hunks rejected at 31 and 103. |

For the ioctl patch, both old-context blocks occur byte-exactly at the stated
source lines. That source hash is
`db2699f9301563dfe7cd8c87e7087d00a1c7af432a41161a6e2ec21b07d3d576`.
The precise reason GNU patch rejects its hunk representation remains
**UNKNOWN**; context equality alone is not a successful application check.
No relaxed-fuzz application, alternative patch tool, handwritten Makefile,
added EOF blank line or source correction was attempted.

Correcting retained integration inputs exceeds this task's compilation-only
scope. This is a concrete offline input blocker, not a compiler, ABI or target
failure. The old build's edited Makefile is historical evidence, not a
replacement qualified integration input.

## Qualified inputs verified before the STOP

| Input | Identity / SHA-256 |
| --- | --- |
| Exact antiX source archive | `e5c5d7c6bdcc7a845589e5956cbf50a8d6198232df017f0398a2902371042e1d` |
| Exact source-package diff | `5c41ae456d77d9e0af181fe3153c4f80e89bacdd2b541ba15fbac0b297d69b03` |
| Captured target config | `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9` |
| Qualified target Module.symvers | `faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca` |
| Successor `.config` | `a9f866159ee08437f22c569b0f6864dd2d9cf8f4b777dd90d0397eba339f5ca4` |
| Source | `/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp` |
| Qualified output | `/home/gama/sgx535-offline/antix-kbuild-successor-20260930/output` |
| Fresh module staging | `/home/gama/sgx535-offline/candidate-01-20260930/module` |

The 71 original gma500 source files and 15 current fixed source/header/include
inputs were copied and hashed. All remain unchanged. The seven kernel owner,
backend, entry and UAPI files plus eight contract/service/IO/generated-include
files are individually identified in `fixed-inputs.json`. No `.o` or `.ko`
was inherited into the module staging directory.

Actual executable identity and hashes match the qualified environment:

- Target `T/usr/bin/i686-linux-gnu-gcc-14 --sysroot=T`: GCC 14.2.0
  (Debian 14.2.0-19), target `i686-linux-gnu`.
- Target `T/usr/bin/i686-linux-gnu-ld.bfd` and `...-as`: GNU binutils 2.44.
- Native HOSTCC `/usr/bin/gcc-14`: GCC 14.2.0, `aarch64-linux-gnu`.
- `T=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root`.

All 20 retained executable hashes were checked. Intended build settings remain
`ARCH=x86`, absolute qualified cross prefix, explicit target sysroot, native
HOSTCC include/library paths and `LOCALVERSION=-486-smp`; the exact selected
environment is retained. They were **not consumed by candidate Kbuild**, because
patch qualification stopped first. No Clang or AArch64 `-m32` was substituted.

The successor's 7,426 config/generated-state file hashes are unchanged; all
10 selected source-rule hashes match. All prior qualification/preparation
artifact manifests pass again (39, 111 and 102 entries). The failed predecessor's
10,569 entries retain hashes, links, modes, inodes and mtimes. The captured
target table is unchanged and was **not installed** into the successor.

## Fresh verification results

| Check | Result |
| --- | --- |
| Scoped Python suite | 193 tests PASS, exit 0. |
| Service, IO and contract C harnesses | Three strict GCC 14 / UBSan builds and runs PASS, six exit-0 steps; no sanitizer diagnostic. |
| Generated frozen initial-image check | PASS, exit 0. |
| Two frozen dry runs | Both exit 0, byte-identical; SHA-256 `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`. |
| `--complete` | Expected exit 1: `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`. |
| Original module vs qualified table | PASS: 222/222 covered, missing 0, CRC mismatches 0. |
| Rejected old candidate vs qualified table | REJECT: 228/228 covered, missing 0, CRC mismatches 154, exit 1. |

The original module's `module_layout` remains `0xb84efb99`. The old rejected
candidate remains `0x995e9910`, SHA-256
`934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`.
The checker reports its literal `REFERENCE_IMPORTS_ONLY` scope; there is no
new artifact whose imports could be qualified.

## Candidate decision and next step

No new `.ko` exists. New size, SHA-256, Build ID, ELF/version sections, vermagic,
module_layout, imports/dependencies and signing metadata are **NOT AVAILABLE**.
Candidate-only/removed import counts and repeat-build reproducibility are
**NOT EVALUATED**. No compilation/modpost failure was observed because neither
ran. **ABI QUALIFICATION: NOT ESTABLISHED.**

The preparation/toolchain gates remain PASS; the source/patch integration gate
does not pass. No implementation/test predicate or historical evidence was
changed. No target contact, deployment, SGX action, staging, commit or push.
Gate B remains BLOCKED and whitelist remains `[]`.

**SINGLE SMALLEST NEXT STEP — OFFLINE:** separately scope correction/review
of the retained Makefile and ioctl integration patches against the exact
qualified antiX source, preserving their intended fixed-service semantics,
and require strict no-fuzz dry-run success before resuming candidate build.
Do not reuse this partial staging directory as a qualified build. The
hot-transition/IRQ lifecycle/recovery blocker remains separate; even a later
ABI-compatible module would not authorize deployment.
