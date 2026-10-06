# Post-Attempt-03 checkpoint audit — 2026-09-30

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

Current execution classification: **Gate B BLOCKED; active whitelist `[]`;
no Attempt 04 authorized or performed.** This audit used local repository and
preserved artifacts only. It did not contact the Mini 12, rebuild a candidate,
alter hardware, stage, commit, or push. Earlier Phase 8 PASS decisions and
whitelist entries are historical, spent exact-action reviews.

**Authoritative update, 19:10 UTC:** The approved
[capture 08](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/RESULT.md)
passed all state guards and root supervisor status. Normal original driver,
PCI/DRM/fb/VT and slimski/Xorg recovery is independently verified. Installed
image/header packages have identical exact version; captured Module.symvers
matches all 222 original imports and covers all nine additional imports.
`module_layout` is `0xb84efb99`. Target exported-CRC evidence is qualified;
the rejected candidate still mismatches 154 of its 228 imports. Config and
generated-header consistency passes. No candidate was rebuilt or deployed.
Native HOSTCC remains AArch64. The subsequent [local cross-toolchain
qualification](i386-gcc-toolchain-qualification.md) passes its offline identity
and smoke obligations. The subsequent [isolated preparation](antix-kbuild-preparation-equivalence.md)
ran but FAILED generated-header equivalence: truncated syscalls_32.h was
skipped by resumed Kbuild. A separate [fresh successor](antix-kbuild-successor-equivalence.md)
now PASSES the complete preparation/config/header/layout gate with natural
Kbuild syscall generation. The predecessor remains unchanged failed evidence.
No new candidate was compiled. Config differs only by the documented cross
compiler-name banner; all non-exact metadata is explicitly classified.
The next build must preserve captured configuration. Gate B BLOCKED, whitelist `[]`.

**Historical preceding candidate-build result (superseded by the current continuation above):** [candidate #1 integration guard](candidate-01-offline-build-qualification.md)
STOPPED before compilation. The retained Makefile patch requires a blank line
after the exact source's EOF; its strict dry run fails. The ioctl patch also
fails strict application; IRQ dry run passes. No source/patch was changed,
no table installed and no candidate built. Existing qualification/preparation
gates remain PASS; all 193 tests and three UBSan harnesses pass again. The
smallest next step is OFFLINE integration-patch correction/review with strict
no-fuzz dry-run success, before any candidate compilation.
The connection/authentication chronology below is historical, superseded by
this successful capture. Full suite at that preceding checkpoint: **193 tests PASS**.

**Research/documentation continuation:** The
[offline GCC build plan](i386-gcc-offline-build-plan.md) now establishes a
publicly documented Debian ARM64-hosted i686 GCC 14.2/binutils 2.44 route.
The toolchain was not yet provisioned at that research checkpoint. Subsequent
[local qualification](i386-gcc-toolchain-qualification.md) now passes; no
replacement candidate was built. The [self-contained ChatGPT handoff](CHATGPT-HANDOFF-POST-ATTEMPT03.md)
consolidates the current checkpoint, immutable artifact hashes, evidence
classes, build checks and separate hot-transition blocker. Gate B/whitelist
remain BLOCKED/`[]`; this documentation turn made no target contact.

**Subsequent authorized read-only observation:** The
[post-reset/build-input capture](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-01/RESULT.md)
made one pinned SSH connection at 18:00 UTC. It timed out before authentication
or remote execution. No target state/build artifact was observed, no retry
occurred, and all ABI/transition/post-reset classifications below remain
unchanged. The operator report is still the latest post-reset information.

**Renewed observation after the operator reported power-on:**
[Capture 02](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-02/RESULT.md)
used the identical pinned script at 18:35 UTC. SSH returned `No route to host`
before authentication or remote execution. No target evidence was obtained;
no retry or endpoint scan occurred. Current network address/connection needs
operator confirmation before the same observation can proceed. Gate B and
the blocker classifications below are unchanged.

**Latest connection result:** After the operator supplied link-local address
`169.254.77.254`, local-only route/interface checks showed this host on
`192.168.18.84/24` with no displayed link-local route. The operator then
requested "try now". [Capture 03](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-03/RESULT.md)
made one explicitly announced fresh connection to the original pinned
`192.168.18.90:22` endpoint at 18:43 UTC; it again returned `No route to host`
before remote execution. No other address was tried. Current reachable target
network address must be confirmed; no target build/state evidence exists yet.

**Successful post-reset identity/display observation, 18:50–19:05 UTC:**
[Capture 04](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-04/RESULT.md)
reached the pinned host. Its two false identity guards were corrected from
retained exact DMI/P-O-E baseline evidence and covered by four fixture tests;
the full scoped suite now passes **191 tests**.
[Capture 05](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-05/RESULT.md)
verified original module hash/build ID/Live state, PCI/DRM/fb/VT ownership.
[Capture 06](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-06/RESULT.md)
verified runit→runsv→slimski→Xorg and the original service path; unprivileged
`sv status` is permission-denied by the root-only supervisor directory.
[Capture 07](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-07/RESULT.md)
then stopped before root execution because noninteractive sudo had no cached
authentication. No build artifacts were read. Detailed public display/driver
recovery is now independently confirmed; root supervisor status and installed
kernel build inputs still need the authorized read-only capture. No target
state-changing operation occurred. Gate B/whitelist remain BLOCKED/`[]`.

## Evidence boundaries

- **Proven offline:** fixed CPU images, ten-BO contract, 49 canonical
  relocations, synthetic Python/C equivalence, rev121 CPU branch admission,
  kernel owner/backend/entry source, one-shot ledger and failure injection,
  frozen i386 client identity, and the rejected module's CRC incompatibility.
  These do not prove live mapping, device readiness, completion, or rendering.
- **Retained target observations:** [Attempt 03](../hardware-evidence/MINI12-20260930-TRIANGLE-ATTEMPT-03/RESULT.md)
  removed the original normally, rejected the candidate at module load, and
  never invoked the client or fired TA/raster. The subsequent
  [original-driver recovery](../hardware-evidence/MINI12-20260930-ATTEMPT03-ORIGINAL-RECOVERY-01/RESULT.md)
  faulted during IRQ registration in `psb_pci_probe`, leaving a partial
  `Loading` module with no usable display stack. These snapshots are history.
- **Operator report and subsequent verification:** the operator reset the
  Mini 12 and reports normal display operation returned. Captures 05/06 now
  independently establish the expected original driver/module identity,
  bindings, framebuffer, VT ownership and runit/display process chain. The
  old partial/unbound state is no longer current. Capture 08 additionally
  establishes root supervisor status and installed build-input provenance.
- **Still requiring verification:** a revised deployment route that avoids
  the active original's unpaired IRQ teardown and provides a reviewed fallback;
  and eventual experiment runtime postconditions. Candidate ABI qualification
  is now complete offline. No deployment or additional target action is authorized.

## Current blocker classification

Statuses classify each named obligation, not a generic driver. A live
postcondition can be established by a separately reviewed first experiment;
it need not require a prior successful triangle.

| Item | Status | Evidence | What closes it |
| --- | --- | --- | --- |
| Frozen i386 client | CLOSED | Static ELF32 i386, no interpreter/dynamic segment; binary/source/UAPI hashes match the reviewed record. | Preserve these inputs; any change requires artifact review. |
| CPU scene, BO/VA/USE contract, relocations and fixed one-shot software | CLOSED | Current kernel sources match the retained candidate build copies; host tests cover golden bytes, ownership, rollback, bounds and one-shot containment. | Offline obligation is satisfied; runtime properties are separate below. |
| rev121 CPU branch rejection | CLOSED | Qualified raw revision `0x00010201` admitted by existing regression-tested CPU model. | No further branch archaeology. Device-ready predicate observation remains separate. |
| L12 primary architectural source bound | UNKNOWN | Phase 7 evidence boundary remains; prior operator acceptance is a scoped risk decision. | Qualified new source-selection evidence would prove it; a new exact-action review may use explicit risk acceptance without claiming proof. |
| FT-AUX architectural source bound | UNKNOWN | Auxiliary CPU images exist; pre-definition source domains remain unproved and previously accepted only for the bounded experiment. | Same distinction between new proof and scoped risk acceptance; no exhausted search is reopened. |
| Publication architectural rule | UNKNOWN | Historical sequence implemented; operator accepted its uncertainty for the frozen bring-up. | New evidence would establish the rule. Further documentation hunting is not an engineering prerequisite under that acceptance. |
| FT-BO live ownership/mapping/USE enforcement | NEEDS-TARGET-STATE-CHANGE | Offline owner and mapping/relocation/lifetime implementation exist; candidate never loaded. | Reviewed ABI-compatible path actually establishes the guarded runtime postconditions during the authorized experiment. |
| FT-SERVICE, bootstrap ready predicate, completion and color output | NEEDS-TARGET-STATE-CHANGE | Fixed XHW/TA/raster executor, bounded status ledger and color readback exist; none ran on the target. | Attributable ready/TA/raster events and expected 32×32 output in a newly authorized one-shot experiment. GPU-to-CPU readback visibility remains a recorded uncertainty, not proved by CPU-to-GPU publication acceptance. |
| Installed-kernel exported CRCs/config/generated-header evidence | CLOSED | Capture 08: exact installed image/header package version; all 222 original imports match; boot/build config and generated configuration agree. | Preserve captured evidence. This is not a complete prepared build environment or a candidate artifact. |
| Nine candidate-only import CRCs | CLOSED | Captured attributable table covers all nine, and all 228 rejected-candidate imports. | Verify every import in a newly built artifact; coverage is not acceptance of the old CRCs. |
| Successor preparation/config/header/layout gate | CLOSED | Fresh normal Kbuild generates target-exact syscall header; complete independent comparison passes with only documented metadata differences; failed predecessor preserved. | Preserve successor/evidence; future candidate build must use qualified inputs unchanged. |
| Compatible candidate artifact/build qualification | CLOSED | Candidate #1:230/230; lifecycle derivative:232/232; zero missing/mismatched CRCs, exact ELF/vermagic/layout, qualified provenance and bit-identical repeats. Old 934bd9… remains rejected. | Preserve both distinct artifacts/evidence; this closes offline ABI obligations only. |
| Hot gma500 transition/rollback qualification | NEEDS-READ-ONLY-TARGET-EVIDENCE | Derivative cleanup correction is compiled/tested; active original still lacks DRM IRQ release. Replaying its first hot unload remains blocked. | Capture normal boot module-loading/initramfs and original fallback facts, then review a first-load route avoiding original hot teardown. No such observation or action is authorized here. |
| Public post-reset driver/display state | CLOSED | Captures 05/06 verify original hash/build ID/Live state, PCI/DRM/fb/VT and runit→runsv→slimski→Xorg on the expected kernel. | Observation matches the baseline; dynamic guards must still be rechecked before any future authorized deployment. |
| Privileged supervisor status / scoped build capture | CLOSED | Capture 08 root sv status and final guards pass; build evidence preserved and hashed. | No further target observation required for these scoped facts. |
| Old residual-fbcon blocker and old partial-state-as-current claim | SUPERSEDED | Console release was tested; normal removal actually succeeded. Operator later reports reset and restored display. | Do not repeat reference archaeology or treat the old partial state as current. Fresh state still needs verification. |
| Old Gate B PASS / whitelist execution authorization | SUPERSEDED | Attempt 03 authorization was spent; its pinned artifact failed and the normal restoration path faulted. | A new exact-action review and explicit authorization after ABI and transition correction. |
| Current Gate B readiness | NEEDS-READ-ONLY-TARGET-EVIDENCE | ABI-qualified derivative exists; no executable original-driver transition/fallback route is qualified. Prior one-attempt authorization is spent. | Boot-route evidence and exact revised transition/recovery review, then new whitelist/authorization and fresh guards. |

The fixed service is not missing merely because upstream gma500 has no
legacy CMDBUF/XHW service: the narrow internal equivalent is implemented in
`kernel/sgx535_frozen/`. FT-BO/FT-SERVICE are no longer requests to invent that
software. Their live postconditions remain unobserved. No general scheduler,
arbitrary command ABI, or additional Phase 7 investigation is required here.

## ABI and transition findings

The [module-version follow-up](attempt03-module-version-followup.md) remains
valid. Preserved original SHA-256:
`7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb`.
Rejected candidate SHA-256:
`934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`.
Original/candidate `module_layout` CRCs are `0xb84efb99` / `0x995e9910`.
Both candidate and pre-build table checks return **REJECT**, with 150 CRC
mismatches. The candidate has nine imports not covered by the original:
`drm_clflush_pages`, `memcmp`, `memcpy`, `module_put`, `request_resource`,
`try_module_get`, `usleep_range`, `vmap`, `vunmap`. The original proves its
own imported CRCs, not the complete installed kernel ABI or post-reset state.

A concrete offline transition concern is visible in the retained source:
[`psb_driver_load`](../phase4-2-data/antix-source/drivers/gpu/drm/gma500/psb_drv.c)
calls `drm_irq_install`; `psb_pci_remove` calls `drm_dev_unregister`,
`psb_driver_unload`, and `drm_dev_put`, but that unload function has no
`drm_irq_uninstall` call. The retained 5.10 DRM core's `drm_dev_unregister`
does not itself invoke that helper for this MODESET/GEM driver.
[`gma_power_uninit`](../phase4-2-data/antix-source/drivers/gpu/drm/gma500/power.c)
only disables runtime PM/sets its suspended state.
[`psb_irq_uninstall`](../phase4-2-data/antix-source/drivers/gpu/drm/gma500/psb_irq.c)
is the hardware-mask callback; it is not the DRM helper that releases the
registered IRQ with `free_irq`. The preceding three integration patches do not
repair it. The new separate lifecycle patch corrects the derivative, but cannot
change teardown of the active original; see the current report. These source facts justify reopening **transition
lifecycle qualification**, not Phase 7 archaeology. A stale IRQ action/name
is consistent with the retained `strcmp → register_handler_proc → __setup_irq`
recovery trace, but the precise faulting pointer/root cause is **INFERRED,
not proved**. No kernel fix or target experiment was undertaken in this audit.

## Verification in this audit

- Full scoped Python discovery: **187 tests PASS** (186 baseline; one new
  retained-client artifact/input identity regression). Strengthened existing
  `--complete` refusal test checks the exact four architectural labels.
- Strict host C builds and UBSan runs for fixed service, kernel contract and
  fixed IO: **all exit 0**, no sanitizer diagnostics.
- Generated initial-image `--check`: **PASS**; existing Python/C golden
  equivalence and DPM_TA_MEM_FREE regressions pass unchanged.
- Two independent dry runs: both SHA-256
  `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
- `frozen_triangle_image.py --complete`: exit 1,
  `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.
- All 15 copied fixed implementation/UAPI/generated files match their
  retained temporary antiX build-tree copies. No candidate rebuild performed:
  required target-qualified build inputs are missing.

`--complete` is a **frozen static/architectural proof check**, not a dynamic
Gate B or deployment readiness evaluator. Its conservative refusal remains
correct within that scope; its four labels are **not** an exhaustive current
critical-path list. It does not consume operator acceptances or test module
ABI, hot-reload safety, post-reset state, or authorization. Its output and
fail-closed predicates were not weakened; its docstring now states this scope.

## Historical critical path after capture 08 (superseded above)

1. **OFFLINE:** separately scope the first external candidate build using the
   equivalence-qualified successor, unchanged captured target export table and
   local GCC environment. Verify all new artifact imports/CRCs, source/config,
   metadata and hash. No CRC patching/forced loading.
2. **OFFLINE:** resolve/review the IRQ teardown and failed hot-restoration
   lifecycle before proposing another transition. A compatible module alone
   does not qualify the old remove/restore procedure.
3. Review a revised exact action, preserving architectural UNKNOWNs and scoped
   operator decisions; obtain new authorization. Only then may the approved
   guards/deployment/one-shot experiment be considered. No Attempt 04 is begun.

**Single smallest next step: OFFLINE** — separately scoped first external
candidate build using the equivalence-qualified successor and captured target
table, then complete import/ABI/source/config/artifact qualification. No additional
Mini 12 contact is needed for the captured CRC/config/provenance facts.
