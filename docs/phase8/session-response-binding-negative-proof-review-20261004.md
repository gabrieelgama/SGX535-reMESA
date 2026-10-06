# G01/G02: caller-independent negative proof review

This offline continuation refines the [prior finding](session-response-binding-property-finding-20261004.md)
and [pending property request](session-response-binding-property-review-request-20261004-314e2f3b.json).
It changes no authority, artifact, handoff or historical finding. No excluded
entry/reporting implementation, integration payload, disassembly or alternate
representation was inspected. No executable counterexample was run.

**G01: NOT ESTABLISHED FROM PERMITTED EVIDENCE.** A useful conditional
same-session acceptance theorem is established below; it is not proof of the
actual caller's object selection or initialization/transition discipline.

**G02: NOT ESTABLISHED FROM PERMITTED EVIDENCE.** Readback constraints and
response consistency do not imply common invocation origin.

**Overall SESSION-TO-RESPONSE PROPERTY: NOT ESTABLISHED.** Neither actual clause
is disproven. Countermodels below concern guarantees of the permitted APIs,
not the behavior of the excluded qualified implementation.

## G01 construction chain and strength of each guarantee

| Relationship | Guarantee | Evidence/limit |
| --- | --- | --- |
| Fresh successful constructor O -> embedded S | STRUCTURALLY GUARANTEED / RUNTIME GUARANTEED | [Owner declaration](../../kernel/sgx535_frozen/gma500_bo_owner.h#L27); [construction](../../kernel/sgx535_frozen/gma500_bo_owner.c#L269) zeroes O; session_begin at345 resets S. Applying construction to this I is not established. |
| O -> embedded service V -> S | RUNTIME GUARANTEED | [Constructor bind](../../kernel/sgx535_frozen/gma500_bo_owner.c#L431) supplies exact addresses; [bind](../../tools/psb-dri-re/frozen_fixed_service.c#L48) stores the explicit session pointer. |
| Backend B -> O and O.fixed_service.session == &O.session | RUNTIME GUARANTEED | [backend_begin](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L383) checks this O's embedded relationship before binding B.owner. |
| Operation's actual service argument is V (or a faithful representation using S) | CALLER-DEPENDENT | [run_once](../../tools/psb-dri-re/frozen_fixed_service.c#L287) receives service and context separately; no permitted owner-taking runner derives both. |
| Status sampled through B -> B.owner's device, sequence and active IRQ owner | RUNTIME GUARANTEED | [sample](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L58), checks67-79 and MMIO81-93. B is a separate backend object, not literally O. |
| Status accepted through V -> V.session | RUNTIME GUARANTEED | [service_status](../../tools/psb-dri-re/frozen_fixed_service.c#L195) passes its supplied session at210. |
| B.owner.session == V.session from equal sequences alone | NOT STRUCTURALLY GUARANTEED | No pointer comparison between these two actual arguments; a number is not an identity capability. |
| Hardware status represents this task rather than another source | CONTRACTUALLY REQUIRED | [Contract attribution requirement](../../tools/psb-dri-re/frozen_kernel_contract.h#L262), [status-source requirement](../../tools/psb-dri-re/frozen_fixed_service.h#L81); baseline/isolation/source-lifetime evidence remains necessary. |

The backend-begin comparison is to **B.owner's embedded service**, not the
service later supplied to run_once/status. Neither type structure nor a service
owner field prevents cross-pairing: the generic service stores a session pointer
but no backend/owner identity. BO/command/USE payload comparisons check values,
not session-pointer equality. They may reject different allocations/plans, but
cannot be treated as a universal identity check.

## New conditional negative theorem: a fresh unmatched session cannot sample

Let S_B = B.owner.session and S_V = V.session. Assume:

1. B.owner is freshly constructed for this operation and backend_begin succeeds.
2. The real qualified backend ops/status source are used, with one stable B.
3. One uninterrupted run_once(V,B,q,...) is the only transition producer;
   there are no copies/rebinding, independent session advances, concurrent
   operations, direct scalar writes or owner/backend substitution.
4. q is nonzero, as required by run_once and enter_fire.

Then:

- [session_begin](../../tools/psb-dri-re/frozen_kernel_contract.c#L841) sets S_B.sequence=0.
- Backend begin requires HOST_IMAGE_FINAL and does not assign sequence.
- [prepare](../../tools/psb-dri-re/frozen_fixed_service.c#L80) advances only S_V;
  [fire_ta](../../tools/psb-dri-re/frozen_fixed_service.c#L168) calls enter_fire(S_V,q,...).
- Within these permitted transitions, [enter_fire](../../tools/psb-dri-re/frozen_kernel_contract.c#L899)
  is the only nonzero session.sequence assignment. Backend callbacks do not advance S_B.
- If S_V differs from S_B, S_B.sequence remains0. The first real completion
  sample requires S_B.sequence==q at [backend line70](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L70),
  fails with -ENODEV before returning hardware status to service acceptance,
  and run_once stops at the status-source error.
- Therefore an accepted hardware completion through this isolated fresh run
  implies S_V==S_B, without requiring literal identity of the service object V.

This proves **G01-ACCEPTANCE conditionally**, not G01-CONSTRUCTION. Premises1/3
for the exact ioctl are not supplied by public request layout, one userspace call,
boot identity or the component types. One userspace ioctl is not a proof of one
internal transition producer. These premises must not be silently made fresh guards
if the permitted external interfaces do not actually observe them.

This is also **not a pre-issuance safety check**: TA fire precedes the completion
sample. Baseline/bootstrap already read hardware. A mismatch can fail before
completion acceptance after device operations may have occurred; STOP/NO RETRY
must still apply. No device operation occurred in this review.

## Adversarial cases: what is and is not impossible

| Case | Source-level result |
| --- | --- |
| Two simultaneously valid, successfully constructed frozen owners A/B | Rejected by [global owner claim](../../kernel/sgx535_frozen/gma500_bo_owner.c#L26). This literal two-live-owner example is not a valid constructed state. |
| Detached service/session A with matching payload values, fresh backend owner B, sole run_once on A | Can pass value comparisons in principle; B's sequence stays0, so first completion sample rejects. No accepted completion in this conditional case. |
| Distinct session copied after the active owner's fire, same numeric sequence, supplied to service_status | Generic acceptance is not rejected merely because the session address differs. The [existing copied-session test](../../tools/psb-dri-re/test_frozen_fixed_service.c#L212) supplies a different sequence45 versus44; it does not assert rejection with equal sequence44. |
| Above detached session receives accepted classes while backend owner's session is otherwise untouched | Sample checks backend owner's matching sequence; status mutates the detached session. TA/raster acceptance can be possible through the stage APIs if their ordinary preconditions/payload checks hold. [USE_RELEASE](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L361) still rejects unless the backend owner's own session reaches SCENE_COMPLETED. Acceptance is weaker than successful retirement. |
| Detached snapshot of an already completed owner session, same plans, service_retire using its backend | Both phase checks can pass by value: service session completed and backend owner session completed. They do not compare their addresses. This is not a fresh mismatched run_once, nor proof of an actual bad reporter. A faithful snapshot can carry valid provenance. |

Thus the answer to "can accepted events occur under mismatched session pairing?"
is **YES at the generic stage API level**, subject to the listed state/payload/source
preconditions; **NO for the isolated fresh run_once mismatch theorem**. A second
live constructed owner is excluded, but standalone/copied sessions are not.
The universal claim "acceptance itself enforces pointer identity" is disproven;
the actual candidate's G01 is not disproven.

We do not require embedded-object identity as an end in itself. A faithful snapshot
or a service using the same S can be sufficient. The weakest sufficient G01'
concerns accepted-event **lineage**:

> The accepted state underlying the authoritative response derives from the
> actual qualified status source of the freshly initialized active hardware owner
> for this ioctl, directly or by a faithful snapshot; no independent/stale session
> transition lineage substitutes its state.

Remaining exact G01 proposition: whether that current-ioctl lineage/freshness is
guaranteed. The conditional theorem supplies it if its actual premises are proven;
we need neither caller intent nor a specific service address beyond that.

## G02 field lineage and negative checks

| Public field | Permitted producer and valid lifetime | Proven check / remaining projection |
| --- | --- | --- |
| observed_events | S.observed_events; reset on begin/fire; qualified acceptance; retained through retirement | Helper ledger semantics RUNTIME GUARANTEED. R's source session/current lifetime CALLER-DEPENDENT. |
| phase | S.phase; completion8, retirement9, releaseEMPTY | Transition discipline RUNTIME GUARANTEED. Response sampling/projection CALLER-DEPENDENT. |
| outcome | No public-outcome producer/storage in permitted owner/session/service interfaces | Mapping from actual operation to public outcome UNKNOWN. Expected2 is a criterion, not a mapping guarantee. |
| operation_errno | Backend/service/readback return codes, not a typed response writer | Mapping into operation_errno CALLER-DEPENDENT/UNKNOWN; nonnegative syscall result is not sufficient completion. |
| color_bytes | capture_color(O), O's own COLOR BO, live initialized/powered/retired owner | Source within that helper RUNTIME GUARANTEED. Supplied u8* is not tied to this R or I. |
| color_observed | No permitted typed public-field setter | Whether it means a successful capture of this same owner is UNKNOWN at projection. |
| color_fnv1a/nonzero/row counts | Same mapped page passed to summarize_color; candidate summary reset each call | Summary-versus-input consistency RUNTIME GUARANTEED. Projection into R and common ledger origin CALLER-DEPENDENT. |
| request prefix | Client zero-initializes R and supplies abi1/op1 | Initial contents STRUCTURALLY GUARANTEED for the client. No kernel reporting-authority marker or lineage guarantee in UAPI. |
| Exported image versus response image | Client writes R.color_bytes and the complete R | Same userspace source PROVEN subject to file success. This cannot authenticate kernel field origins. |

[capture_color](../../kernel/sgx535_frozen/gma500_bo_owner.c#L204) checks retirement,
initialization, power and one mapped color page. It explicitly derives raw bytes
and [summary](../../tools/psb-dri-re/frozen_kernel_contract.c#L692) from the same page.
It receives one owner, but it does not produce ledger/outcome/phase as a unified
typed response. No inspected permitted reporting helper takes O and constructs
all of R. The [UAPI](../../kernel/sgx535_frozen/gma500_fixed_uapi.h#L10) defines layout
and direction, not authoritative same-operation projection.

Negative guarantees:

- Direct readback after owner_release fails because initialized=false and phase=EMPTY;
  page resources are gone. Readback before retirement also fails.
- Invalid event order/sequence/duplicates and premature retirement are rejected.
- Those checks validate the owner/session passed to each helper; they do not
  identify which earlier saved scalar/buffer is subsequently placed in R.
- Correct summary/image agreement excludes summary corruption, not another image
  source. Raw-image equality to R proves common userspace object, not common
  kernel owner. Correct geometry is not completion provenance.

Could ledger/image originate from different owner lifetimes and still satisfy
these generic permitted invariants? **YES as an abstract composition**: retain
an earlier valid terminal ledger snapshot, release that owner, and separately
retain readback from a later valid retired owner. Their ledger/phase values and
deterministic frozen geometry can agree. No simultaneous double claim, invalid
readback or contract-order violation is required. This does not demonstrate that
the actual reporter does such mixing, or that both lifetimes can exist under the
current zero-invocation history. It proves that response consistency alone is
not a lineage discriminator. Synthetic response qualification independently
illustrates that apparently correct fields can be generated without any kernel owner.

G02-CONSTRUCTION (implementation intent/particular assignments) is unnecessary.
The weakest sufficient **G02-OBSERVABLE** is:

> On an authoritative successful return, all claimed terminal state/results
> and color bytes/summaries faithfully represent the one current ioctl's
> hardware-backed session and retirement-gated color capture, captured while
> their source lifetime was valid; no stale or mixed-operation values substitute.

This permits copying a valid outcome/readback to an independent snapshot before
release, then returning/preserving that snapshot afterward. It does not unnecessarily
require the owner to remain allocated through userspace file preservation.
The exact missing G02 proposition is that **common-current-operation authoritative
projection**. No permitted consistency check makes it unavoidable.

## Artifact-specific cross-checks and limits

No tests were rerun. Existing evidence was inspected specifically for pairing,
projection and lifetime guarantees, not to repeat qualification.

- [Service tests](../../tools/psb-dri-re/test_frozen_fixed_service.c#L143) use an explicit
  standalone session and [mock source](../../tools/psb-dri-re/test_frozen_fixed_service.c#L32).
  Their [build](../../tools/psb-dri-re/test_frozen_fixed_service.py#L14) does not compile
  kernel owner/backend/reporting. Numeric mismatch and state/order tests are not
  an actual backend pointer-mismatch qualification.
- [Contract tests](../../tools/psb-dri-re/test_frozen_kernel_contract.c#L386) cover
  resets, sequence, required ordering and rejection of retirement bypass.
- [Response tests](../../tools/psb-dri-re/test_frozen_triangle_response_export.py#L66)
  inject synthetic returned fields. Their [preservation checks](../../tools/psb-dri-re/test_frozen_triangle_response_export.py#L184)
  prove exporter fidelity, not kernel projection.
- [_Static_assert](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c#L19)
  and retained ABI records prove object size/offsets, not provenance.
- The [separate scene-owner API](../../tools/psb-dri-re/frozen_scene_owner.h#L6)
  is not ioctl integration; source-manifest scene_owner_in_module=false.
- Exact candidate source manifest/identity/offline-result records establish pins,
  qualified component tests and ABI/import/build scope. They do not state G01'/G02'.
- Earlier corrected-image/module qualification records concern different module
  identities and import/image qualification, not this semantic boundary. They
  supply no stronger current-artifact postcondition.
- Response client's offline integration contract and independent-review record
  explicitly exclude kernel-entry/hardware attribution. No currently available
  permitted property confirmation was found there.

Retained evidence paths inspected:

1. `/home/gama/sgx535-offline/request-byte-decoding-20261002T222753Z/source-manifest.json`
2. `/home/gama/sgx535-offline/request-byte-decoding-20261002T222753Z/qualification/identity.json`
3. `/home/gama/sgx535-offline/request-byte-decoding-20261002T222753Z/qualification/offline-result.json`
4. `/home/gama/sgx535-offline/phase8-response-export-20261004T092750Z/final/client-qualification.json`
5. `/home/gama/sgx535-offline/phase8-response-export-20261004T092750Z/final/SYNTHETIC-paired-evidence/results.json`
6. Same final directory: `offline-integration-contract.json`, `independent-review.json`, `final-verification.json`
7. `docs/phase8/artifacts/private-gpu-va-fix-20261002/corrected-image-01/module-qualification.json` and `qualification-summary.json`
8. `/home/gama/sgx535-offline/private-gpu-va-diagnostic-20261002/qualification-recheck-01/proof/qualification.json`

Files with unestablished permission classification and excluded integration/caller
tests were not read merely to broaden this search. Their absence from the proof
is not evidence of a defect. No whole-module absence claim is made.

## Decision boundary and unchanged state

[The narrower pending confirmation request](session-response-binding-minimum-property-review-request-20261004-314e2f3b.json)
asks separately for G01' lineage and G02' common authoritative projection, with
the exact conditional premises and permitted supporting guarantees required.
It does not request hidden code, implementation intent or forbidden access.
The original request and [contingent fallback](session-response-binding-independent-design-review-request-20261004-314e2f3b.json)
remain unchanged. Available permitted artifact analysis is exhausted; external
property confirmation has not been obtained. The fallback is **not activated or
approved**, and no implementation-review request is justified yet.

Fresh init clears stale ledger7; retirement preserves the current ledger; release
invalidates direct readback but does not authenticate copied records. G01/G02
remain independent proof obligations; neither is collapsed into a generic UNKNOWN.

Raw STATUS history, a new token, module change or UAPI change: **NOT DEMONSTRATED
necessary**. No patch or qualification change is justified. Existing qualification
and historical70/70 retain their established scopes, without becoming fresh.

Design PARTIAL; READY FOR IMPLEMENTATION REVIEW NO; PRE07 live BLOCKED;
READY FOR EXECUTION AUTHORIZATION NO. Gate B/client approvals unchanged;
sgx_execution_authorized=false; SGX0; task hardware interactions0;
triangle NOT ATTEMPTED; no new HOLD/recovery. Only two review-only files are
created, untracked; no stage/commit, source patch, builds, qualification, hashes,
test runs, contact, staging or execution.
