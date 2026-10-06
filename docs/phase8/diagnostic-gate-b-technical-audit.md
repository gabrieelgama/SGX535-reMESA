# Diagnostic Gate B technical audit — 2026-10-02

Gate B remains **BLOCKED**. Whitelist: `[]`. No target contact or SGX action occurred in this audit. A reported platform restriction interrupted the earlier exact-action review; it is not evidence of an SGX535 failure. Its explanation is not a technical prerequisite for the experiment.

The current evidence is the [66/66 diagnostic capture](../hardware-evidence/MINI12-20261002T063600Z-DIAGNOSTIC-FIRSTOWNER-02/RESULT.md), the [diagnostic staging and previous same-image recovery](../hardware-evidence/MINI12-20261002T054751Z-DIAGNOSTIC-STAGING-01/RESULT.md), and the [recorded diagnostic offline qualification](../hardware-evidence/MINI12-20261002T053407Z-STOCK-POSTATTEMPT05-PREFLIGHT-01/diagnostic-image-offline-recheck.json). Older first-load and hot-transition reports describe their own checkpoints.

| Predicate | Supported evidence | Remaining limit or contrary evidence | Where further evidence belongs |
| --- | --- | --- | --- |
| Target/kernel identity | Current capture: Inspiron 1210, i686, expected release, PCI identity and boot `55bb90c8-fffe-4993-b656-7aa7a7b9b5e8` | State after the capture is not newly observed | Existing immediate same-boot checks before any future invocation |
| Diagnostic ABI/build qualification | Recorded ELF32/i386, expected vermagic, module_layout `0xb84efb99`, 232/232 imports, zero missing/mismatched CRCs | No binary/source requalification performed in this audit; no contrary result in the permitted records | Reuse the qualified record unless drift is detected |
| Diagnostic image and loaded identity | Captured staged image size/hash agrees with offline qualification; loaded note agrees with recorded diagnostic Build ID | Build-ID/boot provenance is operational identification, not memory attestation | Existing same-boot identity checks |
| First ownership | Exact ordered hook trace and diagnostic note; PCI/DRM/fb/IRQ16 guards pass | Old reports lacking a trace are superseded for this captured boot | No repeated first-load cycle is justified by these records |
| Display/userspace/access | Physical display and userspace operator-reported normal; services and bounded SSH capture pass | Future availability is not guaranteed | Observe under the existing procedure if a future action is allowed |
| Kernel health and signature policy | Current capture accepts the bounded known diagnostics and taint mask; diagnostic module is Live | This does not establish fault-free future GPU execution | Existing fresh health check; HOLD on new fault |
| STOCK recovery and preservation | Previous diagnostic boot returned to distinct STOCK boot with 39/39 root guards; stock files/default preserved | Wrapper STOP was a field-name mismatch; raw evidence is unchanged. Recovery from a future GPU failure is not guaranteed | Existing operator machine-boundary recovery; no hot restoration |
| Original hot removal/restoration | Retained source/lifecycle evidence identifies the IRQ-lifetime defect | Remains BLOCKED for that path | It is not a prerequisite of the already observed first-owner path |
| Frozen workload and diagnostic semantics | Recorded diagnostic qualification says workload/UAPI/order/addresses/timeout/HOLD/one-shot semantics unchanged; prior 330 PASS, one existing skip | Direct review of the previously restricted implementation/wrapper items was not repeated | Reuse the recorded result; restricted items remain unavailable |
| Architectural source/publication questions | Existing bounded-attempt policy records scoped accepted assumptions | L12, FT-AUX and architectural publication remain UNKNOWN; `--complete` remains `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE` | They are not newly closed by first-load or ABI success; no new gate is added here |
| Frozen client and immediate invocation prerequisites | Pinned i386 client qualification exists; no client invocation occurred in this diagnostic cycle | No exclusive client staging/readback or final immediate same-boot validation is recorded for this boot | Target-dependent existing pre-invocation procedure, if independently permitted |
| Current exact-action decision | Passive qualification is supported | A completed permissive exact-build/action decision is not recorded; whitelist is empty | Gate B stays BLOCKED; the unavailable review item is not replaced by an indirect review |

## Offline checks and unresolved questions

Seven consistency assertions passed: recorded image versus captured destination hash/size; loaded note versus recorded Build ID; release versus vermagic; all 66 root guards; current boot distinct from previous STOCK recovery; captured saved STOCK default; preserved passive manifest hashes. These checks read permitted records only and add no live observation.

The capture-format fix is unchanged from its recorded 30/30 passing capture tests. Those tests were not repeated without a new change. The fix accepts `kernel` or `kernel_release`, rejects conflicting values, and does not rewrite captured records. The historical wrapper STOP remains preserved.

No new failing offline Gate B technical predicate was established. The original requester during stock boot and complete architectural source/publication semantics remain UNKNOWN, but neither is a newly demonstrated blocker to the specified first-owner experiment. [Attempt05](../hardware-evidence/MINI12-20261002T041334Z-SGX-ATTEMPT-05/RESULT.md) still has an UNKNOWN failing callback, TA fire UNKNOWN/POSSIBLE, TA completion unobserved and raster submission NO. That runtime question is why the diagnostic candidate exists; it is not a new requirement to solve before collecting its diagnostic result.

The smallest missing target prerequisite is the existing immediate same-boot/client verification before invocation. This audit does not perform or authorize it. No further boot, rebuild, repeated passive capture or new test infrastructure is justified by the reviewed evidence. The particular previously restricted Gate B/artifact/implementation review remains unavailable; this does not make independent documentation or passive records unavailable.

The next proposed evidence-producing action is the narrowly scoped read-only portion of that existing pre-invocation check, only if target-contact authorization and the procedure permit it. Client staging and SGX execution remain separate state-changing actions. No stage/completion/readback/triangle is inferred from preparation.

## Immediate checks versus qualification history

This is an audit of the existing requirements, not a replacement invocation wrapper. The previously restricted exact-action/wrapper/kernel-entry review was not reopened or reconstructed. The following separates facts that can go stale in a running boot from qualification already recorded above.

Immediately before a future permitted invocation, the existing procedure still needs:

- The expected diagnostic boot ID, kernel/architecture and loaded module identity, with the module Live. Check the loaded note against the qualified diagnostic identity. Installed stock `modinfo` is not proof of the loaded derivative.
- Current expected PCI/DRM/framebuffer/VT/IRQ16 ownership and acceptable kernel health/taint. An old capture cannot establish that these stayed unchanged.
- An unused one-shot for this boot, supported by the invocation/evidence history. Do not issue an ioctl to test availability, reset the counter or reuse Attempt05's consumed module.
- The exact frozen client, using the existing exclusive-create/no-follow staging and destination readback rules; protected file identity and the expected absent output destinations. Its pinned SHA-256 remains `758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf`. A matching hash does not replace path/ownership checks.
- Working noninteractive privilege in this boot. A local `sudo -v` report alone did not guarantee that previous remote captures could authenticate.
- The existing exact-build/action decision and authorization, recovery availability, unchanged bounds and no-retry/HOLD rules. The whitelist is currently empty.
- A same-boot check spanning the preparation/invocation boundary, so a reboot during preparation cannot silently reuse earlier evidence.

These are operational prerequisites, not all read-only operations. Client staging is state-changing; invocation is a separate action. Neither is authorized by this offline audit.

No contradiction in the reviewed records justifies repeating the full toolchain, source/config/header, ABI/import, image reconstruction, repeat-build or first-owner qualification. Preserve those records and investigate only if fresh checks detect drift. Rehashing every historical source, stock boot file and build input immediately before an ioctl would not establish current loaded ownership or an unused one-shot.

The current first-load path does not require the old original-driver hot removal, PCI unbind, VT/display teardown, replacement or hot restoration. Those operations remain prohibited, not repaired. A new menu photo or boot cannot recover a fact missing from Attempt05. The current 66/66 capture already establishes the ordered first-owner trace for its boot. Normal userspace/Xorg under that selected path is not itself evidence that a new hot-transition sequence is needed.

The old 120-second combined handoff/capture window is superseded by the recorded v2 timing. Its 600-second boot watch and 1,200-second passive-capture ceiling are qualification timing, not replacements for the workload's `5×HZ` and 300,000-sample bounds. Historical documents retain their original requirements; none was rewritten here.

## Attempt05: the remaining failure interval

[Attempt05](../hardware-evidence/MINI12-20261002T041334Z-SGX-ATTEMPT-05/RESULT.md) established owner construction, CPU publication and translation publication, then returned `operation_errno=-1`, `phase=11`, `events=0`. It did not record an internal failing callback. The diagnostic service is used below to map the unchanged control flow; the earlier recorded diagnostic-only comparison supplies that correspondence. This audit does not repeat the unavailable kernel-entry review.

In [`frozen_fixed_service.c`](../../tools/psb-dri-re/frozen_fixed_service.c), `prepare()` sets `unsafe_possible=1` before device maintenance. Consequently a preparation failure can produce HELD_AFTER_FAILURE before the TA-fire callback is reached. Phase 11 alone is not evidence of a fire.

The table is ordered. A nonzero backend return short-circuits the sequence through `fail()`. Inline checks also short-circuit. All rows through stage 13 precede the frozen `TA_FIRE` callback; this does not imply that earlier initialization/maintenance performs no device operations.

| Stage / boundary | Operation and intervening checks | What the diagnostic distinguishes |
| --- | --- | --- |
| 3 DEVICE_MAINTAIN | Maintenance callback; then session advance, bootstrap begin and rev121 init-plan generation | Callback failure: source 1, exact stage/raw return. Later inline failure: source 3, stage 3; the individual helper is not named. |
| 4 INIT_WRITES | Initialization writes; then bootstrap INIT_WRITES_RETURN | Callback versus following bootstrap check. |
| 5 XHW_INIT | Initialization callback; then XHW_INIT_REPLY and TA-cookie generation | Callback versus inline checks, but not which inline check. |
| 6 TA_INFO | TA-info callback; then TA_INFO_REPLY and scene-info generation | Callback versus inline checks. |
| 7 SCENE_INFO | Scene-info callback; then SCENE_INFO_REPLY and TA-load-plan generation | Callback versus inline checks. |
| 8 TA_LOAD | Load callback; then LOAD_KICKS, LOAD_STATUS2, INITEND_STATUS and TA_LOAD_REPLY checks | Callback raw return plus existing load flags/status2/initend. Several bootstrap checks still share stage 8. |
| 9 SCENE_VALIDATE | Scene validation callback; then SCENE_VALIDATED | Callback versus bootstrap check. |
| 10 USE_RESERVE | Reserve callback; success sets `use_owned=1` | Exact callback failure and raw return. |
| 11 USE_PROGRAM | Program callback | Exact callback failure and raw return. |
| 12 STATUS_BASELINE | Baseline callback; then selected status1/status2 rejection and service-ready check | Callback versus service check, with existing baseline words. Service-check subbranches share stage 12. |
| Between 12 and 13 | Ownership/bootstrap checks, TA schedule/fire plan generation and `session_enter_fire()` | These return BAD_REQUEST directly, without `fail()`. After a successful prepare they do not explain phase 11 through the normal path. |
| 13 TA_SCHEDULE | Schedule callback after entering FIRE_POSSIBLE | Exact callback failure; the frozen TA_FIRE callback has not been reached. |
| 14 TA_FIRE | TA-fire callback | Failure stage/raw return. Entering or returning from the callback is not, by itself, proof of physical TA execution. |
| 20 STATUS_SAMPLE | Existing `sample_and_ack()` call | Source 2 with raw return and returned status words. Reaching this point means the TA_FIRE callback returned zero, not that TA completion was observed. |
| 21 STATUS_PROCESS | Attribution/sequence/phase checks, allowed status bits, fault handling and event-ledger processing | Reached stage and status words distinguish this family. Some HOLD branches leave failure stage/source unset; see below. |
| 22 TIMEOUT | Sample-budget exhaustion in `run_once()` | Source 4, stage 22, raw BAD_REQUEST. This label identifies loop exhaustion, not every possible time-related backend failure. |

A backend or status-source failure returns the generic `-1` to the outer service even when its underlying return differs. The diagnostic preserves that underlying return for sources 1 and 2. Inline service checks already return the generic value, so several helper failures still collapse to the same tuple. The diagnostic does not expose bootstrap `last_event` or the status-source `exclusive_owned` value.

Accepted TA completion records event bit 1 before raster preparation. The ledger does not clear it on a later failure. Attempt05's zero-event result therefore excludes the normal raster preparation/reset/schedule/fire and USE_RELEASE paths. It does not distinguish preparation failure from TA schedule/fire failure or pre-completion status failure. TA fire remains UNKNOWN/POSSIBLE; TA completion was not observed; raster submission remains NO.

## Reporting limits checked offline

`stage_reached`, `failure_stage`, `failure_source`, `raw_result`, `observation_stage` and the already-available load/status/initend values must be interpreted together with phase and events. They are not a history of every successful callback.

Two previously unasserted reporting cases were exercised with the existing mock backend and actual service/contract code:

| Mock input | Result | Diagnostic tuple |
| --- | --- | --- |
| Selected fault bit 28, attributed ownership | `-1`, phase 11, events 0 | reached 21; failure stage 0; source 0; raw 0; status1 `0x10000000` |
| Status bit 24 with exclusive ownership rejected | `-1`, phase 11, events 0 | reached 21; failure stage 0; source 3; raw -1; status1 `0x01000000` |

The first case occurs because the contract accepts the fault observation and sets HOLD while returning zero; the service then returns BAD_REQUEST. The second records a service-check error but does not assign `failure_stage`. Neither tuple means success or absence of a failure. Both cases made 14 mocked backend calls and one sample, with no raster/reset/release callback. These are offline fixtures, not observations of Attempt05 or the GPU.

Both new cases passed under UBSan with warnings treated as errors. Compilation and execution exited 0; stderr was empty. The established full harness was not rerun. The [probe](artifacts/diagnostic-observability-audit-20261002/probe.c), [exact commands/exit codes](artifacts/diagnostic-observability-audit-20261002/commands-results.json) and [output](artifacts/diagnostic-observability-audit-20261002/run.stdout) are preserved. No production source, candidate, image, UAPI or workload changed.

The unchanged client/UAPI reports errno, outcome, phase, events and color results, not the internal diagnostic tuple. The recorded diagnostic implementation reports that tuple in the kernel HOLD log. A future permitted invocation therefore needs its existing attributable kernel-log capture as well as client output; stdout alone will not answer the new diagnostic question. The previously restricted kernel-entry/reporting item was not reopened to reproduce that material.

One bounded invocation can distinguish a failing backend stage, status-source failure, status-processing family and sample-limit exhaustion if its report is emitted and preserved. It cannot guarantee an exact helper or instruction inside a callback. A hang or lost log can leave the boundary UNKNOWN. Multiple inline checks share a stage, status-source internal causes are not decoded here, and phase 11/raw -1 without the tuple remains insufficient. No additional ioctl or diagnostic GPU transaction is justified to obtain those fields.

## Continuation checkpoint

GATE B: BLOCKED

WHITELIST: []

SAME-BOOT MINIMUM: expected boot/loaded diagnostic identity and Live state; current ownership/health; unused one-shot history; exact protected client/readback/output paths; privilege; existing authorization/recovery/HOLD bounds; no intervening reboot.

ATTEMPT05 NARROWED INTERVAL: DEVICE_MAINTAIN through pre-completion STATUS_PROCESS or sample-limit TIMEOUT; includes preparation and TA_SCHEDULE/TA_FIRE. Raster/release paths excluded by zero events. Failing callback UNKNOWN.

DIAGNOSTIC OBSERVABILITY: backend stage/raw return, status-source return, load/baseline/status snapshots and status-processing/timeout family; preserve the complete tuple and kernel HOLD report. Not every inline helper is uniquely identifiable.

STALE REQUIREMENTS REMOVED: none from authorization policy. Hot-transition operations and old 120-second timing are excluded from this current-path checklist; historical records remain intact. No repeat first-owner boot, build or full suite is justified by this audit.

UNRESOLVED AMBIGUITIES: Attempt05 callback and TA fire; shared inline labels; zero failure-stage/source on some status HOLD paths; backend-internal failure causes; whether a future report survives a hang. The previously restricted exact-action/wrapper/kernel-entry review remains unavailable and was not retried.

EXACT NEXT TARGET-SESSION STEP: if normally permitted and separately authorized, perform only the existing bounded read-only same-boot portion against boot `55bb90c8-fffe-4993-b656-7aa7a7b9b5e8`, using the qualified loaded diagnostic identity. Any mismatch means STOP. A permissive exact-action decision, client staging and the single workload invocation remain separate boundaries; this audit supplies no substitute wrapper or whitelist.

FILES CHANGED: this audit and its new small mock-probe evidence directory only.

TESTS ACTUALLY NECESSARY/RUN: two new status-reporting edge cases, UBSan PASS; no repeated full suite, capture, build, hash qualification or archaeology. Documentation diff/link checks recorded with this update.

TARGET CONTACT: NONE
