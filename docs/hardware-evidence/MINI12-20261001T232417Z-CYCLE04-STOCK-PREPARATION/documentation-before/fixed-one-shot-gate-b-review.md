# Fixed 32×32 triangle: exact-action Gate B review (updated 2026-09-30)

## Latest result: first-load cycle03 (2026-10-01)

[Cycle03](first-load-cycle-03-result.md) passed a fresh 53-guard STOCK preflight
and existing staged-file receipt checks. The operator reported experimental
userspace/display at approximately 120 seconds. No experimental connection was
made because no remaining capture budget was established. The menu photo was
not captured; the operator's correction is preserved. Experimental boot identity,
loaded note, hook trace and ownership evidence are missing.

Stock recovery capture did not run because readiness/deadline facts were not
established. The operator confirmed current STOCK userspace and a normal physical
display; that is an operator observation, not fresh module/ownership/kernel-health
proof. LIVE FIRST LOAD, FIRST OWNER and full LIVE RECOVERY remain NOT ESTABLISHED.
The cycle is spent. No restaging, hot transition, fixed ioctl or SGX fire occurred.

Gate B remains BLOCKED; SGX whitelist `[]`; FIRST TRIANGLE NOT ATTEMPTED.
307 repository tests, 14 boot-analysis tests, three UBSan harnesses and existing
CRC/generator/dry guards passed. The pinned image/candidates remain unchanged.
The [timing redesign](first-load-capture-timing-redesign.md) is a draft for review;
current 120-second guards are unchanged. The next step is OFFLINE design review,
then separately scoped implementation/qualification and live authorization.
Earlier current-state and next-step notices below are historical.


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

**Current checkpoint audit:** [Post-Attempt-03 audit](post-attempt03-checkpoint-audit.md)
supersedes the execution classifications below. **Gate B: BLOCKED. Active
whitelist: `[]`.** The rejected candidate is ABI-incompatible, the original
driver's hot restoration faulted, and no new attempt is authorized. The
operator reset the Mini 12; [capture 08](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/RESULT.md)
now independently confirms the normal original driver/display state and
qualified installed export table/configuration. Compatible candidates now exist offline; no executable original-driver
hot-transition qualification exists. Previous
one-attempt risk acceptances remain recorded as decisions, not architectural
proof or an unspent authorization. The PASS and whitelist below are historical.

**Offline Attempt 03 follow-up:** [Symbol-version comparison](attempt03-module-version-followup.md)
shows the pinned candidate's `module_layout` CRC differs from the preserved
installed module's and 149 other shared import CRCs also differ. The
historical PASS below describes Attempt 03's reviewed procedure; it no
longer qualifies the rejected artifact for a new attempt. The authorization
is spent. At that checkpoint the target remained in HOLD; recovery or a new
artifact needed separate review and authorization.

**Attempt 03 snapshot:** [Attempt 03](../hardware-evidence/MINI12-20260930-TRIANGLE-ATTEMPT-03/RESULT.md)
ended in HOLD before the ioctl. The original module detached after the tested
console release, but the pinned candidate was rejected for a
`module_layout` symbol-version mismatch. At capture the target had no gma500 module or
DRM card. The one-attempt authorization is spent; this historical PASS does
not authorize recovery or another attempt.

**Attempt 02 historical snapshot:** [Attempt 02](../hardware-evidence/MINI12-20260930-TRIANGLE-ATTEMPT-02/RESULT.md)
reached the mandatory module-removal preview and stopped when the original
`gma500_gfx` remained in use after the display service released its file
descriptors. The service-only reversal restored the original passive state.
The PASS below records the pre-attempt risk review; **further execution was
BLOCKED at that checkpoint** by the newly observed stage-2 prerequisite. The one-attempt
authorization has ended. The whitelist identifier remains a historical
record and is not an active retry authorization.

**Historical decision: PASS for one exact reviewed action, with mandatory live stop
guards. Whitelist: `[MINI12-SGX535-REV121-FROZEN-32x32-SEQ1]`.** This review
concerns one fixed scene, one TA/raster sequence, and no retry. It does
**not** authorize deployment or submission in the transition-inventory turn;
a new explicit operator authorization is required before any target write.
The previous operator acceptance of CPU→SGX publication applies only to this scene and is
not architectural proof.

## Operator decisions recorded 2026-09-30

The operator explicitly **ACCEPTED** all four remaining decisions for **one
frozen first-triangle attempt only**: active-driver replacement, the selected
historical ISP soft-reset bracket, primary PDS source uncertainty (L12), and
auxiliary source uncertainty (FT-AUX). These are risk acceptances, not new
architectural or target observations. They do not establish safe driver
replacement, safe reset/recovery, PDS/auxiliary instruction semantics, source
containment, or a GPU-only failure boundary. No retry or other experiment is
authorized. The earlier publication acceptance retains the same narrow scope.

Before any active SGX action, compare a read-only live target preflight with
the retained Mini 12 baseline. Any mismatch, ambiguity, or missing
prerequisite stops the attempt. Even a passing preflight does not itself
authorize a module replacement or fire: present the complete result and exact
proposed deployment/one-shot operation before the first active action. Do not
force-load. The one-shot constraints remain sequence 1, at most one TA and one
raster fire, `5×HZ` deadline, 300,000 status-sample cap, HOLD on unexpected or
contradictory status and after any ambiguous possible fire, retained BO/USE/
scene state, and no automatic retry or speculative recovery.

## Reviewable offline action

The source-only integration patch adds root-only private ioctl index zero to
the exact antiX gma500 tree. Its fixed request begins with the already tested
16-byte `{1,1,0,0}` contract. No user pointer, BO handle, address, register,
relocation, shader, or command list enters the kernel. The output returns an
operation errno, phase, event mask, 32 row counts, hash, and the complete
4,096-byte color BO if readback succeeds. The client refuses to open DRM
without its exact one-shot flag. The kernel checks PCI `8086:8108`, CORE_ID
`0x01130000`, and CORE_REVISION `0x00010201` before constructing the scene.
The one-attempt latch prevents another attempt within that module instance.
The selected kernel call uses sequence `1`, a `5×HZ` deadline, at most
300,000 status samples, one TA fire and one raster fire for that same scene.
An error after possible fire preserves the BOs, USE reservation and module
reference. It never retries or resets the GPU.

The target action, if separately authorized, has three ordered parts: (1) a
read-only recheck of live kernel release, loaded gma500 identity, PCI binding,
display ownership, runit service state, and root-visible graphics FD holders
against the retained captures; (2) the
[reviewed active display/module transition](active-gma500-transition-review.md),
with no force and the original installed module preserved; (3) one invocation
of the fixed ioctl on the verified Poulsbo DRM node. A mismatch or failure in
an earlier part stops before the ioctl. The active-display replacement risk
was explicitly accepted for this one experiment; its runtime success remains
unobserved. The first passive preflight passed on 2026-09-30 with 28/28
checks matching the retained H0 baseline; its raw capture and comparison are
in [MINI12-20260930-TRIANGLE-PREFLIGHT](../hardware-evidence/MINI12-20260930-TRIANGLE-PREFLIGHT/README.md).
The separate root read-only
[transition inventory](../hardware-evidence/MINI12-20260930-TRANSITION-INVENTORY/RESULT.md)
established runit supervision of `slimski`, only Xorg holding a graphics
device FD at capture, and a successful privileged read-only route. Both
captures are snapshots; dynamic conditions must be checked again just before
the active sequence. No active part has occurred.

The [static i386 one-shot client](artifacts/frozen-triangle-one-shot-i386.md)
is built and verified offline against the same UAPI header as the candidate
module. The [active transition review](active-gma500-transition-review.md)
is **PASS as a bounded exact-action procedure** after the successful root
read-only inventory. That inventory followed an initial passwordless sudo
refusal; the successful credential-backed run used the same read-only script.
Installing the candidate module is itself an active SGX/display operation.
No target file was transferred and no active transition occurred.

The module built offline as ELF32 i386 from the exact-version public antiX
source, with retained symbol versions. Its vermagic matches the prior target
observation: `5.10.240-antix.1-486-smp SMP mod_unload modversions 486`.
Build SHA-256: `934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`.
This establishes source/build compatibility only. The built module has not
been installed, loaded, or compared with the target's current live state.
The build copies `kernel/sgx535_frozen/gma500_*.[ch]` and the fixed C contract
files from `tools/psb-dri-re/` into the extracted gma500 source directory,
then applies the three scoped patches in `kernel/sgx535_frozen/patches/`:
Makefile object inclusion, IRQ capture before normal clear, and the fixed
private ioctl. Its temporary build config sets local version `-486-smp`; the
original package symbol-version table is used for modpost. No source package
or installed target module was modified.

Attempt 03 subsequently proved that this package table does **not** match the
installed target module's import CRCs. Any future candidate must first pass
the fail-closed offline `tools/psb-dri-re/frozen_module_versions.py` check
against the preserved target module and account for every candidate-only
import with a target-qualified symbol table. Matching vermagic alone must
not be used as the module-compatibility gate.

## Historical Attempt 03 exact-action classifications (superseded above)

| Item | Classification | Basis and remaining condition |
| --- | --- | --- |
| Identity and fixed input | PASS | Fixed PCI/core guards and 16-byte request; wrong identity rejects before scene construction. |
| Ten BOs, SGX VA, 49 relocations, USE 3/4 | PASS for the bounded first attempt | Kernel owns every value; VA, mapping and reservation failures reject or hold. Live acceptance remains the experiment's runtime observation, not prior proof. |
| CPU→SGX publication | ACCEPTED ASSUMPTION | Operator accepted only this exact frozen bring-up. Cache/coherency rule remains unproved. |
| rev121 bootstrap predicate | PASS for the bounded first attempt | Historical selected init and TA-load plans use bounded polls; failure holds after device maintenance. This is a driver predicate, not complete architectural readiness proof. |
| TA/raster event mapping | PASS for the bounded first attempt | Current Poulsbo header names TA finished bit 13, PBE end-render bit 18, DPM 3D memory-free bit 0, and BIF requester fault in STATUS2 bit 4. Candidate scheduler requires TA completion, then both raster events. DPM TA memory-free bit 24 is a separate nonterminal event; a duplicate holds. Fixed capture precedes current IRQ clear; polling shares its lock. Unknown or contradictory new SGX status holds. Normal 2D bit 27 and aggregate bit 31 are excluded from scene attribution. Live event delivery remains to be observed. |
| ISP reset assert/clear | ACCEPTED ASSUMPTION | Historical `psb_schedule_raster` brackets raster programming with `PSB_CR_SOFT_RESET` bit 5. Current header names bit 5 ISP and bit 1 2D. Failure at either bracket stage holds. Active-display impact and interrupted-assert recovery are not target qualified. |
| One attempt and bounded failure | PASS for software containment; ACCEPTED ASSUMPTION for platform consequence | Timer, fault/unknown-event HOLD, retained BO/USE/module ownership, no retry and no automatic reset are implemented. A GPU/display/system hang may require operator power-cycle. |
| Primary L12 source bound | ACCEPTED ASSUMPTION | Selected opaque PDS source domain remains unbounded by available qualified evidence. No new source-selection evidence arose here. |
| FT-AUX source bound | ACCEPTED ASSUMPTION | Secondary/auxiliary CPU images are deterministic, but their pre-definition eligible sources remain unbounded. |
| Target module replacement and active display | ACCEPTED ASSUMPTION | Prior passive evidence has gma500drmfb driving the display. Replacing the loaded module, even with matching vermagic, may blank or hang that display. No target module was replaced in this pass. Loading must never force vermagic/symbol mismatches. |
| Target i386 client | PASS offline | Static ELF32 Intel 80386 artifact SHA-256 `758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf`; matching UAPI, no interpreter, and offline wrong-flag refusal. No target execution. |
| Active transition procedure | PASS for the exact reviewed plan; live stop guards required | Root inventory identified runit/`slimski`, Xorg as sole root-visible graphics FD holder, and a working privileged route. The plan uses runit stop, verifies no holders, requires a removal preview naming only `gma500_gfx`, then removes the original without force, verifies detach, and loads only the pinned candidate. Any deviation stops or holds before ioctl. A physically present operator and independent log capture remain mandatory pre-action checks, not claims of observed state. |

## What the failure injection established

The host one-shot harness fails every fixed callback in turn, including ISP
assert and clear, TA/raster possible-fire, and retirement. Before device
maintenance it permits bounded cleanup. From maintenance or possible fire it
holds ownership. Timeout, duplicate/wrong-sequence completion, selected fault,
unknown status, and a second submission reject or hold. These tests do not
prove live GPU completion or recovery. `--complete` correctly remains
`PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE` because that historical completeness
gate still requires source proof and live target postconditions; the scoped
operator acceptances do not change its architectural classifications.
The scoped Python suite passes 175 tests, the one-shot C harness passes with
UBSan, and two unchanged dry runs hash to
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
The 2026-09-30 offline service harness also exercises a separate
`DPM_TA_MEM_FREE` status before TA finish: it remains nonterminal, then
allows attributed TA and raster completion. Repeating that status holds the
scene. A deliberately mutated contract that treated the event as unknown
failed this regression. This strengthens the offline test of the already
recorded event classification; it is not live IRQ evidence and does not
change the Gate B risk acceptances.

## Recorded decision and next boundary

Publication and the four decisions above are accepted for this exact
experiment. The 28/28 passive target preflight and the separate root
read-only transition inventory passed. The active transition procedure is
reviewed with mandatory live stop/HOLD guards. **Gate B is PASS for the
single whitelist entry below as a readiness decision, not as permission to
execute it in this turn.** The acceptances do not prove SGX architecture,
target cache semantics, safe reset recovery, or success. The next active
target action requires a fresh explicit operator authorization.

### Narrow whitelist entry

`MINI12-SGX535-REV121-FROZEN-32x32-SEQ1` permits only a separately
authorized operator-controlled transition and one fixed submission on the
Dell Inspiron Mini 12 with kernel `5.10.240-antix.1-486-smp`, PCI
`0000:00:02.0` / `8086:8108`, retained CORE_ID `0x01130000` and
CORE_REVISION `0x00010201`. It pins the unchanged original installed module
SHA-256 `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb`,
candidate ELF32 module SHA-256
`934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`,
and static i386 client SHA-256
`758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf`.
It requires a fresh exact passive-state match; physical operator presence;
independent log capture; root-only artifact staging and rehash; one runit
`slimski` stop; zero unexpected graphics FD holders; a `modprobe -n -v -r`
preview naming only `gma500_gfx` and no configured removal helper; one
nonforced original module removal; clean detach; one nonforced candidate
load; exact loaded
Build ID `d4cb8d750efa6eb401f82e6eaec7eb82ebdf9c3c`, PCI/DRM binding;
then exactly one client invocation with flag `--one-shot-sgx535-rev121`,
ioctl `0xd0ac6440`, request `{1,1,0,0}`, sequence 1, at most one TA and one
raster fire, `5×HZ` deadline and 300,000 status-sample cap. No retry,
additional workload, force operation, speculative reset, or power/clock
change is within the entry. Stop on any pre-fire mismatch; HOLD on partial
driver state, any possible fire with an error, fault, timeout, unexpected
status, or ambiguous completion. Preserve BO/USE/scene and captured evidence.
The entry is **not a general SGX authorization** and does not authorize an
active step until the operator separately approves the exact transition.

### Primary L12 decision record

- **Exact uncertainty:** whether `0x07000345 / 0xaf000000` can consume a
  pre-definition DS, temporary, or implicit source outside the deterministic
  controlled image and known launch state.
- **Existing evidence:** exact CPU literals and 12 selected data DWORDs are
  deterministic; producer correlations are not operand proof. Phase 7's
  architectural, corpus, and observation routes ended without a complete
  source bound.
- **Why offline proof is unavailable:** retained public sources neither define
  the selected operand set nor provide a target-equivalent read trace.
- **If wrong:** SGX may fetch an unintended USE address/state, fault, hang, or
  write unexpected GPU-visible memory. The clean-room zeros do not constrain
  an unclassified source outside the backing.
- **Failure boundary:** not proven GPU-only; gma500 also drives the active
  display, and a GPU/MMU hang can affect display or system progress.
- **Acceptance would authorize:** one frozen scene attempt under the other
  Gate B conditions. It would not prove operand semantics or general safety.

### Auxiliary FT-AUX decision record

- **Exact uncertainty:** the pre-definition source envelope of the fixed
  secondary and auxiliary PDS/USE launch contexts.
- **Existing evidence:** their selected CPU byte images, relocations, and
  fixed addresses are reproducible; complete source selection is unproved.
- **Why offline proof is unavailable:** the same missing SGX535 source rule
  applies in distinct launch contexts, and retained execution observations
  do not isolate auxiliary source reads.
- **If wrong:** auxiliary work may consume inherited state, produce incorrect
  geometry/shader input, fault, hang, or write unintended GPU-visible memory.
- **Failure boundary:** not proven GPU-only; active display/system impact is
  plausible through a shared SGX/MMU or a stuck driver path.
- **Acceptance would authorize:** the same single frozen scene attempt, not
  arbitrary auxiliary programs and not proof of their semantics.

### ISP reset and platform decision record

- **Exact uncertainty:** effect on the live Mini 12 display/platform if the
  historical ISP bit-5 assert/clear is interrupted or raster later hangs.
- **Existing evidence:** `psb_schedule_raster` and current `psb_reg.h` agree
  on `PSB_CR_SOFT_RESET=0x80`, ISP bit `0x20`, assert before raster pairs,
  clear before fire. The separate 2D reset bit is `0x02`. There is no target
  observation of this reset under the active gma500drmfb desktop.
- **If wrong:** display corruption, stuck GPU, lost desktop/SSH or a system
  hang may require operator power-cycle. The service holds all ownership and
  never attempts a speculative reset.
- **Acceptance would authorize:** this one selected ISP bracket in the frozen
  raster path, not whole-GPU reset or any power/clock/PLL change.

### Driver replacement decision record

- **Exact uncertainty:** whether the active-display Mini 12 can leave its
  currently loaded gma500 module and enter the rebuilt fixed module without
  disrupting the display or stranding the machine.
- **Existing evidence:** the prior passive capture linked the loaded module
  to installed SHA-256 `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb`.
  The new offline build has matching target vermagic and retained symbol
  versions, but is not the installed/loaded binary. The new module is not
  signed or target-loaded by this review.
- **If wrong:** replacement may fail cleanly, blank the display, interrupt
  Xorg/fbcon, or leave the host inaccessible until operator recovery.
- **Acceptance would authorize:** an exact separate target preflight and a
  one-attempt deployment plan that rejects any mismatch and never forces a
  module load. It would not authorize arbitrary driver reloads or repeat GPU
  submissions. The deployment operation itself must be reviewed before use.
