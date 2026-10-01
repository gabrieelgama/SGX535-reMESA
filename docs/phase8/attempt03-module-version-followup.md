# Attempt 03 module-version follow-up (offline)

**Current status:** See the [post-Attempt-03 checkpoint audit](post-attempt03-checkpoint-audit.md).
The later original-driver hot recovery faulted; the operator subsequently
reports a reset and normal display operation. Detailed post-reset state is
not retained. The HOLD/recovery paragraphs below preserve the earlier
checkpoint and must not be used as a current live-state assertion. The ABI
findings remain unchanged; no compatible replacement has been built.

Attempt 03 stopped before the fixed ioctl. The original module was removed
normally, but the pinned candidate was rejected with `Invalid module format`;
the retained kernel log says `gma500_gfx: disagrees about version of symbol
module_layout`. No candidate driver, TA/raster fire, or color result followed.
The attempt ended in the [recorded HOLD state](../hardware-evidence/MINI12-20260930-TRIANGLE-ATTEMPT-03/RESULT.md).

## Exact ABI evidence

The [static comparison](attempt03-module-version-comparison.json) reads the
ELF32 `__versions` sections of the preserved installed target module and the
rejected candidate. Both report vermagic
`5.10.240-antix.1-486-smp SMP mod_unload modversions 486` and ELF32 i386.
Those matching fields did not establish module ABI compatibility.
The `.modinfo` dependency lists also differ: the candidate adds `backlight`
to the original's `drm,drm_kms_helper,video,i2c-algo-bit` list. This is a
separate metadata difference; the observed loader refusal names
`module_layout`.

| Artifact | SHA-256 | `module_layout` import CRC | Imported symbols |
| --- | --- | --- | ---: |
| Preserved installed `gma500_gfx.ko` | `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb` | `0xb84efb99` | 222 |
| Rejected candidate `gma500_gfx.ko` | `934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf` | `0x995e9910` | 228 |

Of 219 shared imported symbol names, **150 CRCs differ**. The candidate also
imports nine names absent from the preserved module: `drm_clflush_pages`,
`memcmp`, `memcpy`, `module_put`, `request_resource`, `try_module_get`,
`usleep_range`, `vmap`, and `vunmap`. The preserved module cannot establish
the target CRCs for those nine imports. Its GCC 14.2.0 `.comment` differs
from the candidate's Clang 21.1.8 `.comment`; that identifies a build-input
difference, not the proven cause of the supplied table's CRC mismatch.

## Candidate build provenance and limit

The exact-version public antiX source package is
`linux-5.10.240-antix.1-486-smp-6`. Its retained
[`antix-package.diff`](../phase4-2-data/antix-package.diff) **adds** a
25,195-line `Module.symvers`, including `module_layout 0x995e9910`. Its bytes
match the extracted build tree's `Module.symvers` and `vmlinux.symvers`
(SHA-256 `f4732fe605f0bda023e1f83d4a3ff0e61460f7c7e1eca92599235047a1b35edc`).
The package config enables `CONFIG_MODVERSIONS=y` and records GCC 14.2.0;
the temporary build tree uses Clang 21.1.8 and sets
`CONFIG_LOCALVERSION="-486-smp"`. Its generated `utsrelease.h` matches the
target release; its generated `autoconf.h` reflects that Clang configuration
and `CONFIG_MODVERSIONS=1`. The corresponding generated headers from the
*running target kernel* are not retained. `scripts/Makefile.modpost` takes
CRCs from the symbol table, and the generated `gma500_gfx.mod.c` contains
`{ 0x995e9910, "module_layout" }`. The candidate's `__versions` retains it.
All 228 candidate import CRCs match entries in the extracted package table.
Thus the package-supplied symbol table, not a guessed runtime version or a
vermagic-only check, is the demonstrated route by which the wrong CRC entered
the candidate.

The retained package `.config` and temporary candidate `.config` differ in
22 `CONFIG_` assignments, including GCC versus Clang/LLVM, local version,
and `CONFIG_FORTIFY_SOURCE`. Both retain `CONFIG_X86_32=y`, `CONFIG_M486=y`,
`CONFIG_SMP=y`, and `CONFIG_MODVERSIONS=y`. The candidate compile command uses
Clang's `i386-linux-gnu` target, `-m32`, and `-march=i486`; the preserved
original module's `.comment` names GCC 14.2.0. These facts verify the
candidate's architecture and build inputs, but the running kernel's actual
`.config`, generated architecture headers, compiler flags, and export table
are not retained. Matching the release string did not match the ABI.

| Retained build input | SHA-256 |
| --- | --- |
| Package `.config` | `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9` |
| Temporary candidate `.config` | `804baeeeac8f5baa7586091130cc8784c045783fb47767b616d65727f5f6e630` |
| Candidate generated `include/generated/autoconf.h` | `0c8f61bf9c5809c9e83b56d9fc6a3af2d9192851ce7b7802f97c86c40d406e83` |
| Candidate generated `include/generated/utsrelease.h` | `829215a06900fa9d8cd030792507ba37742880744ff62c123de128c3fb936cf6` |

All 222 original-module import names are present in the package table, yet
150 of their CRCs differ. The package's table is therefore **not an
ABI-equivalent target symbol table** for the installed module. Retained
evidence does not establish why the package table and installed target kernel
diverged: a stale/foreign table and a differently built installed kernel are
both possible. The complete target export table, target build config, and
target-generated headers are not retained. Replacing only the
`module_layout` value, copying the original module's partial CRC list, or
forcing the load would leave other mismatches or unverified imports. No
corrected module was built, and the rejected artifact remains unchanged.

The fail-closed checker
[`frozen_module_versions.py`](../../tools/psb-dri-re/frozen_module_versions.py)
compares every candidate import against the preserved installed module. It
rejects any shared CRC mismatch and any candidate-only import. It rejects the
Attempt 03 candidate with 150 mismatches and nine unverified imports. Its
synthetic ELF32 regression tests cover a `module_layout` mismatch, a
non-`module_layout` mismatch, a candidate-only import, and a fully matching
set. This check must precede any future module-transition review. A passing
comparison against one preserved module would prove only the covered imported
CRCs, not all target runtime properties.

Its `--check-symvers` mode also checks a proposed **build table before
compilation** against every import CRC in the preserved installed module. On
the extracted package table it returns `REJECT`: 150 mismatches, zero missing
reference imports, and `module_layout` `0xb84efb99` versus `0x995e9910`.
The table has 25,195 entries. A `PASS` from this mode means only that the
table covers and matches the preserved module's imports; it does **not**
verify a future candidate's new imports or establish target provenance.

## HOLD and next evidence boundary

The target's recorded final state is: `slimski` down, no `gma500_gfx`, no
`/dev/dri/card0`, PCI `0000:00:02.0` unbound, and `vtcon0` bound. The original
module's removal also emitted `drm_mode_config_cleanup`/`ida_free` warnings.
There is no qualified automatic restore after that state; loading the
original module, restarting display service, or rebooting would be a separate
reviewed **target-state-changing recovery**, not part of this offline work.
Attempt 03's authorization is exhausted. The old whitelist entry pins the
rejected candidate hash and is not usable for another attempt.

A recovery review would first need a fresh passive confirmation of that HOLD
state and of the preserved original module's hash, then a guarded **normal,
nonforced** original-module insertion and verification of PCI/DRM/fbdev and
console ownership before any `slimski` restart. It would need a final passive
comparison with the pre-attempt display/service baseline and a stop boundary
at every mismatch. The unload warnings and the missing live driver mean the
success and safety of that restoration are **UNKNOWN**; this paragraph is a
review requirement, not an authorization or a tested recovery sequence.

The smallest missing fact for a compatible rebuild is the **complete
symbol-version table used by the running target kernel**, especially CRCs
for the nine candidate-only imports. A narrowly scoped future **read-only
target observation** could inspect whether
`/lib/modules/5.10.240-antix.1-486-smp/build/Module.symvers` (or another
installed, attributable target build artifact) exists, capture its hash and
required symbol CRCs, and compare its `module_layout` CRC with `0xb84efb99`.
That observation must not load a module, open DRM, change PCI/display state,
or run the fixed client. If no target-qualified full table exists, the
candidate cannot be corrected from the retained module's partial imports
alone. No such target observation was performed in this follow-up.

For a genuinely ABI-compatible rebuild, the target-qualified table must be
paired with source/headers, `.config`, generated configuration and
architecture headers, release metadata, and compiler/ABI settings attributable
to the **installed running kernel**. The preserved module establishes only
its own imported CRCs, format, metadata and GCC `.comment`; it is not a
complete build environment or a substitute for those missing inputs.

## Subsequent installed-kernel capture — 2026-09-30 19:10 UTC

[Capture 08](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/RESULT.md)
supersedes the missing-target-input statements above. Installed i386 image
and headers have exact version `5.10.240-antix.1-486-smp-6`; their captured
export table SHA-256 is
`faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca`.
All 222 preserved original imports match, including module_layout b84efb99;
all nine candidate-only imports now have attributable target CRC evidence.
Against that complete coverage, the rejected candidate has **154** mismatches
(not the 150 shared-import mismatches measured against the original alone).
The recorded boot/header/public-package config is byte-identical, generated
configuration is consistent, and installed Makefile matches retained source.
The public export table differs at 17,520 shared symbols and has 1,275 more
entries; its generation provenance remains UNKNOWN. Identical package config
rules out that recorded config difference as the explanation, without proving
which other producer inputs caused the stale/incompatible table.

No new candidate was built. The local GCC targets AArch64; an offline i386
GCC route preserving the captured GCC configuration must be established.
A full prepared header package was not streamed, so source preparation must
regenerate remaining config stubs/helpers consistently, or use that exact
package. The target is restored; ABI evidence closure is not hot-transition
qualification, deployment authorization or live SGX proof. Gate B BLOCKED;
whitelist `[]`; no Attempt 04.
