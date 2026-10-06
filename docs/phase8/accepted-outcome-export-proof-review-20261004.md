# Accepted-outcome export proof: narrow pending review

Prepared 2026-10-04T20:18:23Z. Offline review preparation only, at the maintainer's
request. No approval, implementation, live observation or authority change.

The pending property question is in
[the exact-context request](accepted-outcome-export-property-review-request-20261004-314e2f3b.json).
Its decision, authority and issuance timestamp remain null. Historical authority,
qualification and raw evidence remain unchanged; the authoritative handoff is not
edited by this record.

## Established scope

CONFIRMED from permitted source and the completed source investigation:

1. Hardware STATUS1/2 sampling exists in [the backend](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L58).
2. [Contract acceptance](../../tools/psb-dri-re/frozen_kernel_contract.c#L953) distinguishes TA finished, end-render and 3D memory-free, rejecting invalid ownership, sequence, phase and ordering.
3. [Backend begin](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L383) binds the backend to an owner whose embedded service uses that owner's session.
4. Sampling and acceptance check the same supplied session sequence. This is internal association, not an exported globally unique token.
5. [The ledger](../../tools/psb-dri-re/frozen_kernel_contract.c#L926) retains accepted completion classes through retirement.
6. [The sampling loop](../../tools/psb-dri-re/frozen_fixed_service.c#L310) overwrites latest raw observation fields on every later sample, including empty samples.
7. [The public UAPI](../../kernel/sgx535_frozen/gma500_fixed_uapi.h#L10) does not export accepted raw snapshots with owner/session/sequence provenance.
8. No sufficient successful-completion emitter was found in the permitted inspected surfaces. This is the scoped negative finding from [the permitted inventory](pre07-kernel-attribution-20261004.md#L24) and completed source investigation, not a new global search.

NOT ESTABLISHED: absence of an adequate emitter anywhere in the complete module;
necessity of a module change, UAPI change or new correlation token; absence of
adequate emission in the excluded entry/reporting implementation; or that latest-
diagnostic overwrite is an execution defect. Its [declared purpose](../../tools/psb-dri-re/frozen_fixed_service.h#L66)
is latest-point/first-failure diagnostics, so the demonstrated limitation concerns
evidence retention.

## Representation-independent proof obligation

For invocation I, preserved response R and original image C when produced, there
must be one identified current session S such that the facts below hold. This is
a proof obligation, not a claim that these facts have been observed.

| Required fact | Possible representation; none mandated |
| --- | --- |
| I occurred in the exact approved boot/module/build/action/client/UAPI context, with continuity | Independently captured context and producer evidence |
| S was the active owner/session for I; no other eligible operation can supply its outcomes | Causal binding guarantee or proved exclusive-current-operation association |
| Accepted outcomes are from S's actual hardware-status source and qualified contract, not an injected/synthetic ledger | Permitted qualified-interface guarantee plus actual source/context evidence |
| S began with cleared completion state; stale hardware/software outcomes cannot substitute | Qualified reset/baseline semantics tied to S and current isolation/history evidence |
| TA completion, end-render and 3D memory-free were accepted for S | Attributable accepted ledger, accepted-event records, or another proven equivalent |
| Required partial ordering held: TA accepted before raster continuation; raster outcomes after raster-start gating | Qualified transition invariants with attributable terminal ledger, or accepted-event ordering evidence |
| Terminal success/failure/HOLD state refers to S and is not erased by report/cleanup | Stable terminal outcome evidence; do not infer success from zero diagnostic fields |
| R belongs to I and faithfully carries S's relevant outcome at response construction | Publicly supportable same-session export guarantee plus producer/artifact preservation evidence |
| C, when produced, belongs to that same response/readback transaction | Producer/source binding and full-byte equality to R[172:4268] |
| Required outcome evidence persists until preservation, with loss detectable | Retained response/outcome record and capture completeness/integrity evidence |
| Client failure, incomplete copy-out, missing records or unknown issuance remains explicit | Actual syscall/process outcome, partial originals and UNKNOWN/STOP evidence |

No UUID, PID, timestamp, new sequence field, raw register history or total ordering
of end-render versus memory-free is required merely for convenience. Sequence1
alone is not unique across boots or module instances. A hash authenticates neither
producer nor session. These obligations follow [the unrestricted correlation
requirement](pre07-clean-room-review-scope-20261004.md#L53), not excluded internals.

## Public 4268-byte response: meaning and limits

These are schema/approved-client facts and conditional report meanings. The public
header does not establish how excluded kernel code constructs or copies R.

| Field | Classification and supported meaning | Not established by the field |
| --- | --- | --- |
| abi_version | Protocol/request compatibility; client supplies 1 | Unique invocation, boot or hardware identity |
| operation | Fixed requested operation; client supplies 1 | Which particular invocation/session produced an outcome |
| flags | Request options; client supplies zero | Runtime ownership or completion |
| reserved | Request constraint; client supplies zero | Provenance |
| operation_errno | Reported service error; necessary success value 0 | Exact failing callback, actual execution or success alone |
| outcome | Reported outcome; necessary success value 2 | Independently attributable hardware completion |
| phase | Reported phase; necessary success value 9 | Same-session lineage without export binding |
| observed_events | Reported accepted-class ledger; expected success 7 | Producer/session binding, timestamps, complete raw history or independent proof alone |
| color_observed | Reported color observation; necessary success value 1 | GPU origin or preserved original completeness |
| color_fnv1a | Color summary/checksum | Geometry, origin or attribution |
| color_nonzero_pixels | Whole-image summary | Pixel positions/values or triangle proof |
| color_row_nonzero[32] | Per-row coverage summaries | Exact geometry/foreground/background or origin |
| color_bytes[4096] | Complete candidate image byte array, starting at offset172 | Hardware production, layout qualification or completion provenance by itself |

There are no public boot/build/session/process identifiers, accepted raw status
records, diagnostic failure-stage/source fields or explicit accepted-event order
entries. The four fixed input words identify requested semantics, not a nonce.

[The approved client](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c#L77)
has one ioctl site, exports its local complete post-call object before normal
summary/image handling, and exports the image from that same object. Thus actual
producer and capture proof could bind the two exports together without a new
client. Equality establishes byte consistency, not kernel session origin. Negative
ioctl bytes remain UNAUTHORITATIVE: initialization/partial updates can leave
plausible values without an authenticated returned service result.

## Ledger sufficiency: six separate answers

| Question | Answer and qualification |
| --- | --- |
| A. Required completion classes accepted? | YES for the genuine qualified current-session ledger: bits1/2/4 represent accepted TA/end-render/3D-free. NO for an unbound byte word merely equal to7. |
| B. Accepted for the current session? | Internally YES: phase/sequence checks and per-session state enforce association. Externally NOT ESTABLISHED that the returned field is that current session's ledger. |
| C. Session bound independently to exact approved ioctl? | NOT ESTABLISHED by presently permitted records. This is the pending property question. |
| D. Stale state excluded? | Internal begin/enter_fire reset ledger and raster flags. Applying those transitions to I, hardware baseline/isolation and history requires current-session binding and independent continuity/exclusivity evidence. |
| E. Required ordering established? | YES conditionally through qualified invariants: TA precedes raster-start gating; non-TA completions require TA and raster_started. End-render/free may be accepted in either order or one sample. Total order, exact event times and raw occurrence multiplicity are not encoded. |
| F. Raw snapshots unnecessary? | YES for completion-class attribution if actual hardware-backed contract acceptance, same-session export, exclusivity, ordering and preservation are proven. Not for reconstructing detailed failure history, which remains separately bounded. |

Relevant [reset](../../tools/psb-dri-re/frozen_kernel_contract.c#L841),
[enter_fire](../../tools/psb-dri-re/frozen_kernel_contract.c#L899),
[raster gate](../../tools/psb-dri-re/frozen_kernel_contract.c#L915),
[acceptance](../../tools/psb-dri-re/frozen_kernel_contract.c#L926), and
[retirement](../../tools/psb-dri-re/frozen_kernel_contract.c#L1029) invariants are
permitted source facts. A ledger from a CPU harness still cannot establish GPU
execution. The model can accept supplied events; provenance must identify the
actual qualified backend in the real transaction.

Minimum gap classification: **B. SESSION-TO-IOCTL BINDING GAP**. This includes
establishing that the reported ledger/phase/image faithfully refer to S at export.
Completion-class retention already exists; response byte export is solved; required
partial ordering is encoded. Neither a separate retention defect nor a separate
export implementation defect has been demonstrated necessary for completion proof.
This refinement does not satisfy other PRE07 live prerequisites or change authority.

## Three conceptual options; no implementation selected

| Option | Missing proof supplied | Execution/request/UAPI impact | Identity and qualification impact |
| --- | --- | --- | --- |
| 1. No module change | Establish existing ledger-to-current-ioctl/response binding, actual hardware source and independently observed context/exclusivity/preservation | No SGX behavior, request or UAPI change | No artifact identity/qualification changes; approved client remains reusable. Property and eventual live evidence still required. |
| 2. Retention only | Preserve bounded accepted outcomes for later observation if an actual loss at the necessary evidence boundary is demonstrated | Intended observational change only; fixed request unchanged; no UAPI change inherently required | Any implemented in-module retention changes module bytes and image identity. Alone it does not solve external binding/export. Not presently justified. |
| 3. Export/binding change | Provide independently reviewable same-session outcome/response association if existing boundary is proven inadequate | Intended evidence change only; actual semantics must be reviewed. No need for changed request bytes; UAPI necessity not established | In-module export changes module/image identity; separate observation-only work may leave them unchanged only if a suitable permitted interface exists. Approved client reusable only while its exact interface/bytes remain unchanged. Not presently justified. |

All three leave the one-shot unspent. If a module changes, its previous byte-
identity qualification cannot qualify the successor: new module identity/offline
qualification, image construction/identity and applicable runtime qualification
would be needed. Historical70/70 remains valid for the original artifact/context,
not a changed module. Unchanged geometry, request/UAPI/client evidence and independent
offline archive requirements remain reusable. Any new evidence schema/parser or
retention/export behavior requires its own focused verification. No build,
qualification, patch or interface selection occurs here.

## Narrowest next review and conditional fallback

Property-only review is conceptually possible: answer whether the exact existing
qualified boundary guarantees the required externally supportable properties, using
only independently permitted evidence. Do not request source, control flow, hidden
implementation details, reconstruction hints or an alternate means of obtaining
restricted information. No available reviewer/capability or affirmative answer is
asserted. An inaccessible property remains NOT ESTABLISHED; this is not evidence
that the property is false.

The request asks for a supported property finding, not approval to access material,
change artifacts or execute. A finding can resolve this specific semantic gap only
if its public/permitted basis, exact scope and resulting observable guarantee are
reviewable. An unsupported attestation, readiness approval or client approval does
not replace technical evidence. No additional signer/credential gate is invented.

No fallback design-permission request is issued now: property-only review has not
been shown impossible or answered negatively. If it later cannot establish the
property, separately seek design-only review of an independently derived outcome
binding/export contract, excluding every restricted input and indirect substitute.
There is no automatic escalation or permission to design/implement kernel changes.

If new work is actually justified, the minimum semantic object is a faithfully
bound current-session accepted ledger (or equivalent accepted entries), required
partial order, terminal outcome/HOLD state and context/invocation provenance,
retained until preservation. Raw hardware history is not inherently necessary.
No concrete ABI, emitter or patch location is selected.

## Symmetry, stopping and current state

Failure/HOLD often preserves stronger detail because the service returns before
another sample overwrites its latest diagnostic. Known zero-stage/raw-result
ambiguities remain; see [the diagnostic audit](diagnostic-gate-b-technical-audit.md#L87).
Success and failure can share an attributable terminal-outcome contract; symmetry
does not require identical diagnostic detail or manufacture a record after a crash.
Failure retains the accepted prefix, failure/HOLD/UNKNOWN state and all partial
artifacts. Missing response/logs, possible issuance, loss or ambiguity always means
STOP / NO RETRY, with recovery governed separately.

Attributable acceptance plus full original image analysis could support a future
triangle; ledger alone, perfect summaries, hashes or a synthetic image cannot.
Original response/image/kernel evidence and streams must still be preserved under
the live evidence requirements, regardless of the eventual representation choice.

Design PARTIAL; complete implementation review NOT READY; PRE07 live BLOCKED;
execution authorization NOT READY. Gate B and client approvals unchanged.
sgx_execution_authorized=false; candidate SGX invocations0; task hardware
interactions0; triangle NOT ATTEMPTED; no new HOLD/recovery. No live paths selected,
no staging/guard collection, no patch/build/test/rehash, no stage/commit.
