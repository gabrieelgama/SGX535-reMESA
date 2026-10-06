# ChatGPT handoff — post-Attempt-03, qualified target ABI evidence

## Current result: cycle02 (2026-10-01)

[Cycle02 evidence](first-load-cycle-02-result.md) records a corrected and tested health guard, a fresh 49-guard
preflight and exact exclusive staging, all PASS. The operator reported normal
userspace after one experimental boot. The sole experimental root capture never
ran because sudo was unavailable. We have no loaded derivative identity, hook
trace or experimental boot ID. There was no retry.

After the operator crossed the machine boundary, the stock recovery capture
passed 54 guards. It verified the original note, PCI/DRM/fb/VT/IRQ ownership,
services, files and default. The operator confirmed that the display was normal.
Full first-load and three-boot recovery qualification remain NOT ESTABLISHED.
Capture-readiness, boot-ID/timing and existing-created-file guards are now
tested offline for a separately authorized successor.

304 tests passed with zero skips, along with 14 boot-analysis tests, three UBSan
harnesses and the generator/CRC/dry guards. Gate B BLOCKED; whitelist []; no SGX
action; FIRST TRIANGLE NOT ATTEMPTED.

The next step needs new explicit authorization for a non-SGX first-load/recovery
cycle. Preserve the staged files and revalidate their exact creation identities.
Do not overwrite them or use a hot transition. The notices below are historical.

### Earlier preflight: cycle01 (2026-10-01)

[Cycle01 result](first-load-cycle-01-result.md): the separately authorized non-SGX cycle STOPPED BEFORE
STAGING. The physical/operator prerequisites were confirmed. After local sudo
authentication, the root read-only preflight checked stock identity, ownership
and services, then rejected three preexisting diagnostics. The complete fresh
dmesg matches the retained log from the same stock boot. The OFFLINE guard and
baseline disagree; there is no evidence of a new SGX fault. No guard was waived
or changed. No staging, experimental selection, reboot, module/service/PCI/VT
mutation, fixed ioctl or SGX fire occurred. Live first load and recovery
remained UNKNOWN. Gate B BLOCKED; whitelist `[]`; FIRST TRIANGLE
NOT ATTEMPTED. The next step at that point was OFFLINE testing of precise
baseline-versus-fault health discrimination. No further target action occurred
in that stopped cycle.

### Earlier offline operational review (2026-10-01)

The staging, manual first-load and operator reset-to-stock design review is in
the [qualification report](first-load-operational-review.md), [exact procedure](first-load-staging-boot-recovery-procedure.md) and [checklist](first-load-operator-checklist.md). No pinned
image or candidate changed; there was no target contact or staging. 266 tests
passed with zero skips, along with 14 boot-analysis tests and three UBSan
harnesses. The non-SGX procedure was READY TO REQUEST SEPARATE LIVE
AUTHORIZATION, subject to fresh stock, privilege, physical-control and visible
menu guards. Live first ownership, display, SSH and recovery remained UNKNOWN.
Delivery of the volatile hook log after pivot was UNKNOWN; without the trace,
qualification could not pass. No hot transition, restoration or SGX action was
authorized. Gate B BLOCKED; whitelist `[]`; FIRST TRIANGLE NOT
ATTEMPTED. Earlier next-step notices are historical.

### Earlier offline image qualification (2026-10-01)

The [experimental first-load image report](experimental-first-load-image-qualification.md) and [evidence](artifacts/experimental-first-load-01-20261001/README.md) record PASS OFFLINE for
construction, guards, pre-udev ordering, exact dependency CRC coverage, stock
preservation and non-saving GRUB text. The final image is 50,804,481 bytes,
SHA-256 `4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`. Two corrected builds are byte-identical. 249 tests
passed with zero skips, including 34 new image checks. The 14 boot-analysis
tests and three UBSan harnesses also passed. No module was rebuilt, no target
was contacted or changed, and no image/entry was installed. There was no SGX
fire. Early first ownership, display/SSH and reset/fallback remained UNKNOWN.
The next step then was the OFFLINE staging, experimental-boot and reset-to-stock
recovery review, followed by separate authorization if justified. Gate B
BLOCKED; whitelist `[]`. Earlier “no image constructed” and
next-step statements below are historical.

### Earlier read-only boot observation (2026-10-01)

[Stock boot and first-load design](first-load-stock-boot-observation.md) supersedes older statements below that the
boot/initramfs/fallback files were absent. The separately authorized capture
passed identity/end guards and found a normal stock display. It captured the
GRUB saved stock entry and exact image hashes. The stock initramfs contains no
gma500; its udev trigger runs before /conf/modules loading. Root eudev modalias
coldplug remains an INFERENCE: no requester PID was traced. GRUB's existing
custom.cfg sourcing supports a separate non-saving entry. No entry or image was
created in that task. The next step then was separately scoped OFFLINE
image/entry construction and qualification. Experimental first-load, display and
reset-fallback remained BLOCKED or UNKNOWN pending that work, new boot
authorization and live preflight. Gate B BLOCKED; whitelist `[]`;
target contact READ-ONLY only; SGX fire NO. Earlier no-contact and
missing-boot-file statements describe their historical turns.

### Earlier first-load review (2026-10-01)

[Offline boot/first-load qualification](first-load-boot-qualification.md) proves that the same alias can bind an unbound device.
Kernel-name blacklisting cannot distinguish original and derivative. Seven new
tests using actual source and native UBSan brought verification to 215 tests,
zero skips. The derivative retains full KMS/fbdev and could become the first
Linux PCI driver owner without removing the original. At that point, the actual
boot entry, initramfs/load policy and stock fallback had not been captured.
First-load alternative/fallback remained UNKNOWN/BLOCKED. Gate B BLOCKED;
whitelist `[]`; no target contact. The next step then was a
separately authorized read-only boot-selection inventory. It did not authorize
boot modification, insertion, SGX execution or replaying hot restoration.

### Earlier focused lifecycle review (2026-10-01)

The [first-original-removal Gate B review](lifecycle-first-removal-gate-review.md) verified both frozen candidates, complete target CRCs and
preserved bit-identical repeats without rebuilding. Three new actual-source PCI
remove, module-exit and immutable-original control tests brought verification to
208 tests, zero skips. The PCI/devres and legacy IRQ-control paths do not supply
the original's missing IRQ release. The derivative fixes its own teardown; it
cannot change the active original. The review classifies every Gate B predicate
individually. Gate B BLOCKED; whitelist `[]`; no target contact.
The separately scoped boot-route observation had not run. Neither replaying hot
removal nor another deployment was authorized.

Updated: 2026-10-01. Repository: `/home/gama/sgx535-gfx`.
**FIRST TRIANGLE: NOT ATTEMPTED at the SGX submission stage.**
**Gate B BLOCKED; whitelist `[]`; no new SGX attempt is authorized.**

This handoff describes the current checkpoint, not the historical Gate B PASS
or the machine's earlier recovery HOLD snapshots. Read this, the
[checkpoint audit](post-attempt03-checkpoint-audit.md), and the
[offline GCC research/build plan](i386-gcc-offline-build-plan.md) first.
The preceding research turn changed documentation only. The subsequent
[toolchain qualification](i386-gcc-toolchain-qualification.md) provisioned a
rootless local cross-toolchain and ran smoke tests. The subsequent isolated
[Kbuild preparation](antix-kbuild-preparation-equivalence.md) completed its
commands but FAILED the header comparison on a truncated syscall header.
A separate [fresh-output successor](antix-kbuild-successor-equivalence.md)
now PASSES the complete documented preparation/config/header/layout gate;
the failed output/evidence remain unchanged.
**Current offline build/lifecycle continuation:** [qualified candidate results](candidate-01-build-and-lifecycle-review.md)
close the mechanical patch and module ABI blockers. Candidate #1 and the separate
IRQ-lifecycle derivative both build through normal external Kbuild, match every
actual import against the captured target table, and repeat byte-identically.
The derivative now pairs IRQ release before private/MMIO destruction and masks
all device routing on actual handler removal, preserving direct-PM behavior.
**The first hot removal of the active unpatched original remains BLOCKED.**
The correction cannot change that original's teardown. Gate B remains BLOCKED,
whitelist `[]`; no target contact or SGX action occurred. The next missing fact
is the normal first-load boot/initramfs source and original fallback, requiring
a separately authorized read-only boot-route inventory before a cold-start-only
alternative can be reviewed. Current verification: 205 tests, zero skips; three
UBSan harnesses plus lifecycle UBSan PASS; dry-run hash unchanged; `--complete`
retains its four labels. Neither artifact is authorized for deployment.

## 1. Project, target and experiment

SGX535-reMESA aims to run one provenance-backed, fixed **32×32 triangle** on an
owned Dell Inspiron Mini 12/1210, Intel Poulsbo/US15W GMA500, PowerVR SGX535.
Historical separately authorized observations were CORE_ID `0x01130000` and
CORE_REVISION `0x00010201`: revision 1.2.1/rev121. PCI revision 0x06 is a
separate namespace. Do not repeat hardware identity reads to rediscover this.
Target userspace is i686 antiX Linux; verified kernel release is
`5.10.240-antix.1-486-smp`.

The selected primary PDS program is the opaque historical literal pair
`0x07000345`, `0xaf000000`, bytes `45 03 00 07 00 00 00 af`. Do not decode or
edit it speculatively. Primary producer commit is 56 bytes: 48 data bytes then
the two words at +0x30/+0x34. +0x00 is relocated USE value U, +0x04=0,
+0x20=0x20; nine historical holes are deterministically zero by clean-room
construction. This is not architectural source-containment proof. Secondary
PDS is `00 00 00 af`; linked USE is `00 00 00 00 40 01 04 f8`.

## 2. Build/lifecycle checkpoint (historical; later results above)

| Item | Current classification |
| --- | --- |
| Phase 7 | COMPLETE WITH UNRESOLVED EVIDENCE BOUNDARY |
| Frozen CPU scene, primary/secondary/USE bytes and serializer | PASS offline |
| Frozen i386 client | PASS offline; unchanged, no rebuild required |
| Fixed kernel-owned service implementation | PASS for tested offline obligations; live operation unobserved |
| Post-reset original-driver/display state | PASS for the retained 19:10 UTC read-only observation |
| Target export-table provenance/original import coverage | Established; 222/222 match |
| Candidate-only target CRC evidence | Established for all nine names |
| Old candidate | REJECTED; 154 CRC mismatches against qualified target table |
| New candidates | Candidate #1 and lifecycle derivative: ABI PASS offline; not deployed |
| i386 GCC toolchain | PASS for local rootless provisioning/identity/smoke checks |
| Isolated Kbuild preparation/equivalence | PASS for fresh successor; 7,426 config/generated-state hashes unchanged across new builds |
| GMA500 hot-transition/recovery | BLOCKED; previous restore oopsed |
| Gate B / whitelist | BLOCKED / `[]` |
| Active SGX action | NOT PERFORMED; no TA/raster work occurred in Attempt 03 |

## 3. Attempt 03 and recovery chronology: PROVEN BY REPOSITORY EVIDENCE

[Attempt 03 evidence](../hardware-evidence/MINI12-20260930-TRIANGLE-ATTEMPT-03/RESULT.md)
records an explicitly authorized, operator-present transition. Graphical
service stopped; normal VT-console release dropped the original module's
reference count to zero. Nonforced removal succeeded. Insertion of the pinned
candidate failed with `Invalid module format`; kernel log named a
`module_layout` version disagreement. Candidate did not become the active
driver. The fixed client/ioctl was never invoked. No TA/raster submission,
color output or triangle followed. The spent attempt ended in HOLD.

The separate [original-driver recovery](../hardware-evidence/MINI12-20260930-ATTEMPT03-ORIGINAL-RECOVERY-01/RESULT.md)
verified the original installed hash, then tried one normal original load.
It faulted during IRQ registration (`strcmp → register_handler_proc →
__setup_irq → request_threaded_irq → drm_irq_install → psb_pci_probe`), exit
137; the module was left Loading with no usable DRM/framebuffer/service.
No load retry or speculative reset was performed by that procedure.
The operator later reported resetting the machine and seeing normal display
operation. That report was subsequently independently checked by authorized
read-only capture 08. Earlier HOLD/Loading snapshots are not current state.
Attempts 01, 02, 03 and failed recovery evidence must remain unchanged.

## 4. Exact hashes and versions: PROVEN BY REPOSITORY EVIDENCE

| Artifact | Location | SHA-256 |
| --- | --- | --- |
| Preserved original module | `docs/hardware-evidence/MINI12-20260927-H0/raw/installed-gma500_gfx.ko` | `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb` |
| Rejected candidate | `/tmp/sgx535-antix-source-6/source/drivers/gpu/drm/gma500/gma500_gfx.ko` | `934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf` |
| Captured target table | `docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/build_Module_symvers` | `faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca` |
| Old public/build table | old extracted build `Module.symvers`/`vmlinux.symvers`; added by `docs/phase4-2-data/antix-package.diff` | `f4732fe605f0bda023e1f83d4a3ff0e61460f7c7e1eca92599235047a1b35edc` |
| Captured boot/header .config | capture-08 `artifacts/boot-config`, `artifacts/build__config` | `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9` |
| Captured autoconf.h | capture-08 `artifacts/build_include_generated_autoconf_h` | `df4c243870d720c7fbb9481f0572cfd1fced552a24652d7a90474a3eb420340b` |
| Captured generated header tar | capture-08 `artifacts/build_generated.tar` | `f899bce235cc04a3aa3c74f569fe2ba793a0b606676144ff762a57be2aa1f45c` |
| Frozen static i386 client | `docs/phase8/artifacts/frozen-triangle-one-shot-i386` | `758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf` |
| Client source | `tools/psb-dri-re/frozen_triangle_one_shot.c` | `4271a62846a0f02694e574161658920987ee89ad69e3bf5b2be79fc83f5634d2` |
| Fixed UAPI | `kernel/sgx535_frozen/gma500_fixed_uapi.h` | `04dd2080deeb0056a366546fe27faddbdeff6c1657a2471c96e39749dc080420` |
| Retained exact source original archive | dsc-described original tar.gz; currently under `/tmp/sgx535-antix-source-6/` | `e5c5d7c6bdcc7a845589e5956cbf50a8d6198232df017f0398a2902371042e1d` |
| Exact source package diff.gz | `docs/phase4-2-data/linux-5.10.240-antix.1-486-smp_5.10.240-antix.1-486-smp-6.diff.gz` | `5c41ae456d77d9e0af181fe3153c4f80e89bacdd2b541ba15fbac0b297d69b03` |

Original loaded Build ID: `d8dcb4d38b774ad64799d5e13aaedede069371f3`.
Rejected candidate Build ID: `d4cb8d750efa6eb401f82e6eaec7eb82ebdf9c3c`.
Both modules have matching ELF32/i386 and vermagic:
`5.10.240-antix.1-486-smp SMP mod_unload modversions 486`.
Those matching fields did not establish ABI compatibility.

| Comparison | Result |
| --- | --- |
| Original module_layout / captured target table | `0xb84efb99` / `0xb84efb99` |
| Rejected candidate module_layout / old public table | `0x995e9910` / `0x995e9910` |
| Original versus rejected candidate | 219 shared imports; 150 CRC mismatches; nine candidate-only imports |
| Original versus captured target table | PASS: 222/222 covered, zero mismatches |
| Rejected candidate versus captured target table | 228/228 covered, **154 mismatches**, REJECT |
| Old versus target full tables | 25,195 versus 23,920 entries; 17,520 shared-symbol CRC mismatches |

The nine qualified target CRCs are `drm_clflush_pages=0xceb9549d`,
`memcmp=0x5152e605`, `memcpy=0x2e60bace`, `module_put=0x59ab9688`,
`request_resource=0xe0a6b585`, `try_module_get=0x502b702c`,
`usleep_range=0x12a38747`, `vmap=0x75f5ea6d`, `vunmap=0x94961283`.
Future imports need not have the same count; every future name needs checking.

## 5. Why the old candidate was rejected

**PROVEN BY REPOSITORY EVIDENCE:** the source package supplied the wrong table;
modpost consumed it; generated `gma500_gfx.mod.c` and ELF `__versions` contain
those CRCs. Every old candidate import matched that old table, while the
running kernel rejected module_layout. This is the demonstrated loader-failure
cause. See [follow-up](attempt03-module-version-followup.md) and capture-08
[candidate/table report](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/candidate-target-table-check.json).

**UNKNOWN:** why that public source-package table was generated differently.
The recorded public .config and captured boot/header .config are byte-identical.
Old candidate config changed 22 assignments and used Clang 21.1.8, versus
original GCC 14.2.0. Those are build differences, not proof that compiler
choice caused the public table's incompatible CRCs. Do not guess its producer
configuration or provenance. Do not repair the old .ko by editing CRCs.

## 6. Verified post-reset state and installed inputs

**PROVEN BY REPOSITORY EVIDENCE:** [capture 08](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/RESULT.md)
ran 2026-09-30 19:10:31–19:10:40 UTC, root read-only, exit 0 with empty stderr.
It verified the original hash/loaded Build ID/Live state, refcount 2; PCI
8086:8108 at 0000:00:02.0 bound to gma500; card0 associated with that PCI
function; gma500drmfb 1280×800, 32 bpp, stride 8192; vtcon0=0/vtcon1=1;
root sv status and runsv→slimski→Xorg. Boot ID was
`8ae37532-19d4-4ec7-9c29-a791acfd889f`; expected P/O/E taint was 12289 with
no additional bits. These are a dated retained snapshot, not a new live check.
No DRM node was opened and no graphics state was changed by the capture.

Kernel/image/header identity is `5.10.240-antix.1-486-smp`, package version
`5.10.240-antix.1-486-smp-6`, i386 packages. Build symlink resolves to
`/usr/src/linux-headers-5.10.240-antix.1-486-smp`; no source symlink exists.
dpkg attributes the table/config/generated headers to that exact installed
header package. Running proc/version and compile.h agree: GCC Debian
14.2.0-19, GNU ld 2.44, demo@antix1, kernel build #6 dated Aug 7, 2025.
All 7,384 enabled config assignments agree with auto.conf/autoconf.h after
two hex-zero normalizations; Makefile matches retained exact-version source.

Captured files and hashes are indexed in [artifact-manifest.json](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifact-manifest.json).
Raw output/transport metadata and generated-header tar are preserved there.
The capture is not a complete ready-to-run header package; missing config
stubs/build helpers must be prepared locally from the verified source/config.

## 7. Existing implementation map

**PROVEN BY REPOSITORY EVIDENCE:** the project already contains deterministic
CPU images; ten private object requirements; fixed SGX VA/BO ownership;
49 canonical relocations; synthetic Python/C equivalence; USE plan pixel=3,
vertex=4; qualified rev121 CPU branch; fixed XHW/bootstrap/TA/raster plans;
bounded polls and selected IRQ capture; one-attempt/FIRE_POSSIBLE/HOLD ledger;
32×32 color readback and summary. These host/software properties do not prove
runtime SGX mapping, source semantics, completion or color output.

Main files:

- `kernel/sgx535_frozen/gma500_bo_owner.[ch]`: kernel-owned BO/VA/mapping,
  construction, publication, retention and readback integration.
- `gma500_fixed_backend.[ch]`: USE/bootstrap/XHW/TA/raster/status backend.
- `gma500_fixed_entry.[ch]`: one-attempt latch and dormant fixed entry.
- `gma500_fixed_uapi.h`: fixed request/result ABI; no user addresses/commands.
- `kernel/sgx535_frozen/patches/antix-fixed-{makefile,irq,ioctl}.patch`:
  object inclusion, selected IRQ capture and root-only fixed ioctl.
- `tools/psb-dri-re/frozen_kernel_contract.*`, `frozen_kernel_initial.inc`,
  `frozen_kernel_relocations.inc`: canonical image/relocation contract.
- `frozen_fixed_service.*`, `frozen_fixed_io.*`, `frozen_scene_owner.*`,
  `frozen_va_pool.*`: bounded service/actions/ownership validation and tests.
- `frozen_triangle_image.py`, `frozen_triangle_dry_run.py`: static proof
  refusal and deterministic host model.
- `frozen_module_versions.py` and `test_frozen_module_versions.py`: ELF32
  import/table checks, mismatch/missing-symbol refusal and capture regressions.
- Client metadata: [static i386 artifact record](artifacts/frozen-triangle-one-shot-i386.md).

The fixed request is `{1,1,0,0}`, sequence 1, at most one TA and one raster
fire, 5×HZ deadline and 300,000 status samples; no automatic retry. Possible
fire followed by uncertainty retains BO/USE/scene/module ownership in HOLD.
A function/ioctl return is not completion. DPM_TA_MEM_FREE is a handled
scene-memory event, **not** triangle completion or an automatically fatal
fault. Existing regression preserves that correction.

## 8. Historical research-turn verification (newer results above)

**PROVEN BY REPOSITORY EVIDENCE / TEST OUTPUT:**

- Full Python discovery: **193 tests PASS**, 14.136 seconds, exit 0.
- Target-table check against original: PASS 222/222, zero mismatches.
- Table check against rejected candidate: REJECT, 154 mismatches; no missing
  imports. This expected rejection preserves the guard.
- Three strict C builds and UBSan executions: fixed service, fixed IO and
  kernel contract; all exit 0, no diagnostics.
- Initial-image generator `--check`: PASS.
- Two dry runs: both SHA-256
  `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
- `frozen_triangle_image.py --complete`: exit 1,
  `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.
- Documentation link/whitespace checks and `git diff --check`: verified before
  completing the turn; no implementation/test changes or candidate build.

`--complete` is a conservative static/architectural proof check, not a live
Gate B/deployment evaluator. It does not consume operator risk acceptances
or check module ABI, transition safety or authorization. Do not suppress its
four labels or mistake them for the exhaustive current engineering blockers.

## 8a. Subsequent local toolchain qualification

**CONFIRMED BY LOCAL EVIDENCE:** ARM64-hosted GCC 14.2.0-19cross1, GNU
binutils 2.44-3 and 23 supporting packages were authenticated, hash-verified
and extracted into `/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root`.
No global package installation/upgrade occurred. All selected tool executables
are ARM64; target objects are ELF32 little-endian i386; HOSTCC/helper/fixdep
are ELF64 AArch64. Cross libc link and native parser/scanner/SSL probes pass.
Exact commands, versions/hashes, native dependencies and initial smoke-command
corrections are in the [qualification report](i386-gcc-toolchain-qualification.md)
and its [evidence index](artifacts/i386-gcc-toolchain-20260930/README.md).

Use the recorded local environment: explicit target GCC 14 with
`--sysroot=<extracted root>`; i686 binutils prefix; native `/usr/bin/gcc-14`
HOSTCC. Do not omit the sysroot or give target ARM64 library/include paths.
Current kernel inputs remain unchanged. At this qualification checkpoint,
preparation/config/header comparison and new module import verification had
not yet been performed; the subsequent preparation result is below.
Fresh scoped suite: 193 PASS in 11.443s; three UBSan harnesses PASS; dry hash
unchanged; generator PASS; `--complete` refusal unchanged. No module built.
Gate B BLOCKED, whitelist `[]`, separate hot-transition blocker unchanged.

## 8b. Subsequent isolated Kbuild preparation: HARD STOP

The [preparation report](antix-kbuild-preparation-equivalence.md) and
[raw evidence](artifacts/antix-kbuild-preparation-20260930/README.md) establish:
exact antiX archive + package diff reconstructed in a fresh source; source
Makefile matches capture 08; old package table removed in disposable source;
qualified target GCC/binutils and native HOSTCC used by Kbuild. olddefconfig
and resumed modules_prepare exited 0, but header equivalence **FAILED**.

8788 config values match except CONFIG_CC_VERSION_TEXT; auto.conf/autoconf.h
have the same banner-only difference. 32 of 35 captured generated headers
match exactly. compile.h is phase-specific full-kernel identity metadata not
produced by modules_prepare. The hard stop is syscalls_32.h: prepared 11385
bytes/429 lines through syscall 239 versus target 22421/828 through 440.
Prepared bytes are an exact target prefix; resumed Kbuild skipped generation.
Fresh diagnostic generation in a separate file reproduces the target exactly.
Interrupted non-atomic generation is INFERRED; exact interruption cause UNKNOWN.

Failed output remains `/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/output`.
No repair, target-table installation or candidate compilation followed the
hard stop. New fresh verification: 193 PASS (19.678s), three UBSan harnesses
PASS, reference CRCs 222/222, generator PASS, same two dry hashes and --complete
refusal. No implementation/test guard changed. Gate B BLOCKED, whitelist `[]`.

## 8c. Fresh successor preparation/equivalence: PASS

The [successor report](antix-kbuild-successor-equivalence.md) and
[new evidence](artifacts/antix-kbuild-successor-20260930/README.md) supersede
the interrupted-header blocker without modifying its failed output.
Successor output is `/home/gama/sgx535-offline/antix-kbuild-successor-20260930/output`,
using the same verified exact antiX source and qualified toolchain. Normal
olddefconfig/modules_prepare exited 0, empty stderr. Kbuild generated the
previously absent syscall header once: 22421 bytes/828 lines, target-exact SHA
`c4fcafae0822cf103d2822c0d842c6f8bb8f24b00bbb0633a3150ef36033d9fd`.

Full independent comparison: 33/35 captured generated headers exact; only
documented compiler-banner, dependency-path metadata and phase-specific
compile.h absence remain. All 8788 config values equal except banner; no
unexplained architecture/feature/mitigation/layout/header drift. Native helpers
are AArch64, target preparation object is ELF32/i386. Failed-output before/after
snapshots match for all 10569 entries. Prior interruption cause remains UNKNOWN.

Fresh 193 tests PASS (20.653s), three UBSan harnesses PASS, generator PASS,
reference CRCs 222/222, two identical expected dry hashes and --complete refusal.
No candidate compiled or target table installed into successor output. Build
and complete new-import/artifact qualification remain a separately scoped step.
No target contact or implementation change. Gate B BLOCKED, whitelist `[]`.

## 9. Public research and proposed offline strategy

**PROVEN BY PUBLIC DOCUMENTATION:** Debian provides ARM64-hosted
`gcc-14-i686-linux-gnu` 14.2.0-19cross1 and i686 binutils 2.44-3. Linux 5.10
Kbuild separates native HOSTCC from target CC/binutils and selects 32-bit x86
through config/architecture flags. See the dated primary-source register and
claim-specific links in the [build plan](i386-gcc-offline-build-plan.md).

**REASONABLE INFERENCE:** the minimum suitable route is a pinned,
dependency-complete local Debian ARM64 cross-toolchain, native ARM64 Kbuild
helpers, fresh exact antiX source, captured config, prepared generated
headers, and the qualified full target table. No remote machine or Mini 12
build host is needed. An AArch64-native GCC with `-m32` or ARM multilib is not
the route. Cross libc is for configuration link probes, not module linkage.

The plan specifies tool identity/ELF32 smoke checks, fresh isolated source and
output directories, `ARCH=x86`, GNU i686 prefix, explicit GCC 14 target CC,
native HOSTCC, `LOCALVERSION=-486-smp`, config/layout comparisons and normal
external-module `M=...` build mode. External mode avoids the old in-tree
`vmlinux.symvers` input. Preserve target config except a separately recorded
cross-compiler banner difference; any other drift stops for review. Do not
invent compiler metadata, change CRCs or use Clang merely because it linked
before. Toolchain provisioning/smoke checks and the successor preparation gate
have now passed as recorded above; the candidate build has **not** been executed.

Research established the route; subsequent qualification established the
observed local toolchain/smoke obligations, **not a qualified module artifact**.
Candidate bit-reproducibility, final import set/CRCs and runtime conformance
remain to be checked. The documented preparation/header equivalence gate passes;
this does not qualify an as-yet-unbuilt module.

## 10. Mandatory checks for a rebuilt candidate

1. Immutable input/source/patch/toolchain manifest; genuine config/generated
   output comparisons; no architecture, feature, mitigation or layout drift.
2. Kbuild trace proves native host helpers, GCC/i386 target compilation,
   GNU x86 linker and captured table consumption; no ignored errors/warnings.
3. ELF32 little-endian i386; exact vermagic; complete build; module_layout
   b84efb99; every imported CRC covered by the captured table and equal.
4. Existing `--check-symvers NEW_MODULE TARGET_TABLE` checks every import in
   its first argument. Its JSON calls that argument the reference; preserve
   scope/provenance and artifact hash rather than relabeling report flags.
   The two-module original/candidate check alone cannot verify new symbols.
5. Review new imports/dependencies, signature policy, source/UAPI and entry
   identity, final binary hash/build ID; compare repeat-build results.
6. Full existing host tests, UBSan, generation check, dry-run determinism and
   unchanged fail-closed static proof refusal.

Even every offline check passing does not authorize deployment or establish
safe hot replacement, successful GPU execution, architectural PDS semantics
or a rendered triangle. A new artifact requires a new exact-action review.

## 11. Independent transition/recovery blocker

The new [lifecycle review](candidate-01-build-and-lifecycle-review.md) proves a
missing IRQ release in the original source/imports and implements a scoped
correction in a separate ABI-qualified derivative. Actual-source tests cover
release ordering, failure unwind and routing disable, including preservation
of the direct suspend callback. The retained recovery oops remains consistent
with stale action/name lifetime; exact crash causation is INFERRED, not proven.

**The active original still has the old unload implementation.** A replacement
cannot fix the preceding removal of that original. Do not replay the failed
hot-transition/recovery procedure. A cold-start-only first-load route may avoid
this boundary, but its bootloader/initramfs module source and original fallback
are not established by retained evidence. No such deployment is authorized.

## 12. Architectural UNKNOWNs, decisions and authorization

**UNKNOWN:** primary PDS pre-definition source bound L12, auxiliary source
bound FT-AUX, full architectural publication rule and GPU-to-CPU readback
visibility. Deterministic backing/programs do not close these facts.
Phase 7 exhausted its public evidence routes and is complete with an explicit
boundary. Historical operational ready predicates are not complete
architectural definitions. Runtime BO mappings, readiness, completion and
color output remain unobserved for the fixed service.

Prior operator risk acceptances for publication, driver replacement, selected
ISP reset, L12 and FT-AUX are recorded decisions for the narrow experiment,
not architectural proof or permission for retries. Attempt 03's one-attempt
execution authorization was spent. No risk decision turns the rejected module
into a compatible artifact or qualifies the failed hot restoration.
**Current Gate B BLOCKED; current whitelist `[]`. No new SGX attempt,
deployment, target contact, service/module/VT/PCI change, DRM open, MMIO,
ioctl, reset or reboot is authorized by this handoff or the research turn.**

## 13. Do not repeat

Do not redo Phase 7 PDS/0x07/source-envelope archaeology, missing-PDF recovery,
Vita/SGX543, EMULATOR/RTSIM, corpus/differential/nine-DWORD work, backing-provider
or linked-relocation reconstruction, CORE_ID/CORE_REVISION identification,
rev121 CPU-branch rejection, fixed ten-object/49-relocation scene, static i386
client build, old module mismatch diagnosis or completed post-reset capture.
Do not repeat Attempt 03 or failed recovery. Preserve unrelated working-tree
changes; no staging, commit or push. If an artifact/path is missing in a new
environment, check its recorded manifest instead of assuming the project
never implemented it. Temporary /tmp build trees may disappear.

## 14. Critical path recorded at the build/lifecycle checkpoint

1. **READ-ONLY TARGET OBSERVATION, separately authorized:** capture the normal
   boot module-loading source and original fallback, as narrowly specified in
   the [current report](candidate-01-build-and-lifecycle-review.md). Do not
   execute this observation automatically.
2. Review a concrete first-load deployment/recovery route that avoids removing
   the active original with its unpaired IRQ registration.
3. Review the distinct lifecycle-corrected artifact and exact action; obtain
   new operator authorization before any fresh guards/deployment/one-shot test.

**SINGLE SMALLEST NEXT STEP: READ-ONLY TARGET OBSERVATION — boot-route inventory.**
ABI qualification is complete offline; it is not a runtime or authorization PASS.

## COPY/PASTE CHECKPOINT FOR CHATGPT

This block preserves the earlier build/lifecycle checkpoint. Read the current
cycle result above before using it; its boot-route inventory was a next step at
that time.

```text
Repo /home/gama/sgx535-gfx; read candidate-01-build-and-lifecycle-review.md,
CHATGPT-HANDOFF-POST-ATTEMPT03.md and new artifact README first.
Target Mini12/Poulsbo/SGX535 rev121; retained IDs 01130000 / 00010201.
antiX 5.10.240-antix.1-486-smp, i686. No target contact/SGX action this pass.
Toolchain and successor config/header/layout preparation PASS; do not redo.
Integration Makefile EOF and ioctl asymmetric context fixed, same intended
semantics; fixed IRQ patch unchanged. All strict applications no fuzz/offsets/rejects.
i386 __umoddi3 dependency removed via guarded power2 alignment mask; regression PASS.
Candidate1 SHA13591674ef9fa82f185f075185d1fa09d94606ccce7253ed231627f2649c600f
BuildIDd72f19c97c5fd1feffddad07634e12333562ae60, 242364bytes,230/230imports PASS.
Lifecycle derivative SHA91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74
BuildID074c650d48ccb463e37e90428b486dccdb23eb80,242724bytes,232/232imports PASS.
Both ELF32LE/i386, exact vermagic, module_layout b84efb99,0missing/0mismatch,
ABI QUALIFICATION PASS offline, clean repeat BIT IDENTICAL, unsigned.
Target table faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca
consumed by normal external modpost; all7426 config/generated hashes unchanged.
Source e5c5d7c6bdcc7a845589e5956cbf50a8d6198232df017f0398a2902371042e1d,
captured config93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9.
GCC14.2i686/GNUbinutils2.44, nativeHOSTCC/usr/bin/gcc-14; environment logged.
Apply patches in order Makefile -> fixed IRQ -> lifecycle -> ioctl.
Lifecycle derivative quiesces poll/vblank, calls drm_irq_uninstall before private
teardown, checks install/late-init errors. Actual core removal callback masks all
routing; direct PM retains old mask. Actual-source UBSan7tests/10scenarios PASS.
205tests PASS zero skips,3other UBSan PASS,generator PASS,dryrun both
2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e.
--complete remains exit1 PARTIAL L12,FT-AUX,FT-BO,FT-SERVICE; not a GateB checker.
Old candidate934bd971... remains REJECTED(layout995e9910,154mismatches).
Attempts/recovery unchanged; no SGX submission occurred. Capture08 verified
original/display recovery after operator reset. Runtime signature policy UNKNOWN.
FIRST HOT TRANSITION BLOCKED: currently active original still lacks IRQ release;
new derivative cannot fix its preceding removal. Oops stale-IRQ causation INFERRED.
GateB BLOCKED,whitelist[],prior attempt auth spent; no deployment/SGX authorized.
Single next step READ-ONLY TARGET OBSERVATION, separately approved: identify
normal boot module source/initramfs and original fallback to review cold-start-only
first loading without original hot teardown. Not executed this pass.
Do not redo Phase7, IDs, scene,49relocations,client,capture08,Attempt03,recovery,
toolchain/preparation. Preserve old evidence and both new artifacts. No force,
manual CRCs, target experiments, staging, commit or push.
```
