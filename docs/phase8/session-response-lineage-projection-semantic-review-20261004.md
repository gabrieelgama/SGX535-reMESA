# G01′ lineage and G02′ projection: semantic review

Scope: offline permitted property evidence only, under the current maintainer
instruction. [Minimum request](session-response-binding-minimum-property-review-request-20261004-314e2f3b.json)
and [negative-proof review](session-response-binding-negative-proof-review-20261004.md)
remain unchanged. No actual candidate behavior is disproven. No restricted
implementation/alternate representation was accessed or inferred.

| Property | Finding | Exact remaining guarantee |
| --- | --- | --- |
| G01′ | NOT ESTABLISHED FROM PERMITTED EVIDENCE | Accepted state presented as this ioctl's outcome actually follows the fresh current operation's qualified hardware-sample lineage, rather than another supplied transition history. |
| G02′ | NOT ESTABLISHED FROM PERMITTED EVIDENCE | The authoritative successful response's terminal state/results and color capture are a faithful common-operation projection of that current transaction. |

These findings concern the exact ioctl guarantee. A conditional internal lineage
theorem is established independently of G02′ below. Failure to prove the exact
guarantee is neither disproof nor proof of a needed patch.

## Internal transitive lineage theorem, with explicit premises

P1. Fresh qualified initialization/fire clears the actual receiving session's
ledger and relevant transition state: [session_begin](../../tools/psb-dri-re/frozen_kernel_contract.c#L841)
and [enter_fire](../../tools/psb-dri-re/frozen_kernel_contract.c#L899).

P2. Every input contributing to the genuine ledger originates in the real,
stable, current active-owner hardware backend, not another callback or a supplied
software claim. [Sampling](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L58)
checks active/power/sequence/IRQ ownership and reads status; the [service loop](../../tools/psb-dri-re/frozen_fixed_service.c#L311)
forwards its selected source's returned words. The bodies guarantee this data flow
when those actual components/arguments are used. They do not identify the exact
ioctl as their caller or authenticate arbitrary caller-supplied status words.

P3. Hardware and software pending observations have no prior-operation content;
continuous ownership, history and isolation exclude another eligible producer
through the interval. Existing [evidence policy](accepted-outcome-export-proof-review-20261004.md#L34)
and [guard contract](pre07-clean-room-design-contract-20261004-314e2f3b.json#L416)
require baseline/history/continuity/producer exclusion. Their availability as
requirements is not proof that the actual observations exist.

P4. This receiving ledger, or a demonstrably faithful snapshot, is the one relied
upon for the current operation. No unrelated transition history is substituted.
This is the unestablished application/lineage guarantee, not something proven by
boot identity, sequence equality, fresh destination creation or one userspace call.

P5. Qualified acceptance alone updates the cleared ledger; it checks sequence,
classes, duplicate/order conditions and raster gating, and retirement preserves
accepted classes: [acceptance](../../tools/psb-dri-re/frozen_kernel_contract.c#L926)
and [retirement](../../tools/psb-dri-re/frozen_kernel_contract.c#L1029).

Therefore: a genuine ledger satisfying P1–P5 contains only accepted current-operation
events, directly or by faithful snapshot. No pointer identity, globally unique
sequence or raw-status journal is needed for that implication.

**The implication is proven; the exact ioctl guarantee is not.** Promoting G01′
to established for the candidate would require treating P2/P4's actual producer
and state selection as established. The current public interface and external
guards do not supply that guarantee. Qualified-source existence is not actual
invocation data-flow evidence. The [contract header](../../tools/psb-dri-re/frozen_kernel_contract.h#L216)
expressly distinguishes bookkeeping claims from hardware evidence; its [attribution
requirement](../../tools/psb-dri-re/frozen_kernel_contract.h#L262) is a precondition,
not authentication of a caller's supplied claim.

This separates A (hardware origin), B (accepting session origin), and C (current
operation). P2/P3 establish A for real backend observations; P1/P5 establish B
for genuine component state; P4 binds the state relied upon to C. G02′ is not
needed to prove the internal implication, but is needed to rely on a returned
public field as that state. G01′ is not left unestablished merely because G02′ is.

## Alternative lineages

Each classification is scoped to the stated case, not an actual candidate defect.

| Candidate | Classification | Reason / exclusion needed |
| --- | --- | --- |
| Old ledger surviving successful fresh initialization/fire | IMPOSSIBLE BY QUALIFIED INVARIANT | Both reset the actual ledger to0; applying them to this operation still needs evidence. |
| Copied session | POSSIBLE UNDER PERMITTED API | A faithful copy preserves lineage and is allowed. A different supplied history can also satisfy numeric checks; proof must distinguish these. |
| Another simultaneously constructed frozen owner | IMPOSSIBLE BY QUALIFIED INVARIANT | Global claim rejects a second owner while held; not a universal GPU-producer lock. |
| Previous invocation | EXCLUDED BY REQUIRED FRESH GUARD | Complete applicable history/current-operation association must exclude reuse; values alone cannot. |
| Previous boot | EXCLUDED BY REQUIRED FRESH GUARD | Continuous actual boot/module and producer/artifact association, not declared UUID metadata. |
| Ordinary unrelated eligible GPU producer | EXCLUDED BY REQUIRED FRESH GUARD | Continuous producer isolation is needed; fixed IRQ ownership alone does not establish it. |
| Stale software pending IRQ words | POSSIBLE UNDER PERMITTED API | begin does not clear pending_status1/2; sampling consumes them. Fresh accumulator initialization/source lifetime is a required premise. |
| Selected pre-existing register status present at baseline | IMPOSSIBLE BY QUALIFIED INVARIANT | Service rejects a non-clean selected baseline. This does not exclude delayed earlier events or software pending words. |
| Delayed prior event after clean baseline | EXCLUDED BY REQUIRED FRESH GUARD | Quiescence/history/source continuity must exclude it; baseline alone is insufficient. |
| Second internal transition producer | POSSIBLE UNDER PERMITTED API | Generic session/status APIs permit independent transitions. One userspace ioctl does not prove their absence. A second faithful consumer of the same current evidence is not itself substituted lineage. |
| Numeric sequence reuse | POSSIBLE UNDER PERMITTED API | q must be nonzero/matching, not globally unique. Current source/lifetime history supplies the required distinction. |
| Session reinitialization | POSSIBLE UNDER PERMITTED API | Valid begin clears state; old saved records remain possible. Required current-lifetime association excludes substitution. |
| Owner release/reuse | POSSIBLE UNDER PERMITTED API | Release invalidates direct readback; later construction is fresh. Earlier copied terminal/image values need provenance, not pointer permanence. |

The software-pending distinction follows [capture/sample](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L41),
[baseline](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L287),
[service baseline rejection](../../tools/psb-dri-re/frozen_fixed_service.c#L156),
and [backend begin](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L383).
No assertion is made that the actual caller fails to initialize its backend.
No inaccessible internal observation is relabeled an available fresh guard.

## Authority of a returned object

| Evidence class | Meaning |
| --- | --- |
| FAILED CALL BYTES | Negative ioctl: local bytes are explicitly UNAUTHORITATIVE; issuance/execution may remain UNKNOWN; preserve/no retry. |
| PARTIAL FAILURE EVIDENCE | Incomplete response/export, failure/HOLD tuple or missing records: retain all available evidence. Claim only separately supported facts; no terminal-success inference. |
| Candidate terminal-success tuple | Nonnegative syscall and operation_errno0/outcome2/phase9/events7/color_observed1 are necessary reported values, not proof of authority. |
| AUTHORITATIVE TERMINAL RESPONSE | Complete actual returned object plus applicable semantic producer/operation guarantee and provenance. Preservation proves retained integrity; it does not supply missing kernel lineage. |

The [client](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c#L77) explicitly
labels negative-call bytes, preserves R and exports R.color_bytes. The [public UAPI](../../kernel/sgx535_frozen/gma500_fixed_uapi.h#L10)
does not define a verified authority marker or a same-transaction field postcondition.
Defining an authoritative response as one having proven provenance must not be
used circularly to prove that a success-looking response has provenance.

## G02′ transitive proof attempted

P1: qualified session state supplies a valid ledger/phase through retirement.
P2: [capture_color(O)](../../kernel/sgx535_frozen/gma500_bo_owner.c#L204) supplies
O's color page and summary only while O is initialized, powered and retired.
P3: this response's terminal fields/results and color capture are projected from
that one current operation (including a valid retained snapshot).
P4: client raw/image export faithfully preserves its returned local object.

P1/P2/P4 hold for their named components and preconditions. **P3 is missing.**
Thus the candidate-level conclusion does not follow. A faithful pre-release
snapshot is allowed; simultaneous field copying or keeping O alive through
userspace preservation is unnecessary.

| Mixing case | Classification | Why observable consistency does not exclude it |
| --- | --- | --- |
| A. Current ledger + old image | Possible as abstract API composition | Current retirement checks do not authenticate an earlier saved byte array subsequently put in R. |
| B. Old ledger + current image | Possible as abstract API composition | Color helper validates its own source, not separately supplied terminal scalars. |
| C. Current phase + another lifetime's ledger | Possible as abstract API composition | Phase/ledger compatibility is a value relation, not common origin. Simultaneous valid double-owner construction is separately rejected. |
| D. Current image + unrelated outcome | Possible as abstract API composition | No inspected permitted helper unifies public outcome production with color capture. |
| E. Snapshot returned after another owner initializes | Possible as abstract API composition | Can be valid if the snapshot belongs to this operation and was captured while valid; later allocation itself neither invalidates nor authenticates it. |
| F. Correct values from previous invocation | Possible as abstract API composition | External old-file substitution can be excluded by producer/continuity/file guards; newly returned stale content still requires the semantic projection guarantee. |

None is a demonstrated candidate counterexample. Authoritative-response semantics
would exclude these mismatches **if** a qualified same-operation postcondition
were established. Layout/tuple/client checks do not establish that postcondition.

There is no proven single anchor field. Ledger provenance does not automatically
bind color; readback provenance does not bind outcome; phase/outcome numbers do
not bind either. A permitted contract identifying one common transaction result
object could be the minimal anchor, but no such kernel response guarantee was found.
The local userspace object groups bytes, not their kernel semantic origins.

## Permitted contractual evidence exhausted

Headers, qualified source comments and component API contracts provide reset,
acceptance, sequencing, owner/readback and source-attribution preconditions. They
do not state that this public response reports this ioctl's one transaction.
Evidence-policy/design records require that relation; requirements are not proof
that the qualified boundary supplies it. Metadata proves pins/layout/component
qualification; it does not qualify response projection. The retained independent
client review explicitly excluded kernel-entry/hardware attribution.

Existing records inspected, not requalified:

- `/home/gama/sgx535-offline/phase8-response-export-20261004T092750Z/final/client-qualification.json`
- Same directory: `offline-integration-contract.json`, `independent-review.json`
- `/home/gama/sgx535-offline/request-byte-decoding-20261002T222753Z/qualification/offline-result.json`
- [Accepted-outcome proof obligations](accepted-outcome-export-proof-review-20261004.md)
- [Clean-room scope](pre07-clean-room-review-scope-20261004.md) and [design contract](pre07-clean-room-design-contract-20261004-314e2f3b.json)

All current contract/header/source paths inspected are linked above. The handoff
and current minimum request were read first. Prior permitted qualification and
negative-proof results are reused; no repeated tests/hashes/builds or additional
unknown/restricted artifact search occurred.

## Narrow endpoint

One sentence covers the remaining boundary without merging the two findings:

> The exact qualified ioctl must guarantee that its successful returned object
> faithfully reports the terminal results and color capture genuinely generated
> by its fresh current-operation hardware session.

The lineage part is G01′; faithful common-result projection is G02′. An internal
conditional theorem is available; no actual-artifact disproof is available.

The existing [minimum property request](session-response-binding-minimum-property-review-request-20261004-314e2f3b.json)
already asks exactly these semantic questions and remains pending. No narrower
field-only request is justified, and no duplicate request is created. Permitted
artifact analysis is exhausted; there is no evidence that the still-pending
property-confirmation route is forbidden or has produced a denial. Its pending
status is not an authenticated technical confirmation capability either.
Therefore the [contingent design fallback](session-response-binding-independent-design-review-request-20261004-314e2f3b.json)
remains inactive, unapproved and unchanged. No new review gate is invented.

Raw STATUS history, new token, module/UAPI change: NOT DEMONSTRATED necessary.
Design PARTIAL; implementation-review readiness NO; PRE07 live BLOCKED;
execution-authorization readiness NO. Existing approvals unchanged;
sgx_execution_authorized=false; SGX0; task hardware0; triangle NOT ATTEMPTED.
Only this untracked review finding is created. No source/authority/handoff/raw
evidence changed; no stage/commit, implementation, live observation or execution.
