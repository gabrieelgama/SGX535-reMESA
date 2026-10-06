# Codex continuation: SGX535-reMESA, 2026-10-02

Repository: `/home/gama/sgx535-gfx`. This is a session-continuation document. Technical conclusions belong to the linked evidence and audit. No target contact occurred while writing it.

## Corrected-image live preparation — 2026-10-05

[New exact live-preparation authorization](corrected-image-live-preparation-maintainer-20261005-b19838c1.json)
permits the corrected `b19838c1…` image only: fresh passive STOCK preflight,
conditional distinct staging, one manual corrected boot and passive preparation;
no client/ioctl/SGX execution. Evidence:
`/home/gama/sgx535-offline/phase8-corrected-live-preparation-20261005T041729Z-b19838c1/`.
Actual fresh STOCK boot `cb90b955-4477-4603-bc6c-1cb11f19197f` passed **57/57**
guards, including the unchanged health classifier. Exact corrected image staged
once at the separate `-b19838c1` destination; independent poststage **57/57 PASS**.
The malformed image and old backup/entry remain preserved. The new six-entry
config preserves the previous five entries as an exact prefix; STOCK remains
the saved default. No remote reboot or client execution occurred.

The operator reported the one corrected boot reached normal userspace and local
sudo readiness at approximately 135 seconds. Actual bounded first-owner capture
started at boot uptime 138.02 seconds and passed **74/74** guards on boot
`ad1c8ae6-0c52-4ed0-b117-bcd7473bffb1`. Both qualified module notes/Live states and
the complete BEGIN/FILES-VERIFIED/PASS loader trace matched. The actual kernel
capsule was **UNUSED**; no unexpected consumed operation or boot HOLD appeared.

Protected client/evidence preparation passed **50/50** checks. Exact approved
client was copied only as data into a root0500 single-link file; no client ran.
Private root0700 evidence destination:
`/root/sgx535-frozen-seq1-cd9b947371f18c2d19af05bfda69fbf9462c2e62-b19838c1/evidence-ad1c8ae6-0c52-4ed0-b117-bcd7473bffb1`.
Actual context/initial capsule/kernel preparation originals were synced and
readback verified; all future response/image/stream/capsule outputs remain absent.
Independent final passive snapshot passed **196/196** checks with the same boot,
module pair, clean health and unchanged UNUSED capsule. Only Xorg was observed
holding a DRM file; that point snapshot is not proof of continuous isolation.

`pre07-result.json` records **PRE07 BLOCKED BY UNESTABLISHED SOURCE QUIESCENCE /
CONTINUOUS PRODUCER ISOLATION**. These requirements already exist in the qualified
package (`guard_requirements[2]`); the UNUSED observer does not expose hardware
pending state or delayed-event exclusion. No MMIO/probe, invocation, new interface,
service change or second candidate boot was used to fill that evidence gap.
No ready execution card was issued. Historical Gate B/client records retain their
original exact context; they were not silently rebound in archive inputs.
**sgx_execution_authorized=false; SGX invocations=0; triangle=NOT ATTEMPTED;
READY FOR EXECUTION AUTHORIZATION: NO; readback=NONE; completion=UNOBSERVED.**

## Latest operator state — normal STOCK, 2026-10-05

The maintainer reports that the malformed candidate was exited through the normal
recovery/reboot boundary and the Mini 12 is now powered on in normal STOCK with
normal display/userspace. The corrected `b19838c1…` image has **not** been booted;
no triangle client or intentional SGX work was reported. Exact report preserved
in `/home/gama/sgx535-offline/phase8-firstload-hash-fix-20261005T040456Z-cd9b9473/operator-current-stock-state.json`.
This supersedes only the current physical-state assumption; historical HOLD,
malformed image and recovery/preparation records remain unchanged.

Current boot UUID/loaded STOCK identity and fresh qualified health guards are
not yet directly collected. Start from this reported running STOCK; do not
request another STOCK reboot merely to begin preparation. Next is a fresh passive
STOCK preflight under applicable live authority, then corrected-image preparation
only after PASS. Preserve the occupied malformed image/entry and use distinct
corrected-image destinations. This update introduces no staging/boot/FIRE
permission. No target contact occurred while recording it.
**sgx_execution_authorized=false; SGX invocations by this work=0;
triangle=NOT ATTEMPTED; READY FOR EXECUTION AUTHORIZATION: NO.**

## Prospective FIRSTLOAD image construction fix — 2026-10-05

[Scoped offline qualification](firstload-hook-binding-fix-qualification-20261005-b19838c1.json)
records the minimum stale-hook-hash fix. The new image is **50,811,746 bytes**, SHA-256
`b19838c188e9257834ed470f1c401f6ce3938da46f8dae3b7822e46839db7142`.
Evidence/package: `/home/gama/sgx535-offline/phase8-firstload-hash-fix-20261005T040456Z-cd9b9473/`.
The finished archive verifies `/init` expects exactly the packaged hook's digest;
two builds are byte-identical. Only `/init` differs from the malformed image.
Driver `cd9b947371f18c2d19af05bfda69fbf9462c2e62`, observer
`143b1284fc643ea9a7ddd4aeeefa05cdc682718b`, hook, approved client and UAPI bytes
are unchanged. 39 scoped construction/regression tests pass, zero skips.

The malformed image and all prior FIRSTLOAD HOLD evidence remain unchanged.
The qualified malformed `/init` retained the old hook digest and therefore refused
the new hook before execution; this is an image construction error, not evidence
of a driver/SGX execution failure. Its outer HOLD is a conditional failure branch,
not the intended successful FIRST-LOAD ONLY endpoint.

**Corrected artifact READY FOR STAGING within its offline scope.** This fix
performed no staging, target contact, boot, module load or client execution and
creates no new live authority. The prior live STOP remains; a current healthy
STOCK baseline and separately authorized corrected-image preparation are still
required. Existing one-use boot authorization is not silently renewed.
**sgx_execution_authorized=false; SGX invocations=0; triangle=NOT ATTEMPTED;
READY FOR EXECUTION AUTHORIZATION: NO.** No Git staging/commit.

## STOCK recovery and resumed exact preparation — 2026-10-05

[New bounded recovery/preparation authorization](stock-recovery-preparation-maintainer-20261005-cd9b9473.json)
permits one operator-controlled STOCK recovery, then exact candidate preparation
only if the new baseline passes. Evidence:
`/home/gama/sgx535-offline/phase8-stock-recovery-and-preparation-20261005T032920Z-cd9b9473/`.
The prior failed STOCK boot and all its evidence remain unchanged; its RCU stall
occurred before candidate staging and its cause remains UNKNOWN.

The operator reported recovery readiness; new STOCK boot
`8d919fe1-426e-4bde-ba93-db49ac544eb6` was directly observed with original driver
and root access. **Fresh STOCK health/baseline PASS: 52/52**, no current-boot RCU
stall/Call Trace. The exact qualified image was staged once with exclusive
creation/sync/readback; independent poststage **53/53 PASS**. Its SHA-256 remains
`5001a64ff5ef78751aea45f8762eeea8abbfc8ef3c3774c12a34fa214858288d`.
The old four-entry config is preserved as an exact prefix and unique backup;
only one exact new entry was appended, while STOCK remains the saved default.

Current checkpoint: **STOP — operator-reported candidate boot HOLD**, received
2026-10-05. Latest exact operator wording: **“SGX535 FIRSTLOAD HOLD: no continuation or second load.”**
Preserved in `operator-firstload-hold-clarification.json`; the earlier wording
remains unchanged in `operator-candidate-hold.json` and `final-preparation-result.json`
inside the evidence directory above. No further target connection, module load,
reboot or client invocation was attempted after that report. The single candidate
boot attempt budget is treated as consumed; no second attempt is authorized.

Candidate boot UUID, actually loaded driver/observer, capsule initial state,
normal userspace, fresh candidate guards and protected live destinations remain
**NOT ESTABLISHED**. The specific HOLD trigger is **UNKNOWN**: the report does not
contain the preceding diagnostic, and no candidate kernel log was collected.
Do not infer loaded identities from staged filenames or infer an SGX invocation
from the preparation HOLD. The prior STOCK RCU failure remains separate,
unchanged historical evidence; the captured new STOCK boot did not reproduce it.
**PRE07 BLOCKED BY CANDIDATE BOOT HOLD; READY FOR EXECUTION AUTHORIZATION: NO.**
No ready execution card was issued. The verified STOCK default/recovery files
remain pre-boot evidence, not proof of a completed post-HOLD recovery. Any further
recovery/evidence activity needs its applicable bounded authorization; no remote
recovery, second load or experiment retry is introduced here.
**sgx_execution_authorized=false; SGX invocations by this work=0;
triangle=NOT ATTEMPTED; readback=NONE; completion=UNOBSERVED.**
No remote reboot, hot replacement, client execution or SGX operation occurred.

## Latest live-preparation checkpoint — 2026-10-05

[Exact maintainer live-preparation authorization](live-preparation-maintainer-20261005-cd9b9473.json)
covers the qualified Architecture B pair/image below, one controlled candidate
boot and passive preparation only. It does not authorize the frozen ioctl or SGX.
Evidence: `/home/gama/sgx535-offline/phase8-live-preparation-20261005T024220Z-cd9b9473/`.

Before contact, exact package/module/image/client/UAPI identities verified PASS.
Two successful passive contacts observed STOCK boot
`a837fea7-1b10-4b5a-b52a-bb1026dcde95`, expected kernel/i686/Inspiron1210,
and loaded original driver `d8dcb4d38b774ad64799d5e13aaedede069371f3`.
STOCK kernel/initrd/module and saved GRUB default identities matched; current
four-entry custom.cfg was preserved as the proposed new entry's exact prefix.
The historical candidate boot89fc is not current.

The first actual sudo-n check returned password-required. The maintainer then
reported local authentication, normal userspace/display and keyboard/GRUB/power
controls. One ROOT STOCK preflight connection timed out before its program ran;
that failure remains preserved. Following maintainer endpoint clarification, a
separately recorded fresh passive preflight reached root on the same STOCK boot.
It passed **51/52 guards** but the qualified health classifier **REJECTED** an
`rcu_sched` CPU stall and Call Trace records. Root privilege and within-capture
boot continuity were directly established; a healthy baseline was not.
**STOP BEFORE STAGING. PRE07 BLOCKED BY CURRENT STOCK KERNEL HEALTH REJECTION;
READY FOR EXECUTION AUTHORIZATION: NO.** Do not clear logs, exempt the fault or
boot the experimental candidate from this failed baseline. Use the existing
operator-controlled recovery/healthy-STOCK boundary before resuming preparation.

Final result: `final-preparation-result.json`; full log:
`stock-preflight-after-endpoint-confirmation/kernel-log.txt`, lines679/686/699.
No target writes, transfer, candidate boot, reset/recovery or SGX occurred.
Four passive connection attempts (three successful); **sgx_execution_authorized=false;
SGX invocations=0; triangle=NOT ATTEMPTED.** Actual destination preparation and
loaded new-producer verification have not occurred. Failed attempts remain
unchanged; no automatic retry or execution card was issued.

## Latest prospective offline implementation update — 2026-10-05

Architecture B has been implemented under the delegated
[APPROVE_MINIMUM_IMPLEMENTATION](current-operation-result-provenance-implementation-maintainer-20261005-314e2f3b.json)
and is **PROVENANCE MECHANISM QUALIFIED within its offline scope**.
[Implementation report](current-operation-result-provenance-implementation-20261005.md)
and [exact qualification](current-operation-result-provenance-qualification-20261005-cd9b9473.json)
are authoritative for the new candidate. Evidence and `one-shot-package.json`:
`/home/gama/sgx535-offline/phase8-provenance-implementation-20261005T005304Z/`.
Earlier missing-engineering statements below are historical; they do not describe
this new independently implemented producer. Restricted-material prohibitions
and unresolved legacy G01′/G02′ findings remain unchanged.

- New driver Build ID: `cd9b947371f18c2d19af05bfda69fbf9462c2e62`,
  SHA-256 `400b16b14a7fd648fe219842e3b0d5994ee8eec910e5a7894f31c373098e8b54`.
- New observer Build ID: `143b1284fc643ea9a7ddd4aeeefa05cdc682718b`,
  SHA-256 `6842b21b4ab0905e198ca95e07bee7ff52f0d225c8992e9ef18921055482a11d`.
- New image: 50,811,745 bytes,
  SHA-256 `5001a64ff5ef78751aea45f8762eeea8abbfc8ef3c3774c12a34fa214858288d`.
- Approved response-export client, fixed UAPI, geometry and request bytes unchanged.
  New supplemental root0400 read-only evidence interface:
  `/proc/sgx535_current_operation`, 4152 bytes. No launcher, UUID or raw STATUS journal.
- Tests: 45 capsule checks and 7 real-service differential cases PASS in native,
  UBSan and static i386/qemu; 13 new checker/materialization/binding tests PASS,
  zero skips. Module/import/CRC/reproducibility and image checks PASS.
- **PRE07 live readiness: BLOCKED; READY FOR EXECUTION AUTHORIZATION: NO.**
  The new pair/image are unstaged and unbooted. No new boot ID, verified loaded
  producer, fresh identity/isolation/health/destination/privilege/recovery receipts
  exist. Supplied saved bytes alone remain UNKNOWN hardware attribution.
- Prior boot `89fc7306-6dac-4ab9-af62-360b7152ef22`, Build ID `314e2f3b37195dc56df7c57dd938d78377a5ea8e`
  and 70/70 remain valid historical evidence, not qualification of changed bytes.
  Original Gate B PASS/whitelist and exact client decisions retain their original
  scope; the changed candidate needs exact-context binding and fresh observations.
  Do not hot-replace modules or use old hardcoded first-owner pins unchanged.
- Next separate maintainer-authorized step: stage the exact new artifacts, perform
  their first-owner boot and passive preparation/capture **without SGX**, then bind
  the observed context before a separate execution-authorization decision.
  No further implementation review is pending.
- **sgx_execution_authorized=false; SGX invocations=0; hardware interactions=0;
  triangle=NOT ATTEMPTED.** No completion/readback/new HOLD observed. No files staged
  or committed; historical decisions/raw evidence remain unchanged.

## Authoritative prospective maintainer readiness decision — 2026-10-04

**Current readiness state:** [new exact-context maintainer decision](gate-b-maintainer-readiness-20261004-314e2f3b.json), recorded 2026-10-04T07:00:07Z.
Authority: the project maintainer's explicit new decision in this conversation,
based on the reconstructed matrix **27 PASS / 0 FAIL / 0 UNKNOWN / 5 NOT_APPLICABLE**.
This prospectively supersedes earlier current-state statements that the project
readiness decision is missing; every earlier checkpoint/decision below remains
historical and unchanged. It is not an external/platform approval and does not
reopen restricted material or certify the unavailable invocation procedure.

- Gate B: **PASS FOR EXACT OFFSCREEN READINESS**.
- Whitelist: `[MINI12-SGX535-REV121-FROZEN-32x32-SEQ1]`.
- **sgx_execution_authorized: false**; SGX invocations on this candidate: **0**.
- Exact reviewed boot: `89fc7306-6dac-4ab9-af62-360b7152ef22`;
  Build ID: `314e2f3b37195dc56df7c57dd938d78377a5ea8e`.
  The new record binds the exact qualified module/image, existing selected client,
  UAPI and qualification/70-of-70 first-owner evidence. No fresh target observation
  occurred. Triangle: **NOT ATTEMPTED**; TA/raster/3D-memory-free unobserved; no readback.

**Prospective client-substitution review is now approved**, by the separate
[new maintainer substitution decision](prospective-client-substitution-maintainer-20261004-314e2f3b.json),
recorded 2026-10-04T08:56:20Z for request
`MINI12-PROSPECTIVE-CLIENT-SUBSTITUTION-REVIEW-20261004`. It accepts the existing
offline qualification and identical pre-call semantics for the exact successor
source/binary/UAPI, prospectively supplementing only the forward client binding.
Earlier substitution-unapproved statements below remain historical; original
request/qualification/Gate B decisions and old client artifacts remain unchanged.
The successor is **qualified for promotion**; no target selection/staging
transaction has occurred. This substitution review approval grants no target
contact, staging, boot or SGX execution permission.

**Response-export client substitution is now approved**, by the separate
[new exact maintainer decision](response-export-client-substitution-maintainer-20261004-314e2f3b.json),
recorded 2026-10-04T10:23:56Z. It prospectively binds only the qualified source
`919c2611e048e4a660ecae3542264adf5a5367d9b0fbe7fbceff2f45f90adedf`
and 775,264-byte binary
`2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835`
with the unchanged exact UAPI. The archive context binder accepts this new record.
Earlier pending/unapproved statements remain historical; requests, pending
proposal, prior decisions and artifact bytes were not changed. This approval
permits no staging, contact, fresh guard collection, SGX/ioctl, retry, action
alteration, restricted-wrapper reconstruction or triangle claim. Target
selection/staging has not occurred; execution remains false and SGX count zero.
The preceding approved client's missing successful-response export remains a
historical property; the newly approved response client has the qualified export.

Current PRE07 investigation: [permitted kernel observability inventory](pre07-kernel-attribution-20261004.md)
finds an existing internal active-owner/sequence/status attribution facility,
but no applicable reviewed external invocation capture/association route.
Successful kernel emission details remain UNKNOWN; a new token or kernel source
change is not established as necessary. Even resolving kernel attribution would
leave the applicable current-context guard source and protected target transaction
unestablished. The [PRE07 matrix](pre07-responsibility-matrix-20261004.json) retains
5 satisfied / 15 partial / 2 unavailable capabilities. The complete reviewed live
preservation executor and protected target destinations remain unavailable.
No fresh target checks occurred; historical 70/70 is not fresh invocation evidence.
**NOT READY for execution authorization**; no staging/contact/SGX or wrapper reconstruction.


Offline preservation/guard component work, 2026-10-04:
[archive/checker](../../tools/psb-dri-re/frozen_evidence_bundle.py) and
[18 tests](../../tools/psb-dri-re/test_frozen_evidence_bundle.py) now preserve
already-captured originals in unique private directories, sync originals before
creating a validation copy, retain partial failure evidence, seal finished
manifests and independently verify saved bytes. Supplied guard snapshots reuse
the existing identity/ownership/health conditions without the historical
first-owner clock or a target collector. Even apparently perfect synthetic
inputs retain UNKNOWN live attribution/freshness and cannot authorize execution
or establish a triangle. No transport, client execution or substitute invocation
wrapper was implemented. The approved client source/binary remain unchanged.

36 focused tests passed (18 new plus 18 validator integration tests). The saved
actual first-owner record remained consistent and explicitly not fresh; the
synthetic validator CLI left its original image unchanged. Logs, synthetic
artifacts and consistency results are in
`/home/gama/sgx535-offline/phase8-offline-preservation-guards-20261004T090254Z/`.
Its `authority-snapshot/` retains local copies of this handoff before this note
and both maintainer decisions. Git confirmed these authority files and the
existing round-2 preservation test are untracked; local snapshots are not Git
commits. The round-2 test was preserved unchanged. This work closes only the
offline archive/checker component scope; complete successful-response capture,
protected target transaction and reviewed fresh collection remain unavailable.
No Mini 12 contact, staging, SGX, new boot or artifact qualification occurred.

Prospective response-export implementation, 2026-10-04: a separate
[response client](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c)
now preserves the exact complete 4268-byte post-call object before success
summary/image processing, and preserves failure bytes with UNAUTHORITATIVE /
UNKNOWN / DO NOT RETRY semantics. The previously approved client stays unchanged.
The new source is 5,514 bytes, SHA-256
`919c2611e048e4a660ecae3542264adf5a5367d9b0fbe7fbceff2f45f90adedf`;
its two static i386 builds are byte-identical: 775,264 bytes, SHA-256
`2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835`.
Nine CPU-mock tests plus nine under UBSan passed; three differential cases
retained identical complete pre-call requests, device flags, ioctl number,
call ordering and one-call count. ELF/static and inert/refusal checks passed;
unchanged UAPI ABI evidence was reused. No module/image build was needed.
An independent offline review found unchecked cleanup close reporting after
raw write/sync failure; two failing fault subcases were preserved before the
small correction. Initial completed builds/logs remain alongside final evidence.

The archive accepts an optional declared producer pin, rejects a mismatch with
the supplied exact approval before creating an archive, and records only
identity consistency; live provenance stays UNKNOWN. Twenty-one archive/guard
tests passed after this interface addition. Existing sealed offline synthetic
artifacts still verify; historical real first-owner records remain consistent
and explicitly not fresh. The unchanged full-image validator suite was not rerun.

This new client is **offline technically qualified, not selected or approved**
by the prior exact-byte substitution decision. The bounded
[review request](response-export-client-substitution-review-request-20261004.json)
identifies the new bytes and unchanged dependencies; it issues no decision.
Evidence, review, interface/role mapping, fresh PRE01–PRE08 requirements and a
proposed history-preservation set are in
`/home/gama/sgx535-offline/phase8-response-export-20261004T092750Z/final/`.
Only client source/binary/forward-binding evidence changes. Existing module,
image, UAPI, frozen scene, boot binding and historical 70/70 remain applicable
in their original scopes. The exact new-client binding, available reviewed live
collector/executor, protected target paths and fresh observations are still
required; no restricted procedure was reconstructed. This closes the byte-export
gap in a separate offline implementation, not the live transaction. Gate B and
its whitelist remain unchanged; execution authorization false, SGX count zero,
triangle NOT ATTEMPTED. No target contact, staging, boot, HOLD or recovery action.
No files were staged or committed; untracked authority/evidence remain uncommitted.

PRE07 responsibility investigation, 2026-10-04:
[22-responsibility matrix](pre07-responsibility-matrix-20261004.json) separates
five complete reusable component capabilities, fifteen partial live bindings,
and two unavailable/restricted attribution responsibilities. A reviewed generic
`frozen_first_load_stage.copy_exclusive()` already supplies qualified-file
copy, inode/ownership/link checks, file/parent sync and destination readback;
these mechanisms are not missing algorithms. The passive capture API requires
a caller-supplied reviewed root program and cannot be substituted for the
unavailable one-shot program. The archive UUID/prefix/service tuple never
observes or authenticates the actual client/ioctl-to-kernel association.

The smallest irreducible PRE07 capability is **reviewed runtime association of
the single exact client ioctl and returned artifacts with attributable kernel
completion/HOLD evidence**. Protected target paths, stream capture and fresh
collector parameterization also depend on the unavailable current-context live
executor/source; they are not established by the file primitives. The contents
and success-report details of that unavailable layer remain UNKNOWN. PRE07
remains **BLOCKED**; no substitute collector or invocation wrapper was created.
A [pending exact client decision](response-export-client-substitution-maintainer-pending-20261004-314e2f3b.json)
now prepares only the new source/binary/UAPI forward binding, with decision,
authority and issuance timestamp null. It approves no staging, contact, guards,
execution, retry or unavailable procedure. Existing approvals remain unchanged.
No qualification/tests/builds/captures were repeated; only record consistency
checks. Gate B PASS and whitelist unchanged, execution false, hardware/SGX zero,
triangle NOT ATTEMPTED. Evidence and Git snapshots:
`/home/gama/sgx535-offline/phase8-pre07-responsibilities-20261004T101242Z/`.
No stage/commit; untracked authority and tooling remain uncommitted.

**Next boundary:** the legitimately available reviewed client/output and
invocation/preservation procedure, existing fresh pre-invocation identity,
ownership/health, unused one-shot, privilege/recovery and same-boot continuity
checks, and **separate execution permission**. Those checks have not passed by
virtue of this decision. Do not contact the Mini 12, stage, substitute, boot or
invoke on the strength of this record. Existing HOLD/no-retry/recovery rules,
Attempt05's consumed status and boot-loss evidence limits remain in force.

## Authoritative Phase 8 checkpoint — 2026-10-03

**Resume here.** This checkpoint supersedes historical current-state identities
and next-action instructions below. No opening-process search is to be repeated
in this session; its unavailability is confirmed. No target contact, recovery,
shutdown, build, staging, capture or test was performed for this handoff update.

| Item | Preserved checkpoint |
| --- | --- |
| Candidate Build ID | `314e2f3b37195dc56df7c57dd938d78377a5ea8e` |
| Module | 244,960 bytes; SHA-256 `4141a07c1f2d2ffc9ab2770b7b4cf654f68c396275dad15f0ddc68192529d8e6` |
| Diagnostic image | 50,805,231 bytes; SHA-256 `fe64b3dcfe74b78a6d96edcd4fd7c118c3901fcff8631afec6c14647da292e3c` |
| Last observed boot | `89fc7306-6dac-4ab9-af62-360b7152ef22` |
| Selected action | `MINI12-SGX535-REV121-FROZEN-32x32-SEQ1` |
| First-owner capture | **70/70 PASS** |
| Candidate note / module | OBSERVED / Live |
| Ordered hook trace | OBSERVED |
| Health-classifier fault | None in the preserved capture |
| Operator-observation receipt | Valid under the prospective successor procedure; no photograph supplied |
| Gate B / whitelist | **BLOCKED only because the independent opening mechanism is unavailable** / `[]` |
| SGX invocations on this candidate | **0** |
| Triangle | **BLOCKED BEFORE EXECUTION** |

### Exact preserved evidence

All relative paths below refer to repository files; raw records remain unchanged.

- [First-owner verdict](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/candidate-first-owner/verdict.json), [decoded records](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/candidate-first-owner/decoded-records.json), [raw stdout](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/candidate-first-owner/stdout.txt), [stderr](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/candidate-first-owner/stderr.txt), [command/timing](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/candidate-first-owner/command.json).
- [Accepted menu receipt](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/prospective-operator-observation/accepted-menu-receipt.json), [candidate witness](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/prospective-operator-observation/candidate-witness.json), [successor consistency verdict](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/prospective-operator-observation/first-owner-consistency-verdict.json), [opening-boundary receipt](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/prospective-operator-observation/opening-boundary.json).
- [Staging write receipts](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/stage/stdout.txt), [independent poststage records](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/poststage/decoded-records.json), [poststage verdict](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/poststage/verdict.json). Prepared bundle: `/home/gama/sgx535-offline/frozen-staging-314e2f3b-20261002T234125Z/preparation.json`.

### Direct continuation and boot-loss rule

If this boot is preserved and the legitimate independent mechanism becomes
available, resume at **EXACT BOOT/BUILD/ACTION OPENING DECISION**, then complete
only the existing invocation-specific fresh guards and protected client/output
prerequisites through the reviewed procedure. A prior passive PASS is not a
fresh one-shot/ownership/health/privilege/continuity check. Target client/output
destinations and the unavailable invocation procedure must not be invented.
Operator consent is established; it is not the independent opening decision.

After an affirmative applicable decision and passing prerequisites:
**ONE FROZEN INVOCATION → PRESERVE ORIGINAL 4096-BYTE READBACK → FULL-IMAGE
VALIDATION → TRIANGLE CLASSIFICATION**. Preserve raw client/service/event/status
and attributable kernel evidence first. Analyze only a copy with
`python3 -B tools/psb-dri-re/frozen_triangle_readback.py <copy>`:
32×32 linear ARGB8888, stride 128, vertices (8,8), (24,8), (8,24), all 1024
positions/values and zero background. Sampling/edge convention remains
unqualified. Establish a triangle only from compatible full-image evidence
jointly with attributable TA, end-render/raster and 3D-memory-free completion;
exit status, event bookkeeping or checksums alone are insufficient.

If this boot cannot be preserved, unchanged candidate offline qualification,
client/UAPI/workload/ABI evidence, staging receipts, preparation bundle,
successor procedure/tests and validator remain reusable. The 70/70 capture and
operator receipts remain historical proof for this boot only. They do not
establish a new boot's loaded identity, first-owner trace, ownership/health,
privilege, one-shot history or recovery readiness. Fresh boot-specific evidence
and an applicable decision must then bind the new boot; staged bytes/configuration
are not assumed unchanged solely from the old receipts. Reboot alone does not
require rebuilding or requalifying unchanged bytes.

Preserve the current DIAGNOSTIC boot. Attempt05 remains consumed. If an ioctl
may have occurred and failure, ambiguity or HOLD follows: preserve complete raw
evidence, stop, never retry, and use only the existing reviewed operator machine
boundary when needed; no improvised reset/unbind/module replacement/hot recovery.
No unobserved TA fire, completion, rasterization or readback is successful.

Offline focused review, 2026-10-04: two demonstrated tooling defects were fixed.
The [receipt validator](../../tools/psb-dri-re/frozen_first_owner_operator_receipt.py)
now rejects a candidate boot equal to the verified STOCK staging boot, while
allowing an intervening STOCK boot. The [readback validator](../../tools/psb-dri-re/frozen_triangle_readback.py)
rejects stdout aliases of its pinned input (including hard links) before report
writes and avoids aliased-stderr diagnostics. It cannot prevent caller-side
truncation before process startup. Five new regression tests demonstrated the
failures before fixes; 27 receipt/session and 18 readback tests passed afterward.
One offline check of the preserved actual records retained the accepted
first-owner classification. No C, client, module/image or raw hardware evidence
changed; no rebuild, requalification, target contact or invocation occurred.
Commands/results and diffs are in
`/home/gama/sgx535-offline/phase8-focused-review-20261004T034224Z/`;
receipt result files explicitly identify transcribed observed tool output,
while validator test stdout/stderr are captured directly. Historical reviewed
source pins remain historical; they are not identity claims for these edited tools.

Second focused round, 2026-10-04: CPU-only fault injection demonstrated that the
selected client's single file write loses remaining bytes on a recoverable short
write or EINTR and can print misleading "Success" on short/zero writes. Mocked
failed ioctls stopped after one call but omitted raw response bytes and explicit
UNKNOWN/no-retry labeling. No historical hardware loss or wrapper retry is inferred.
The selected source, binary and qualification pins remain unchanged. A separate
[prospective, unqualified source](../../tools/psb-dri-re/frozen_triangle_one_shot_prospective.c)
completes recoverable file writes, checks file sync/close, retains partial files
on failure, and reports failed syscalls as UNKNOWN with unauthenticated raw bytes.
[Preservation tests](../../tools/psb-dri-re/test_frozen_triangle_preservation.py)
and [failure-semantics tests](../../tools/psb-dri-re/test_frozen_triangle_failure_semantics.py)
demonstrated failures before changes; afterward 10 and 5 tests passed, respectively.
Only native CPU/mock harnesses were compiled; no selected client/module/image
was rebuilt or requalified. File sync does not establish directory durability;
diagnostic streams remain best effort. Independent review found no blocker.
Commands, raw logs, synthetic fixtures and source diff are preserved in
`/home/gama/sgx535-offline/phase8-evidence-round2-20261004T040431Z/`.
The selected client does **not** contain these prospective fixes. Do not stage,
invoke or substitute the prospective client using the old qualification: normal
client qualification and applicable exact-action review must cover it first.
No raw hardware evidence or first-owner tooling changed, so the preserved 70/70
checkpoint remains valid for its observed boot. No target contact or SGX occurred.
Completion-ledger guards and the offline image classifier had no new demonstrated
defect; independent hardware attribution remains required. The unavailable
invocation/copy/correlation layer was not reconstructed or claimed audited.

Client-promotion round, 2026-10-04: the unchanged prospective source passed
normal **offline technical qualification** using the retained Clang/LLD i386
sysroot/qemu workflow. Source SHA-256 is
`8b277dcb8c4e00a4eea78774b2bf8529c9396bfbbf05597be05b75cff2bd2b49`.
Two static ELF32/i386 builds were byte-identical: 774,336 bytes, SHA-256
`a854d4f719b92cfe31a9ad33adc15d0f881d397c163ad6c34b5518d4a21da2e9`;
no ELF Build ID was emitted. ABI size 4268, offsets 16/44/172 and ioctl
`0xd0ac6440` passed; default/wrong-flag qemu traces made no device opens/ioctls.
Nine paired CPU-mock differential cases passed, with identical entire pre-call
requests in all six ioctl-reaching pairs. Only post-call reporting/preservation
and qualification comments differ; no request or GPU-execution change was found.
The actual module graph excludes this client and the image verifier excludes
clients, so module/image identity, ABI qualification, staging and historical
70/70 first-owner evidence remain reusable; no new boot follows from this change.
Full records and the new binary are under
`/home/gama/sgx535-offline/phase8-client-promotion-20261004T042709Z/`;
the artifact is `client-build-01/frozen-triangle-one-shot-prospective-i386`.
**B. QUALIFICATION INCOMPLETE — applicable exact-action review covering
substitution of the prospective client has not been established.**
`promotion-decision.json` records that boundary and the dependency/pin impact.
The selected old client/source and forward selector pins remain unchanged;
historical qualification is not reassigned to the new artifact. Do not stage,
substitute or invoke it until the existing review requirement is legitimately
satisfied. Future client selection/staging must bind its own path/hash/size;
module/image restaging is unnecessary solely for this userspace change. No
opening search, target contact, SGX execution or recovery occurred in this round.

The [request-only substitution review package](prospective-client-substitution-review-request.json)
indexes the existing identities, qualification, regressions and dependency evidence.
It supplies no approval and explicitly stops before the unavailable target client
staging/rollback/invocation procedure; it is not execution readiness.

Read [diagnostic-gate-b-technical-audit.md](diagnostic-gate-b-technical-audit.md) first, then use the evidence index below. Do not start by running tests, rebuilding or contacting the Mini 12. Much of that work is already done.

## Where we are

The hardware is an owned Dell Inspiron Mini 12 / Inspiron 1210, Intel Poulsbo PCI `0000:00:02.0`, PowerVR SGX535 rev121. Retained discovery: `CORE_ID=0x01130000`, `CORE_REVISION=0x00010201`. PCI vendor/device `8086:8108`, subsystem `1028:02b1`, IRQ 16.

Phase 7 is COMPLETE WITH UNRESOLVED EVIDENCE BOUNDARY. Phase 8 is active. The long-term goal is a controlled SGX535-generated triangle, with evidence connecting the workload, execution, raster pixels and eventually the physical LCD. A CPU-drawn image does not count. The immediate experiment is the frozen **off-screen** diagnostic workload. Physical-LCD handoff is not authorized and is not implemented by this checkpoint.

```text
GATE B: BLOCKED
WHITELIST: []
FIRST TRIANGLE: NOT ESTABLISHED
VISIBLE SGX535 TRIANGLE ON PHYSICAL DISPLAY: NOT ESTABLISHED
```

Last observed target boot: `55bb90c8-fffe-4993-b656-7aa7a7b9b5e8`. The diagnostic module was Live, with normal PCI/DRM/framebuffer/IRQ16 ownership. One bounded passive capture passed **66/66 guards**. Physical display/userspace were operator-reported normal; slimski/Xorg were captured running. Automatic capture uptime was **213.16–225.40 seconds**; operator-reported time to userspace was about **120 seconds**. These measure different events.

No client was staged or invoked, and no SGX ioctl/workload, TA/raster submission or color readback occurred through this diagnostic cycle's recorded actions. No later target operation occurred in the offline audits. The target's state now is not freshly observed. Preserve the last observed diagnostic boot; do not reboot just to renew already sufficient evidence.

A normally permitted, completed exact-build/action opening decision is missing. The earlier Gate B/artifact review was interrupted by a reported platform restriction. Gate B was never conclusively opened for this diagnostic boot. Authorization or a passive qualification PASS does not fill that gap.

## Established facts and limits

| Class | Conclusion |
| --- | --- |
| OBSERVED, retained target evidence | Correct diagnostic Build-ID note, boot/kernel identity, ordered first-load hook trace and PCI/DRM/fb/IRQ16 ownership passed in the current 66/66 capture. |
| OBSERVED, operator report | Physical display/userspace normal; local sudo succeeded at that capture; userspace reached in about 120 seconds. Future remote sudo readiness is not established by that report. |
| Established offline | Diagnostic module ABI: ELF32/i386, exact vermagic, `module_layout=0xb84efb99`, 232/232 imports, zero missing/mismatched CRCs. Image independent verification PASS and repeat construction byte-identical. |
| Established offline | Diagnostic instrumentation reports existing values only. Frozen workload, UAPI, addresses, relocations, callback order, sequencing, timeout/fault/HOLD and one-shot semantics are unchanged. |
| OBSERVED, earlier same-image cycle | Diagnostic first ownership and a distinct STOCK recovery boot were captured. STOCK recovery had 39/39 root guards. This is not a guarantee of recovery from a future GPU hang. |
| SOURCE-PROVEN | The first-load path avoids the original driver's defective hot-removal dependency. The derivative pairs its own IRQ teardown. Original hot removal/restoration remains BLOCKED. |
| INFERRED / UNKNOWN | Stock loading is consistent with real-root eudev/kmod modalias coldplug. The exact historical requester remains UNKNOWN; it is not a new blocker to the observed first-owner path. |
| UNKNOWN | Full architectural source/publication semantics remain unresolved. Existing scoped assumptions are not promoted to facts by ABI/first-owner success. `--complete` remains `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`. |
| UNKNOWN | Attempt05 failing callback and physical TA fire. No triangle or successful GPU execution is inferred from initialization, callback return, phase or authorization. |

The exact current trace was:

```text
SGX535-FIRSTLOAD BEGIN
SGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE
SGX535-FIRSTLOAD PASS: derivative first owner; boot may continue
```

Captured kernel health was `PASS WITH BOUNDED STOCK DIAGNOSTICS`, no rejected faults, taint **12289**. The bounded ACPI/backlight messages and the unsigned `drm` loader notice are documented operational handling, not permission to ignore arbitrary warnings or faults. Changed/repeated/fault-bearing variants must still reject.

## Attempt05: do not retry it

[Attempt05 result](../hardware-evidence/MINI12-20261002T041334Z-SGX-ATTEMPT-05/RESULT.md) belongs to corrected EXPERIMENTAL boot `203a5b9b-5a90-4fd3-8001-3c92cdefeddd`, not the later diagnostic boot.

Its ioctl returned a UAPI response:

```text
operation_errno = -1 (SGX535_FROZEN_BAD_REQUEST)
outcome = 3 (HELD)
phase = 11 (HELD_AFTER_FAILURE)
events = 0
color_observed = 0
```

Owner construction, including the PDS BO, CPU publication and translation publication completed. The first PDS allocation was already fixed by the private-GPU-VA allocator correction. Attempt05's failure is later; do not diagnose the old EBUSY again.

```text
ATTEMPT05: HELD_AFTER_FAILURE
FAILING CALLBACK: UNKNOWN
TA FIRE: UNKNOWN/POSSIBLE
TA COMPLETION: NOT OBSERVED
RASTER SUBMITTED: NO
RASTER COMPLETION: NOT OBSERVED
COLOR READBACK: NONE
ATTEMPT05 ONE-SHOT: CONSUMED / DO NOT RETRY
```

The old module was subsequently left across the reviewed machine boundary; it is not the last observed diagnostic module. The diagnostic boot has no invocation in the preserved cycle records. That is not permission to reset or assume its one-shot remains available without the required immediate checks.

The narrowed normal control-flow interval is:

```text
last established: CPU + translation publication
  3 DEVICE_MAINTAIN
  4 INIT_WRITES
  5 XHW_INIT
  6 TA_INFO
  7 SCENE_INFO
  8 TA_LOAD
  9 SCENE_VALIDATE
 10 USE_RESERVE
 11 USE_PROGRAM
 12 STATUS_BASELINE
 13 TA_SCHEDULE
 14 TA_FIRE
 20 STATUS_SAMPLE
 21 STATUS_PROCESS
 22 TIMEOUT (sample-budget exhaustion)
```

Inline session/bootstrap/plan checks occur between these callbacks; the audit lists them in order. `unsafe_possible=1` is set before DEVICE_MAINTAIN. A preparation failure can therefore enter phase 11 **before TA_FIRE**. The guards/plans between successful preparation and TA_SCHEDULE return BAD_REQUEST directly; through the normal path they do not explain phase 11.

Accepted TA completion records event bit 1 before raster begins; later failure does not clear it. Zero events therefore excludes the normal raster preparation/reset/schedule/fire and USE_RELEASE paths. It proves no attributable TA completion was recorded, not that TA never fired. Do not choose a failing stage from Attempt05's outer `-1`.

## What the diagnostic can tell us

The diagnostic fields are `stage_reached`, `failure_stage`, `failure_source`, `raw_result`, `observation_stage`, `observation_load_flags`, `observation_status1`, `observation_status2`, `observation_initend`. They are the latest available tuple, not a complete callback history.

Failure-source values: **0 NONE, 1 BACKEND_CALLBACK, 2 STATUS_SOURCE, 3 SERVICE_CHECK, 4 TIMEOUT**. Backend and status-source returns are preserved in `raw_result`; the outer operation can still collapse them to `-1`.

Stage 8 snapshots existing load flags/status2/initend; stage 12 snapshots the existing baseline; stage 20 snapshots the existing status-source return and status words. Stage 21 processes status. Stage 22 specifically labels sample-loop exhaustion; it does not identify every time-related failure inside a backend/status source.

Multiple inline checks share the previous callback's stage. Bootstrap `last_event` and status-source `exclusive_owned` are not diagnostic fields. A stage-localized error is not necessarily an exact helper/instruction diagnosis. Reaching stage 20 means TA_FIRE's callback returned zero, not that TA completed or physically fired.

Two reporting edge cases were tested offline with the actual service/contract and existing mock fixture:

| Case | Outer result | Diagnostic |
| --- | --- | --- |
| Recognized status fault bit 28 | -1, phase 11, events 0 | reached 21, failure stage 0, source 0, raw 0, status1 `0x10000000` |
| Status bit 24, exclusive ownership rejected | -1, phase 11, events 0 | reached 21, failure stage 0, source 3, raw -1, status1 `0x01000000` |

Both made 14 mocked backend calls and one sample, with no raster/reset/release callback. These were not hardware experiments. **Zero failure stage/source or raw zero is not success.** Interpret the tuple with phase, events and status words.

The unchanged client/UAPI exposes errno, outcome, phase, events and color results, not this internal tuple. The already recorded diagnostic implementation reports it in the kernel HOLD log. A future permitted invocation needs the existing attributable kernel-log capture as well as client output. Do not reopen the restricted kernel-entry item to reproduce that material.

One bounded invocation can distinguish callback stages, status-source failure, the status-processing family and sample exhaustion **if the report is emitted and preserved**. It cannot promise a root cause inside every callback/inline helper, or a report surviving a hang. No extra ioctl, GPU read or diagnostic GPU transaction is needed just to obtain these fields.

## Exact identities and paths

All hashes below are recorded qualification identities, not fresh rehashes made for this handoff.

**Diagnostic module**

- Local path: `/home/gama/sgx535-offline/private-gpu-va-diagnostic-20261002/build-02/gma500_gfx.ko`
- SHA-256: `5e106cf2511acf3c5aa7f54cf6bd42b3ca368ab91dbdf45af547e1fa676ef2a0`
- Size: **245,000 bytes**
- Build ID: `594030ac153ce3c6c7dec748025142c92d90dd0c`
- Captured loaded note SHA-256: `a74fb4f5b624980dfb717cb4c5c6fc400da4a9aa0dde264cd2365b5c7c3e4f8e`
- ELF32 little-endian Intel 80386; expected architecture `i686`
- Vermagic: `5.10.240-antix.1-486-smp SMP mod_unload modversions 486`
- `module_layout=0xb84efb99`; 232/232 imports, zero missing/mismatched CRCs
- Module name `gma500_gfx`, PCI driver `/sys/bus/pci/drivers/gma500`. Loaded-note provenance is operational identity, not full memory attestation.

**Diagnostic image and GRUB**

- Local image: `/home/gama/sgx535-offline/private-gpu-va-diagnostic-20261002/diagnostic-image-01/build-03/initrd.img-sgx535-firstload-diagnostic-01`
- Size: **50,805,273 bytes**; SHA-256: `376da01e11c3fd7079f348c0b10c1ea4d09c58dd917fb8be4dda52a7d805abae`
- Repeat: same directory family, `build-04/initrd.img-sgx535-firstload-diagnostic-01`, byte-identical
- Existing target image: `/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-diagnostic-01`
- Entry title: `EXPERIMENTAL SGX535 rev121 DIAGNOSTIC FIRST-LOAD ONLY (no SGX)`
- Entry ID: `sgx535-rev121-firstload-diagnostic-01`
- Entry-only proposal: [diagnostic-entry.proposed](../hardware-evidence/MINI12-20261002T054751Z-DIAGNOSTIC-STAGING-01/diagnostic-entry.proposed), **1,002 bytes**, SHA-256 `892fbbff2c8126bd6d330b59a3bab7bd3884eb53b82d1a2c6fd672522654deaa`
- Staged `/boot/grub/custom.cfg`: **3,042 bytes**, SHA-256 `a2d2b6bd33868deefe64e5e1cb2cd9ecedbfd1fe0e78370af983a92a2ec655c5`
- Preserved `/boot/grub/custom.cfg.pre-diagnostic-01`: **2,040 bytes**, SHA-256 `269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8`; exact prefix of the staged config
- Volatile hook log: `/run/initramfs/sgx535-first-load.log`
- The entry is non-saving; it uses the stock kernel and unchanged stock command line. Do **not** require an experimental token in `/proc/cmdline`: an earlier guard incorrectly did so.

**Frozen client and action**

- Local binary: [docs/phase8/artifacts/frozen-triangle-one-shot-i386](artifacts/frozen-triangle-one-shot-i386)
- SHA-256: `758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf`; **771,320 bytes**, static ELF32/i386
- UAPI identity: source header SHA-256 `04dd2080deeb0056a366546fe27faddbdeff6c1657a2471c96e39749dc080420`; unchanged by diagnostics
- Proposed exact action name, **not currently whitelisted**: `MINI12-SGX535-REV121-FROZEN-32x32-SEQ1`
- Fixed request `{1,1,0,0}`, sequence 1; at most one TA fire and one raster fire; existing `5×HZ` and **300,000-sample** bounds
- Historical Attempt05 client path: `/root/sgx535-frozen-seq1-203a5b9b/frozen-triangle-one-shot-i386`. Do not reuse that consumed invocation or treat its staging as staging for the diagnostic boot. No diagnostic-boot client destination is established by this audit; use only the normally permitted existing exclusive-staging procedure if authorized.
- Historical ioctl ABI identifier: `0xd0ac6440`, UAPI size **4,268**, fixed request prefix **16 bytes**. These are identities, not instructions to invoke it.

**STOCK and ABI reference**

- Kernel `/boot/vmlinuz-5.10.240-antix.1-486-smp`: **5,984,416 bytes**, SHA-256 `cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438`
- STOCK initramfs `/boot/initrd.img-5.10.240-antix.1-486-smp`: **50,863,580 bytes**, SHA-256 `f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341`
- Installed original `/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko`: **205,964 bytes**, SHA-256 `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb`; original Build ID `d8dcb4d38b774ad64799d5e13aaedede069371f3`
- `/boot/grub/grub.cfg`: **10,438 bytes**, SHA-256 `396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f`
- `/boot/grub/grubenv`: **1,024 bytes**, SHA-256 `72c291233d508c8ed06305f0bf9d33130ea206bfaf67873dbe77413f77d19927`
- Saved STOCK entry: `gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1`
- STOCK title: `antiX-26 Stephen Kapos, 5.10.240-antix.1-486-smp` (**no suffix**)
- Captured command line: `BOOT_IMAGE=/boot/vmlinuz-5.10.240-antix.1-486-smp root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro quiet selinux=0`
- Qualified target symbol table: [capture-08 artifacts/build_Module_symvers](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/build_Module_symvers), SHA-256 `faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca`
- Captured target config: [capture-08 artifacts/boot-config](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/boot-config), SHA-256 `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9`
- Exact antiX source archive SHA-256 `e5c5d7c6bdcc7a845589e5956cbf50a8d6198232df017f0398a2902371042e1d`
- Qualified source `/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp`; successor output `/home/gama/sgx535-offline/antix-kbuild-successor-20260930/output`
- Qualified build roles: `ARCH=x86`, GCC 14.2.0 `i686-linux-gnu`, GNU i686 binutils 2.44, native AArch64 `/usr/bin/gcc-14` HOSTCC, qualified explicit sysroot, documented cross prefix, `LOCALVERSION=-486-smp`. No rebuild is currently justified. If a later source change needs one, use the retained plan, not an improvised toolchain.

For a later authorized contact, the recorded endpoint was `gama@192.168.18.90:22`; pinned known-hosts file [MINI12-20260927-H0/ssh_known_hosts](../hardware-evidence/MINI12-20260927-H0/ssh_known_hosts), local key path `/home/gama/.ssh/id_ed25519_sgx535_h0`. Do not print key contents, disable host-key checking, assume the endpoint is still reachable, or treat these identifiers as contact authorization.

## Minimum next-session checks

The next evidence-producing action is **only the existing bounded read-only same-boot portion**, if normally permitted and separately authorized. It must expect boot `55bb90c8-fffe-4993-b656-7aa7a7b9b5e8`. STOP on mismatch; do not silently substitute a new boot or start another cycle.

Before any later invocation, the existing procedure needs:

1. Expected boot, kernel/architecture, loaded diagnostic note and Live state. Use loaded identity, not installed original `modinfo`.
2. Current PCI/DRM/framebuffer/VT/IRQ16 ownership and acceptable health/taint. Earlier capture is not proof of unchanged state.
3. Unused one-shot for this boot from the invocation/evidence history. Do not query availability by an ioctl, reset it or infer unused state from absence of an error.
4. Exact frozen client, protected exclusive-create/no-follow destination, destination readback, ownership/type/link checks and absent output paths under the existing procedure. A hash alone is insufficient. This is staging, not read-only contact.
5. Working noninteractive privilege in this boot; if local authentication is needed, ask the operator to run `sudo -v` locally, never for a password.
6. Completed normally permitted exact-build/action decision, explicit applicable execution authorization, operator/recovery availability and unchanged bounds/HOLD rules. Whitelist must not be populated by this handoff.
7. Same-boot continuity across preparation and invocation.

Do not repeat the entire passive capture or its historical qualification-clock checks to create activity. If the normally permitted existing read-only procedure is unavailable, report that exact boundary rather than deriving a substitute wrapper.

## Stale requirements and recovery

The selected architecture is derivative first-owner at boot, not hot replacement. Original hot removal, PCI unbind, VT/display teardown to enable replacement and hot restoration are not prerequisites for this path; they remain prohibited. The derivative retains stock KMS/fbdev functionality. Normal userspace/Xorg in the qualified first-load path does not require inventing a new display shutdown.

Old missing-first-owner reports are historical, not contradictions of the current 66/66 ordered trace. An experimental command-line marker is not expected. The old combined **120-second** deadline is superseded. Existing v2 limits are **600-second boot watch**, **1,200-second automatic passive capture ceiling**, **40-second connection / 35-second child**. Those are not SGX workload timeout replacements. No authorization policy requirement was removed by the latest audit.

The recovery design is an operator-controlled machine boundary followed by manual normal STOCK selection, using unchanged stock kernel/initramfs/default. There is no hot recovery. A previous same-image diagnostic boot `69847808-be83-4a28-88b1-96688f0b0d66` returned to STOCK boot `bce184cb-f5d1-40a2-8591-bd800f4679cc`, with 39/39 root guards and normal operator-reported display/userspace. Recovery after a future GPU failure remains unproven.

STOP/HOLD on a failed guard, identity drift, unexpected owner, new fault, timeout, possible ioctl issuance, ambiguity or lost attribution. Once an ioctl may have occurred, no retry. Do not unload/reload/reset a one-shot, hot-replace, unbind, force removal or improvise a reset. Preserve raw output before analysis. Physical action requires the operator and the applicable reviewed recovery authorization; this document does not authorize reboot or reset.

## Authorization and unavailable material

This handoff authorizes no target action. Earlier permissions were bounded by exact build/boot/cycle/action and prerequisites; do not turn them into blanket permission.

| Boundary | Current meaning |
| --- | --- |
| Offline records/source/tests | Continue independent permitted work if it answers a new question. No repeated qualification without relevant changes. |
| Read-only target contact | Smallest next proposed action; only under applicable explicit authorization and a normally permitted reviewed procedure. No target contact was performed by the audits/handoff. |
| Client/image/GRUB staging | State-changing and separate. Diagnostic image/entry are already staged; no need to restage. Diagnostic-boot client staging is not recorded. |
| Gate B / exact action approval | Incomplete for this boot. Keep BLOCKED and `[]` until the required condition is established through permitted observed evidence. |
| SGX ioctl/workload | Not currently permitted: no completed opening decision or whitelist. Prior conditional operator permission cannot override failed prerequisites or restrictions. |
| Recovery | Reviewed operator machine-boundary path only when specifically needed and authorized; no automatic reboot just to progress. |
| LCD handoff / expanded GPU work | Unauthorized. No arbitrary MMIO, extra submissions, guessed packets/addresses, altered workload or automatic retry. |

The particular previously restricted **Gate B/artifact/exact invocation-wrapper/kernel-entry review** remains unavailable. The returned records did not supply a rejection reason. Do not retry it, reopen/reconstruct its material, substitute tools/encodings/sources, or derive an alternate path around it. The independent fixed-service/contract/client-interface analysis and passive summary records were permitted. Their conclusions do not reconstruct a restricted implementation or replace the unavailable opening decision.

A restriction is not technical evidence that the GPU/driver is wrong. Do not demand an explanation of it as a GPU prerequisite. If a permitted operation independently encounters a restriction, record that item as unavailable and continue independent allowed work. If the required exact-action decision cannot be completed normally, STOP at that boundary; do not infer PASS.

## Evidence index and completed verification

Read these for their specific facts; do not replay their operations:

- [Newest technical audit](diagnostic-gate-b-technical-audit.md): predicate table, ordered failure map, immediate checks, diagnostic limits and continuation checkpoint.
- [Current diagnostic first-owner result](../hardware-evidence/MINI12-20261002T063600Z-DIAGNOSTIC-FIRSTOWNER-02/RESULT.md), [decoded records](../hardware-evidence/MINI12-20261002T063600Z-DIAGNOSTIC-FIRSTOWNER-02/passive/decoded-records.json), [manifest](../hardware-evidence/MINI12-20261002T063600Z-DIAGNOSTIC-FIRSTOWNER-02/SHA256-MANIFEST.json). Current passive stdout recorded SHA-256 `48c2ffedd59437dba51a9f9c6ff9d130c27f64ac21bcdb53bad1753a2b3d81a5`.
- [Diagnostic staging and earlier full same-image recovery](../hardware-evidence/MINI12-20261002T054751Z-DIAGNOSTIC-STAGING-01/RESULT.md): STOCK 36/36; staging readiness 18/18; post-stage 38/38; previous diagnostic 66/66; STOCK recovery 39/39. Raw recovery STOP verdict remains preserved.
- [Offline module/image qualification record](../hardware-evidence/MINI12-20261002T053407Z-STOCK-POSTATTEMPT05-PREFLIGHT-01/diagnostic-image-offline-recheck.json). Its staging-not-started text is historical; the later staging/current-capture records supersede that state.
- [Attempt05 result](../hardware-evidence/MINI12-20261002T041334Z-SGX-ATTEMPT-05/RESULT.md). Its final target-location sentence describes that attempt's historical endpoint, not the later diagnostic boot.
- [New two-case mock probe](artifacts/diagnostic-observability-audit-20261002/probe.c), [commands/results](artifacts/diagnostic-observability-audit-20261002/commands-results.json), [stdout](artifacts/diagnostic-observability-audit-20261002/run.stdout); empty compile/run stderr is preserved beside them. Original work directory `/home/gama/sgx535-offline/diagnostic-observability-audit-20261002T072101Z/`.
- [Capture v2](../../tools/psb-dri-re/frozen_first_load_capture_v2.py) and [tests](../../tools/psb-dri-re/test_frozen_first_load_capture_v2.py). The recovery receipt bug was `kernel` versus `kernel_release`; both captured values agreed. The offline fix accepts either, rejects conflicts, preserves other guards and never rewrites raw records. Verification `/home/gama/sgx535-offline/capture-format-fix-20261002T064822Z/test-results.json`.
- [Fixed service](../../tools/psb-dri-re/frozen_fixed_service.c), [diagnostic definitions](../../tools/psb-dri-re/frozen_fixed_service.h), [contract](../../tools/psb-dri-re/frozen_kernel_contract.c): independent permitted source map; do not reopen restricted kernel-entry/wrapper material through them.
- [Existing client artifact record](artifacts/frozen-triangle-one-shot-i386.md): static i386 ABI, exact artifact identity and offline build history. Its original “target execution untested” statement predates Attempt05.
- [Current checkpoint audit](post-attempt03-checkpoint-audit.md) points to the newest audit. [Older handoff](CHATGPT-HANDOFF-POST-ATTEMPT03.md), [Gate B review](fixed-one-shot-gate-b-review.md), [first-load procedure](first-load-staging-boot-recovery-procedure.md), [v2 timing redesign](first-load-capture-timing-redesign.md) provide history/policy, not a new live PASS. Respect the restriction on the particular review item; a path in this index is not permission to reopen it.

Recorded diagnostic qualification: **330 repository tests PASS, 1 existing skip**, fixed-service UBSan PASS, expected ABI/import checks PASS, image verifier PASS and deterministic image reconstruction byte-identical. The full suite was not rerun for the latest audit.

Capture-format fix: defect reproduced first; initial red run had four failing subtests and one error; focused **19/19 PASS**, combined v1/v2 **30/30 PASS**, diff check PASS. Fifty-five protected historical evidence files were unchanged in that fix. No target recapture.

Latest observability audit: **two new mocked edge cases PASS under UBSan**, warnings as errors, compile/run exit 0, empty stderr. No full harness repeat. Seven earlier audit consistency assertions PASS; latest audit **9/9 local links resolve**, whitespace/diff checks PASS. These are separate checks, not additions to the 330-test count.

Recorded frozen dry-run SHA-256 remains `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`. `--complete` intentionally refuses with `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.

**Do not redundantly rerun** these suites, probe, CRC checker, image verifier, repeat construction, dry runs or artifact hashes without a relevant source/artifact change or observed drift. A documentation-only continuation needs path/consistency/diff checks, not a GPU qualification rerun.

Do not redo CORE_ID/revision/rev121, PDS/0x07/nine-DWORD/source-envelope archaeology, scene/relocation/backing reconstruction, static client construction, toolchain qualification, kernel preparation/header equivalence, patch integration, previous candidate builds, Attempt03/capture-08, Attempt04 EBUSY diagnosis, the private-VA fix, first-binding/lifecycle archaeology or completed captures. Preserve old rejected artifacts and consumed attempts. Do not stage, commit or push.

## Working tree to preserve

The tree is deliberately dirty. The snapshot below was taken before this handoff was added; every entry is preexisting. Do not reset, clean, overwrite, stage or silently fold these changes into unrelated work. Untracked evidence directories are not junk. The prior offline tasks changed capture-format tooling/tests, then the technical audit and its six-file mock evidence bundle. Production diagnostic/private-VA/health changes also predate this handoff.

```text
 M docs/phase8/CHATGPT-HANDOFF-POST-ATTEMPT03.md
 M docs/phase8/fixed-one-shot-gate-b-review.md
 M docs/phase8/post-attempt03-checkpoint-audit.md
 M kernel/sgx535_frozen/gma500_bo_owner.c
 M kernel/sgx535_frozen/gma500_fixed_entry.c
 M kernel/sgx535_frozen/gma500_fixed_entry.h
 M tools/psb-dri-re/frozen_fixed_service.c
 M tools/psb-dri-re/frozen_fixed_service.h
 M tools/psb-dri-re/frozen_kernel_health.py
 M tools/psb-dri-re/test_frozen_fixed_service.c
 M tools/psb-dri-re/test_frozen_kernel_health.py
?? docs/hardware-evidence/MINI12-20261001T223542Z-FIRSTLOAD-CYCLE-03/
?? docs/hardware-evidence/MINI12-20261001T232417Z-CYCLE04-STOCK-PREPARATION/
?? docs/hardware-evidence/MINI12-20261001T234842Z-FIRSTLOAD-CYCLE-04/
?? docs/hardware-evidence/MINI12-20261002T001828Z-FIRSTLOAD-CYCLE-05/
?? docs/hardware-evidence/MINI12-20261002T005013Z-FIRSTLOAD-CYCLE-06/
?? docs/hardware-evidence/MINI12-20261002T011623Z-TRIANGLE-ATTEMPT-04/
?? docs/hardware-evidence/MINI12-20261002T013129Z-POSTATTEMPT04-SAMEBOOT-READONLY/
?? docs/hardware-evidence/MINI12-20261002T013728Z-ATTEMPT04-CORRECTED-WRAPPER/
?? docs/hardware-evidence/MINI12-20261002T032717Z-CORRECTED-CYCLE-PREFLIGHT/
?? docs/hardware-evidence/MINI12-20261002T033110Z-CORRECTED-STOCK-STAGE/
?? docs/hardware-evidence/MINI12-20261002T034421Z-CORRECTED-STAGE-PROPOSAL/
?? docs/hardware-evidence/MINI12-20261002T040615Z-CORRECTED-FIRSTLOAD-01/
?? docs/hardware-evidence/MINI12-20261002T041334Z-SGX-ATTEMPT-05/
?? docs/hardware-evidence/MINI12-20261002T053407Z-STOCK-POSTATTEMPT05-PREFLIGHT-01/
?? docs/hardware-evidence/MINI12-20261002T054751Z-DIAGNOSTIC-STAGING-01/
?? docs/hardware-evidence/MINI12-20261002T063600Z-DIAGNOSTIC-FIRSTOWNER-02/
?? docs/phase8/artifacts/attempt04-ebusy-offline-20261002/
?? docs/phase8/artifacts/diagnostic-observability-audit-20261002/
?? docs/phase8/artifacts/private-gpu-va-fix-20261002/
?? docs/phase8/artifacts/visible-triangle-readiness-20261001T222342Z/
?? docs/phase8/cycle04-first-load-procedure.json
?? docs/phase8/cycle04-preparation.md
?? docs/phase8/diagnostic-gate-b-technical-audit.md
?? docs/phase8/first-load-capture-timing-redesign.md
?? docs/phase8/first-load-cycle-03-result.md
?? docs/phase8/first-load-cycle-04-result.md
?? docs/phase8/visible-triangle-readiness.md
?? tools/psb-dri-re/frozen_first_load_capture_v2.py
?? tools/psb-dri-re/frozen_first_load_procedure_v2.py
?? tools/psb-dri-re/test_frozen_first_load_capture_v2.py
?? tools/psb-dri-re/test_frozen_first_load_procedure_v2.py
?? tools/psb-dri-re/test_frozen_first_load_trace_handoff.py
```

This handoff is one additional untracked file, `docs/phase8/CODEX-HANDOFF-20261002.md`. No other file was changed by its creation. Inspect fresh `git status --short` at the next thread's start and preserve any later user changes too.

## Start here in the next thread

1. Read this handoff and the newest technical audit. Reuse the established records; do not treat historical endpoint sentences as current state.
2. Keep Gate B BLOCKED and whitelist empty. Identify whether the existing bounded read-only same-boot check is normally permitted and explicitly authorized for the next target session.
3. If it is, that check against boot `55bb90c8-fffe-4993-b656-7aa7a7b9b5e8` is the smallest next evidence-producing action. If not, request only that narrow authorization or report the exact unavailable boundary. Do not derive a new wrapper.
4. Only after fresh prerequisites and a completed permitted exact-action decision may separately authorized client staging/one-shot execution be considered. If any ioctl might have occurred, HOLD; never retry. Preserve attributable kernel diagnostics, client output and original readback before interpretation.

Nothing in this handoff establishes a future fire, completion, readback or triangle. TARGET CONTACT DURING HANDOFF: NONE.


## Continuation update: operator authorization, 2026-10-02

The owner/operator explicitly authorized autonomous repository/offline work, use of qualified artifacts, minimum necessary read-only Mini 12 checks, qualified-client staging, log/result collection, and at most one new selected frozen invocation when every applicable prerequisite is actually satisfied. This supersedes any earlier continuation statement that operator consent for those scoped actions is missing. Do not ask the operator to repeat that consent. Authorization remains conditional on the existing reviewed safety/recovery and independent project/platform boundaries; it does not authorize broader GPU work, Attempt05 retry, hot recovery, or physical LCD/scanout work.

The operator expressly stated: "My authorization resolves operator consent for actions I control. It does not manufacture an independent Gate-B decision."

The continuation read the handoff, technical audit, current diagnostic first-owner result and scope, and checked repository status/metadata. The 44 status entries match the preserved snapshot plus this handoff; no newer relevant repository file metadata supplied a superseding decision. Historical qualification and the immediately preceding triangle-path/evidence checklist were reused. No restricted review, invocation wrapper or kernel-entry material was reopened or reconstructed. No hash qualification, build, test or capture was repeated.

The normally permitted affirmative opening decision remains missing for diagnostic Build ID `594030ac153ce3c6c7dec748025142c92d90dd0c`, expected boot `55bb90c8-fffe-4993-b656-7aa7a7b9b5e8`, and action `MINI12-SGX535-REV121-FROZEN-32x32-SEQ1`. No permitted replacement decision process was identified in the available summary/policy evidence. Operator authorization changes consent, not this predicate. Gate B remains BLOCKED; whitelist remains `[]`.

Under the operator's Phase C instruction, the minimum hardware session begins only if the applicable opening conditions are legitimately satisfied. This continuation therefore stopped before target contact or client staging. The qualified local client path remains `docs/phase8/artifacts/frozen-triangle-one-shot-i386`; diagnostic-boot client/output destinations remain unestablished. No destination or substitute execution procedure was invented.

PRE-INVOCATION DECISION: BLOCKED.

TRIANGLE STATUS: BLOCKED BEFORE EXECUTION; first off-screen triangle NOT ESTABLISHED.

TARGET CONTACT / SGX INVOCATIONS THIS CONTINUATION: NONE / 0. No new boot, module, ownership, health, completion or readback observation was made. No recovery was initiated. Attempt05 and diagnostic-observability conclusions above remain unchanged.

Only this append changed the handoff; its preexisting content was preserved. No other repository file was edited, staged, committed or pushed. The next smallest blocker is the missing normally permitted exact-build/boot/action opening decision, not operator consent or an inferred SGX defect. Do not reopen the unavailable review or derive a workaround to supply it.


## 2026-10-03 — frozen candidate first-owner observed

This new live evidence supersedes the earlier current-build/current-boot identity,
without changing historical Attempt05 findings or the independent Gate-B boundary.
The frozen module Build ID is `314e2f3b37195dc56df7c57dd938d78377a5ea8e`; its
qualified bytes and client are unchanged. The unique image was exclusively staged
at `/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-314e2f3b37195dc56df7c57dd938d78377a5ea8e`,
50,805,231 bytes, SHA-256 `fe64b3dcfe74b78a6d96edcd4fd7c118c3901fcff8631afec6c14647da292e3c`.
The four-entry `custom.cfg` preserves the prior three-entry bytes exactly; old
images/entries/backups and STOCK saved/default state remain preserved.

The operator explicitly authorized a prospective project-controlled direct-menu
observation alternative. [The successor procedure](frozen-first-owner-operator-observation-20261003.md)
and its validator passed 25 focused tests and review. No photograph was supplied;
historical v2 remains unchanged and is not retroactively satisfied. The operator
confirmed the exact candidate and STOCK titles, STOCK default, one selection,
normal display/userspace within the 600-second boot watch, current local sudo,
and zero SGX/hot-module actions.

OBSERVED via the single prepared passive capture: diagnostic boot
`89fc7306-6dac-4ab9-af62-360b7152ef22`; expected loaded GNU-note SHA-256
`edb1e3e59cfd70c48d400bec89f2a30c1a436b05bf115bc1a05581abc153bdc9`, mapping to the frozen
Build ID; module Live; exactly ordered BEGIN/FILES-VERIFIED/PASS hook trace; all
70 guards passed; PCI/DRM/framebuffer/VT/IRQ ownership and services matched;
taint 12289; complete kernel log passed the bounded diagnostics classifier.
Capture uptime was 670.92–686.04 seconds, inside the 1200-second completion
window; child duration 15.12 seconds and host duration 18.14 seconds passed.
First-owner qualification applies under the prospective successor, with actual
transport provenance and operator reports retained separately. No frozen ioctl
was issued, no TA/raster/readback evidence was produced, and no triangle is established.

Exact raw sources, argv/stdout/stderr/timing, staging and physical receipts:
[session evidence](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/).
The normally permitted independent opening decision remains unavailable for this
exact build/boot/action. Gate B remains BLOCKED, whitelist `[]`. Stop here; preserve
the diagnostic boot. No client staging, further target probe, reset, invocation or
retry is authorized by this checkpoint. A future permitted action still needs its
applicable opening/authorization and fresh immediate guards.
