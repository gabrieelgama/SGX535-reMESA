# Candidate #1 build and independent IRQ lifecycle review

Started 2026-09-30; completed 2026-10-01 UTC.

**NEW CANDIDATE #1: BUILT. ABI QUALIFICATION: PASS offline.**
**Lifecycle derivative: BUILT. ABI QUALIFICATION: PASS offline.**
**Gate B: BLOCKED. Active whitelist: `[]`. Target contact: NONE.**

This continuation resolves the preceding mechanical integration STOP and then
implements/tests a separate IRQ teardown correction. It does not deploy either
artifact or reuse Attempt 03's spent authorization. The previous
[candidate staging STOP](candidate-01-offline-build-qualification.md), failed
preparation, rejected module, Attempts 01–03 and recovery evidence are unchanged.
The [new evidence index](artifacts/candidate-01-build-20260930/README.md) contains
commands, input/source hashes, failed builds, successful builds, complete symbol
checks, regression output, source contracts and repeat comparisons.

## Integration diagnosis and exact correction

**CONFIRMED-FROM-ARTIFACT / CONFIRMED-BY-TEST:** the exact Makefile ends at
line 56. The old patch requests a nonexistent blank line at line 57. Regenerating
its diff against the actual EOF preserves the existing fixed object additions.
No source padding was introduced to accommodate obsolete context.

**CONFIRMED-FROM-LOCAL-TOOL-DOCUMENTATION / CONFIRMED-BY-TEST:** the ioctl
hunks have prefix/suffix context lengths 3/1 and 2/1. GNU patch specifies that
more prefix than suffix context anchors a zero-fuzz hunk at EOF. Both hunks are
interior. This explains rejection despite exact text occurrences. Regenerating
balanced three-line context preserves exactly the existing two includes and
root-only fixed ioctl descriptor. The old failing patches and manual excerpt
are retained, including their hashes. No ioctl/UAPI behavior was redesigned.

All three integration patches pass strict dry and actual application with
`--batch --fuzz=0 -p5`: **no offsets, no fuzz, no rejects**. Golden-source tests
compare the resulting files to the intended exact bytes and reject context drift.
The original fixed IRQ capture patch is byte-unchanged.

## First normal Kbuild and supported mechanical correction

The first GCC build compiled the sources but modpost rejected undefined
`__umoddi3`. The i386 object and RED regression prove its origin: a 64-bit
remainder against a runtime alignment in `frozen_kernel_contract.c`.
All canonical alignment requirements are powers of two. A zero/non-power-of-two
check plus equivalent unsigned bit-mask alignment test removes the unsupported
runtime helper without narrowing addresses or changing the frozen contract.
The qualified i386 freestanding-object regression now passes without a skip.
The failing modpost build remains preserved. No CRC/version record was edited.

A later development staging guard caught a stale pre-correction source digest.
Its accidentally launched incomplete build was terminated and retained as
**rejected staging, never a qualified candidate**. A fresh tree was constructed
from the verified successful source manifest. A pre-review lifecycle development
build is also preserved separately; it is not the final derivative below.

## Exact qualified inputs and build route

- Exact source: `/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp`.
- Qualified generated output: `/home/gama/sgx535-offline/antix-kbuild-successor-20260930/output`.
- Fresh external module workspace: `/home/gama/sgx535-offline/candidate-01-build-20260930/module`.
- Source archive SHA-256: `e5c5d7c6bdcc7a845589e5956cbf50a8d6198232df017f0398a2902371042e1d`.
- Source package diff SHA-256: `5c41ae456d77d9e0af181fe3153c4f80e89bacdd2b541ba15fbac0b297d69b03`.
- Captured target config SHA-256: `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9`.
- Target `Module.symvers` SHA-256: `faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca`.

**CONFIRMED-FROM-IMPLEMENTATION / CONFIRMED-BY-TEST:** ordinary external
`make -C <source> O=<qualified-output> ARCH=x86 M=<fresh-module> V=1 modules`
uses qualified GCC 14.2.0 `i686-linux-gnu-gcc-14` with its explicit sysroot,
GNU i686 binutils 2.44, native `/usr/bin/gcc-14` HOSTCC, and
`LOCALVERSION=-486-smp`. Full argv/environment and executable identities are
captured. Kbuild trace shows `-m32 -march=i486`, GNU BFD `-m elf_i386`, and
native AArch64 host helpers. External modpost consumes `-e -i Module.symvers`,
which is the unchanged captured target table copied as an ordinary Kbuild input.
No Clang, old public/vmlinux table, MODVERSIONS bypass, forced loader operation,
manual ELF construction or metadata patching was used.

All **7,426** baseline configuration/generated-state hashes remain unchanged
through each successful build and repeat; no new configuration/generated-header
entries appear. The previously qualified compiler-banner difference remains
classified by the successor report. The exact 71 original + 15 fixed module
inputs and patched result are hashed. No unexplained config/header/layout drift
was observed.

## Two distinct artifacts; neither was deployed

| Field | Candidate #1, integration/contract correction | Lifecycle derivative (candidate #2) |
| --- | --- | --- |
| Size | 242,364 bytes | 242,724 bytes |
| SHA-256 | `13591674ef9fa82f185f075185d1fa09d94606ccce7253ed231627f2649c600f` | `91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74` |
| GNU Build ID | `d72f19c97c5fd1feffddad07634e12333562ae60` | `074c650d48ccb463e37e90428b486dccdb23eb80` |
| ELF | ELF32, little-endian, ET_REL, Intel 80386 | Same |
| Vermagic | `5.10.240-antix.1-486-smp SMP mod_unload modversions 486 ` | Same, including trailing space |
| module_layout | `0xb84efb99` | `0xb84efb99` |
| Versioned imports / target coverage | 230 / 230 | 232 / 232 |
| Missing imports / CRC mismatches | 0 / 0 | 0 / 0 |
| Actual undefined symbols | 229, all versioned; extra version record is module_layout | 231, all versioned; same synthetic record |
| Candidate-only imports vs original | 8 | 10 |
| Removed original imports | None | None |
| Repeat build | BIT IDENTICAL | BIT IDENTICAL |
| Offline ABI qualification | PASS | PASS |

The eight additional candidate #1 imports are `drm_clflush_pages`, `memcmp`,
`module_put`, `request_resource`, `try_module_get`, `usleep_range`, `vmap`,
`vunmap`. The derivative additionally imports `drm_irq_uninstall` and
`drm_kms_helper_poll_disable`; both CRCs are independently checked against the
qualified target table. Each candidate's **complete actual import set** is
checked directly; original/candidate overlap is not substituted for coverage.
Dependencies are `drm,drm_kms_helper,video,i2c-algo-bit`.
Both artifacts are unsigned. Captured config enables module signatures but
not forced/all signing; current runtime enforcement is **UNKNOWN**, and no
loader acceptance is claimed.

Each repeat starts with a fresh external source tree at the same `M=` path and
identical pinned legitimate build metadata/environment. No previous objects are
reused. The first and repeat `.ko` bytes are compared directly. The old candidate
SHA `934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`
remains rejected: layout `0x995e9910`, 154 mismatches. The preserved original
still passes 222/222 against the target table. These are distinct artifacts,
not a repaired or relabeled old binary.

## Independent IRQ/lifecycle correction and its evidence boundary

**CONFIRMED-FROM-IMPLEMENTATION:** original `psb_driver_load` calls ordinary
`drm_irq_install`; original unload frees private/MMIO state without matching
`drm_irq_uninstall`. The exact DRM helper does not automatically clean this
registration: its uninstall calls the hardware callback and `free_irq`, whose
contract drains executing handlers and requires device interrupts disabled
before shared-IRQ release. `drm_dev_unregister` does not supply that pairing for
this driver's explicit removal path. The retained original module has no `drm_irq_uninstall` import. It **does**
import `free_irq`, but disassembly attributes both call sites to the separate
Oaktrail HDMI-I2C init/exit functions. Its selected Poulsbo unload/remove/chip
teardown/modeset/power functions have no IRQ-release call. This distinction is
recorded in `original-irq-callsite-review.json`; import presence alone does not
prove the selected resource is released.

The separate `antix-irq-lifecycle.patch` now:

1. Stops KMS polling and calls `drm_crtc_vblank_off` while CRTCs/private/MMIO
   remain valid, before IRQ unregistration and existing modeset teardown.
2. Calls `drm_irq_uninstall` exactly when IRQ registration is owned, before
   MMU/GTT/register/private destruction.
3. Checks the IRQ-install return value and routes it to existing cleanup.
4. Routes the late backlight-init error through the same existing unwind.
5. Clears all top-level device interrupt routing in the uninstall callback
   when `irq_enabled` is false. Exact DRM core clears it before this callback;
   direct PM suspend leaves it true, so the historical suspend routing mask is
   preserved. Existing mask/enable writes, barrier and posted identity read are
   used; no reset, power sequencing or new SGX register/value is introduced.

Apply **Makefile → fixed IRQ → lifecycle → ioctl**. The lifecycle IRQ hunk is
based on the retained fixed-IRQ result; its driver hunk precedes ioctl additions.
Both full dry and actual applications pass with zero fuzz/offsets/rejects.

**CONFIRMED-BY-TEST:** the UBSan kernel-boundary harness executes the actual
extracted unload, install/error statements, actual error label, and IRQ callback.
Seven tests cover ten scenarios: no private state, no owned IRQ, active vblank,
IRQ before modeset, install failure/success, late-init failure/success, all-source
masking before handler release, and unchanged direct-PM mask. RED tests exposed
both missing IRQ release and retained-routing defects; GREEN passes. It does
not simulate hardware or prove general hot-unplug safety. A fresh read-only
review found no remaining critical/important issue in the scoped correction.

**INFERRED:** stale IRQ action/name lifetime is consistent with the original
restoration `strcmp → register_handler_proc → __setup_irq` oops. The exact
faulting pointer and complete crash cause remain **UNKNOWN**; source correlation
is not promoted to proven causation.

**HARD TRANSITION BLOCKER:** these changes only run once the new derivative is
loaded. They cannot repair removal of the **currently active unpatched original**.
Its first hot unload remains unqualified; neither a zero module refcount nor
an ABI-qualified replacement guarantees release of the original IRQ action.
The previous removal/restoration procedure must not be replayed.

## Gate B re-evaluation for the current checkpoint

| Exact prerequisite | Classification | Evidence / limit |
| --- | --- | --- |
| Target identity / original post-reset state | PASS for retained observation | Capture 08; not a fresh deployment check |
| Fixed UAPI, programs, ten BOs, 49 relocations, one-attempt/HOLD model | PASS offline | Existing tests/images; no new live execution proof |
| Candidate artifact/ABI/import/source/config/toolchain | PASS offline | Both artifacts above; derivative includes cleanup correction |
| Derivative IRQ cleanup ordering/routing | PASS for scoped offline obligations | Actual-source harness, exact Linux contracts, normal Kbuild; live behavior unobserved |
| First removal of active original + normal hot restoration | HARD BLOCKER | Original lacks paired IRQ release; retained recovery oops; derivative cannot repair original |
| Alternative first-load/known-good fallback route | NOT ESTABLISHED | Bootloader/initramfs/module-loading provenance not retained sufficiently |
| Publication, L12, FT-AUX, ISP reset | Historical accepted assumptions | Architectural UNKNOWNs unchanged; no general proof or renewed execution authorization |
| Fresh target preflight / new exact deployment authorization | NOT PERFORMED / NOT AUTHORIZED | Prior attempt authorization spent; no target contact in this task |
| Gate B / active whitelist | BLOCKED / `[]` | No reviewed executable transition from current original state |

There is no TA/raster submission, completion observation, color readback or
32×32 triangle demonstration in this continuation. `--complete` retains its
architectural scope and exact four labels. ABI compatibility does not resolve
L12, FT-AUX, or live FT-BO/FT-SERVICE qualification.

## Verification and preservation

- Full scoped suite: **205 tests PASS, zero skips** (193 initial + 4 strict
  integration + 1 qualified i386 object + 7 lifecycle tests).
- Three strict native UBSan harnesses: PASS; lifecycle UBSan runs additionally
  execute within the seven new tests.
- Generator `--check`: PASS.
- Two dry-run hashes: both
  `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
- `--complete`: expected exit 1, `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.
- Strict patch application, import checks, source/config/helper checks and
  clean repeat comparison: PASS.
- `git diff --check`, scoped diff review, historical input/evidence preservation
  and final Git-visible inventory are recorded in the evidence bundle.

No target contact, transfer, deployment, fire, force operation, reset, stage,
commit or push. Only intended source/test/doc changes and deliberate evidence
artifacts were added. One preexisting Python cache was refreshed by an early
focused test; newly created cache files were removed, unrelated files preserved.

## Single smallest next step

**READ-ONLY TARGET OBSERVATION — a separately authorized boot-route inventory**
identifying exactly where the normal boot obtains its first `gma500_gfx` module
and the existing original-module fallback. Retained evidence names the kernel
boot image but does not preserve the selected bootloader entry/initramfs contents.
A cold-start-only route could avoid unsafe removal of the active original, but
its concrete loading/fallback mechanism cannot be qualified offline from the
current artifacts.

Proposed observation only, **not executed or authorized here**: read the current
`/proc/cmdline`, `/boot` names/symlink metadata and the existing selected boot-entry
configuration; after identifying its exact initramfs path, preserve that regular
file through the approved read-only artifact capture mechanism and inspect its
module/loading contents offline. Compare any embedded original module with the
preserved original hash. Stop on different kernel/entry, missing/ambiguous path
or unexpected original identity. No writes, service/VT changes, module operation,
boot selection, reboot, DRM/MMIO/ioctl or SGX work belongs to that observation.
Then review an exact first-load/recovery route and obtain new authorization;
no cold boot, installation or Attempt 04 is authorized by this report.
