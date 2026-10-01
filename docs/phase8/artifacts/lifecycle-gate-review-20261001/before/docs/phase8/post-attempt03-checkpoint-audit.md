# Post-Attempt-03 checkpoint audit — 2026-09-30

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
