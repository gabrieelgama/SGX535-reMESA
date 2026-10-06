# Current-operation result provenance: independent design

This is a specification, not implemented instrumentation or a live procedure.
The [maintainer decision](current-operation-result-provenance-design-scope-maintainer-20261005-314e2f3b.json)
authorizes only this offline design. The historical G01′/G02′ findings remain
UNRESOLVED, with their current confirmation routes EXHAUSTED. Nothing here
certifies the current candidate's hidden reporting behavior or disproves it.

Intent: prove that one preserved authoritative result R belongs to one current
hardware operation T. Preserve the approved client, command/request semantics,
qualified acceptance rules and readback rules wherever possible. No restricted
implementation, alternate representation, historical wrapper ordering or hidden
assignments are inputs. No launcher, reporter replacement or GPU control path is
designed. All proposed producer/observation mechanisms below are NEW requirements
for independent implementation review, not assertions of existing availability.

## Information before representation

T is a single eligible operation lifetime within one independently observed,
separately authorized invocation of the exact approved client and fixed ioctl.
It begins only after current-context verification and a fresh, one-use observation
instance is ready. It ends with a terminal capture or an explicitly incomplete
outcome. Ownership may subsequently be released; a valid earlier snapshot remains
valid. T is not defined by a pointer, PID, UUID, numeric sequence or matching image.

E(T) is the terminal accepted-event state generated from qualified hardware
observations for T. S(T) is its actual terminal service result and phase. C(T) is
the immutable color capture made from that operation's storage while the qualified
retirement/readback preconditions hold. A failed or unavailable capture is absent,
not a zero image. Accepted-event ordering is the qualified contract's ordering;
there is no additional total ordering of otherwise unordered completion classes.

R is a preserved evidence bundle with a producer-originated immutable capsule,
the approved client's exact 4268-byte local returned object, its original image
when produced, streams/outcome and context/observation receipts. The capsule is
the new authoritative provenance anchor. The legacy response alone remains
insufficient. Its preservation does not become a proof of hidden copy-out logic.

`R belongs to T` requires a fresh authenticated observation of the current call,
one eligible operation admitted in that observation lifetime, capsule creation
from that operation's genuine accepted state and color storage, and faithful
preservation of that capsule. For a complete successful bundle, the relevant
response fields and all 4096 color bytes must agree with the capsule. Agreement
corroborates the represented results of T; it does not retrospectively prove
the legacy reporter's implementation or change G01′/G02′ findings.

The minimum information is: current context and eligible-call lifetime; one
non-reusable operation admission; hardware-source/acceptance lineage; common
terminal/color ownership; validity/completeness; and materialization integrity.
A globally unique identifier is not necessary. A label or hash is insufficient.

## Four architectures

| Criterion | A: existing pieces only | B: one-use operation-owned immutable capsule | C: internal generation plus capsule | D: exported discriminator plus capsule |
| --- | --- | --- | --- | --- |
| Common provenance | Not established: reuses missing guarantee | Sufficient by new construction and admission invariants below | Sufficient only with B's origin invariants | Sufficient only with B's origin invariants |
| New implementation complexity | Low assembly cost, but cannot prove relation | Moderate: observer, bounded capsule producer and read-only materialization | B plus generation allocation/reuse rules | B plus external identifier/association rules |
| Qualification complexity | Existing checks cannot fill proof gap | New producer/boundary/retention integration qualification | B plus wrap/reset/reuse qualification | C plus external identity transport and replay qualification |
| Module change | None, but rejected | Yes for the proposed permitted kernel-side producer instrumentation | Yes for proposed internal producer | Yes for proposed kernel producer/export |
| Fixed triangle UAPI change | No | No required expansion; supplemental export contract still needs review | No required expansion; export still needed | Conditional: separate record can avoid altering fixed response layout |
| Approved client change | No | No | No, if independent read-only collection | Conditional if discriminator were required in its ABI; not selected |
| New operation identifier | No | No: one-use lifetime is discriminator | Yes, local generation | Yes, externally represented discriminator |
| Failure robustness | Lost provenance remains UNKNOWN | Kernel-owned retention independent of client; losses remain UNKNOWN | B plus identifier failure modes | B plus identifier/copy-out failure modes |
| Preservation robustness | Archive integrity only | Existing archive plus qualified capsule materialization | Same as B | Same as B, plus identifier consistency |
| Color/event common origin | Not proved | Construction gates both through one admitted lifetime | Identifier equality alone insufficient | Identifier equality alone insufficient |
| Existing qualification reuse | All pieces unchanged, but inadequate | Existing semantic components and client reused; changed artifact must qualify | B plus generation coverage | B plus export/ABI/client coverage where changed |

A is rejected as a sufficient architecture. Exclusivity and fresh files do not
authenticate which state was projected into an otherwise plausible response.
C and D add naming machinery but cannot repair a producer that combines unrelated
values. B is selected: it adds the missing independently reviewable construction
and observation relation without adding an operation number or raw-status journal.

## Selected B: passive observation and structural capture

The new design has three bounded responsibilities, plus the existing archive.
They must never issue GPU work, change request bytes, retry, reset or recover.

1. **Invocation/lifecycle observer.** Independently observe the public approved
   process/ioctl lifetime, and eligible operation admissions in permitted backend
   interfaces. This requires a qualified observation of the actual call, not a
   shell timestamp or caller-declared PID. The observation must reject process
   lifetime reuse and establish the approved executable/device/request identity.
   No particular Linux observation facility is selected or claimed available.
   The public boundary and permitted backend observations must be connected by
   the same valid calling lifetime, or another independently qualified causal
   relation. Unprovable/asynchronous association yields UNKNOWN. No excluded
   entry/reporting control flow may supply the relation.
2. **Operation-owned evidence producer.** At an admitted permitted backend
   lifetime, create a private bounded evidence object. Only matched real hardware
   observations and qualified acceptance transitions for that lifetime may supply
   accepted state. At terminal retirement/readback, capture actual phase/service
   result and color from the same admitted lifetime. The constructor cannot accept
   unrelated public response scalars or an arbitrary old image as authoritative
   inputs. It validates source/lifetime coupling; mismatches invalidate evidence
   without changing GPU commands or claiming no issuance. Actual integration is
   a new qualification obligation, not an assumed property of the old caller.
3. **Read-only materializer.** Retain and deliver the immutable capsule independently
   of the client's survival and owner release. A future export interface must bind
   a fresh current observation instance, reject old-instance substitution, report
   incomplete/lost records, and provide complete readback verification. No concrete
   transport, pathname, ABI, log emitter or installed interface is selected here.
   This is a separate evidence channel, not a replacement for the existing ioctl
   response projection or an executor. A channel carrying declared metadata alone
   is inadequate.
4. **Existing offline archive.** Preserve originals, verify actual saved bytes,
   make validation copies only afterwards, and retain partial evidence on failure.
   Its hashes authenticate integrity relative to supplied files, not kernel origin.

The observer instance follows a monotonic lifetime:

`UNUSED -> READY -> ACTIVE(T) -> TERMINAL_CAPTURED -> CLOSED -> PRESERVED`

The minimum causal admission rule is explicit: the permitted backend lifetime
must begin in the independently observed active public ioctl lifetime of the
approved process. The observation uses an OS-managed call/process lifetime, not
PID equality alone. A producer source carries its private operation lease into
the permitted acceptance/capture observations; an unrelated session or context
cannot acquire that lease merely by supplying equal sequence values. For the
minimum design, an unconnected asynchronous operation is not admissible. A future
extension for asynchronous lineage would need its own proof and review. This rule
does not require learning how excluded entry code chooses its arguments.

Any ambiguous admission, second eligible operation, context break, source mismatch,
loss or incomplete observation yields `INCOMPLETE`, never another T. It cannot be
reset/rearmed after possible issuance within this observation/boot attempt. This
is an observer design condition, not a new retry permission or a claim that current
module one-shot history is sufficient. A later boot requires a new separately
reviewed context; this design does not authorize one.

Terminal payload is immutable at capture. Validity is published only after the
call interval closes and absence of a second eligible operation is established.
If a later observation invalidates the interval, retain the original payload plus
an append-only invalidity receipt; never rewrite it into a different transaction.
Absence of a second operation requires observation completeness, not just a count.
Persistent observer state must outlive owner storage and client exit. Owner release
does not release a captured image. No capsule is declared durable merely because
it is immutable in RAM.

This removes pointer permanence as a requirement. Internally, structural coupling
must be enforced, not merely commented: evidence creation is private to the one
admitted lifetime, accepted-state inputs carry a verified hardware-sample origin,
and capture reads the same lifetime's retired color storage. Copies preserve that
relation only if made while valid and never rebound. These are concrete producer
construction invariants to qualify, rather than assumptions about excluded code.

## Why B proves the relation

I1. A qualified fresh call observer admits exactly one eligible operation T,
under the actual approved invocation and continuous context; uncertainty invalidates
the interval. I2. Only that admission can create its private capsule. I3. The
accepted-event writer accepts only qualified hardware-origin contributions for T;
unmatched, stale or alternate transitions cannot populate authoritative evidence.
I4. The terminal writer reads actual service result/phase of T. I5. Color capture
uses T's storage while retirement/readback conditions hold and copies it into that
capsule before source invalidation. I6. No different operation can write the capsule.
I7. Closure, immutable retention and trusted materialization preserve its content
and fresh-instance origin. I8. The archive verifies complete retained bytes.

From I1–I3, capsule events are E(T). From I2/I4/I6, its terminal state is S(T).
From I2/I5/I6, its color is C(T). I7–I8 preserve these three facts in R. Therefore
R's authoritative capsule belongs to T. The returned client object is accepted
as corroborating these represented results only after exact field/image agreement.
No correctness premise is borrowed from G01′/G02′ or hidden reporter behavior.

Every I is a future implementation proof obligation. A field named `valid`, a
unique ID or an offline synthetic capsule does not prove I1–I8. The architecture
is sufficient as a specification; no current implementation is certified.

## Freshness and the limits of exclusivity

Exclusivity replaces an exported operation identifier only when combined with
the observer's non-reusable lifetime and qualified producer construction. It
does not replace hardware/source freshness. Required conditions include:

- Actual current boot/build/client/action and continuous approved call identity.
- Unused observer and applicable one-shot history; no prior eligible operation.
- Fresh session and hardware/software pending state, including prior delayed-event
  quiescence. Numeric sequence equality and a clean register snapshot alone are
  insufficient. If the required quiescence cannot be established, UNKNOWN.
- Continuous exclusion of unrelated eligible GPU producers; frozen-owner claim
  alone is not universal producer exclusion.
- Complete observation of admissions, accepted-source transitions and color capture.

These are not observed now. No public UAPI field is invented to express them.
The trusted computing base includes the qualified producer/observer, actual
loaded-artifact attribution, OS observation path and protected preservation.
This is not remote attestation or proof against a malicious privileged kernel.

## Ledger and color

The existing accepted ledger is sufficient for completion classes once genuine
hardware-origin acceptance and T provenance are proved. Preserve the qualified
acceptance/order rules rather than create new hardware meanings. Raw STATUS
history is optional diagnostic richness and is not selected.

Store the complete bounded 4096-byte color capture in the immutable capsule.
A digest alone cannot establish C(T). A protected separately retained full copy
could suffice with qualified copy provenance, but adds a second lifetime/link;
including bytes is the simpler bounded design. Summaries are derived from those
bytes and cannot be supplied independently. Preserve the client-exported ORIGINAL
unchanged and compare all its bytes with the capsule capture. A matching synthetic
triangle is never an authoritative producer. GPU-to-CPU visibility and raster edge
conventions remain separately qualified/observed questions; provenance does not
prove a triangle's geometry or correct memory coherency.

## Adversarial comparison

Codes: A lacks a sufficient proof; B rejects invalidity or retains UNKNOWN by its
construction rules; C requires all B rules plus safe generation semantics; D
requires all B rules plus authenticated export/replay separation. These are design
obligations, not executed test results.

| Attack | A | B response | Additional C/D obligation |
| --- | --- | --- | --- |
| R from T-1 presented as T | No provenance exclusion | Fresh one-use channel must reject prior capsule; otherwise UNKNOWN | Reject reused/replayed generation or external identifier |
| Events T1 + color T2 | Values can agree | Private constructor/capture rejects different lifetime | Equal tags alone cannot justify acceptance |
| Stale ledger | Reset requirement does not prove reported origin | Hardware-origin writer and fresh instance; mismatch invalid | Same as B |
| Stale image | Appearance/hash insufficient | Capture only current retired storage, freeze full bytes | Same as B |
| Copied session | Possible alternate accepted history | Faithful T snapshot allowed; alternate lineage cannot write capsule | Generation on a copy does not establish origin |
| Reused discriminator | No producer proof | No operation discriminator; instance cannot be rearmed/replayed | Define wrap/reset/nonreuse or reject reuse |
| Delayed prior event | Bounded window insufficient | Require source freshness/quiescence; UNKNOWN if unavailable | Token does not identify the cause of hardware status |
| Competing producer | Generic owner claim insufficient | Continuous isolation or UNKNOWN; source mismatch invalid | Same as B |
| Crash after issuance | Export may be absent | Retain observer/payload/streams available; issuance possible, no retry | Same as B |
| Crash after terminal before preservation | Existing export may be lost | Independent capsule retained; absent durable delivery is incomplete | Same as B |
| Partial response | Tuple may be misleading | Retain prefix; no attributable terminal-success bundle | Same as B plus discriminator truncation |
| Copy-out failure | Failed local bytes untrusted | Failed bytes remain UNAUTHORITATIVE; capsule supports only independent facts | Same as B |
| Owner release before preservation | Saved origin unproved | Frozen capsule/color survive; release before valid capture is incomplete | Same as B |

No architecture can eliminate real evidence loss. Sufficiency means loss cannot
be mistaken for authoritative success. If any required exclusion/observation is
unavailable, B does not claim completion and cannot yield TRIANGLE ESTABLISHED.

## Result and failure contract

| Class | Minimum supported facts | Missing evidence / behavior |
| --- | --- | --- |
| SUCCESS — ATTRIBUTABLE TERMINAL RESULT | Qualified closed capsule belongs to T; required accepted classes, terminal state, retired capture and complete agreeing client artifacts are preserved | This is not automatically TRIANGLE ESTABLISHED; full image/geometry and visibility evidence still required |
| EXECUTION OBSERVED — RESULT INCOMPLETE | Independent capsule/observation establishes specific execution facts | Missing terminal/color/response/preservation facts remain UNKNOWN; no retry |
| READBACK PRESENT — EXECUTION ATTRIBUTION INCOMPLETE | Actual retained readback bytes | No common transaction proof; no triangle or completion claim; no retry after possible issuance |
| FAILURE/HOLD — ATTRIBUTABLE | Valid T-linked failure/HOLD observation | Preserve failure and partial artifacts; no invented completion or hot recovery |
| ISSUANCE POSSIBLE — ATTRIBUTION LOST | Call may have reached device; adequate association/reporting absent | STOP; no retry; reviewed recovery boundary only |
| UNKNOWN | Evidence cannot support a narrower claim | Preserve available evidence; never treat zero fields as success |

Negative ioctl return does not make its local response authoritative. An independent
valid capsule can still support a narrower observed execution/failure fact, but
this design does not promote failed copy-out into a successful returned response.

| Loss case | Retained evidence / classification |
| --- | --- |
| Process crash | Producer capsule if captured and retained; partial streams/process observations. Terminal bundle incomplete if response missing; no retry. |
| Kernel-side failure/HOLD | Attributable capsule if construction completed; otherwise available partial observations. No fabricated terminal image. |
| Copy-out failure | Independent capsule plus explicitly UNAUTHORITATIVE local bytes; execution/failure facts only where independently supported. |
| Preservation failure | Preserve partial files and in-memory capsule where still available; no seal certifies missing bytes; STOP/no retry. |
| Partial capsule | Explicit incomplete record; no valid publication; UNKNOWN for missing fields. |
| Power loss | Only actually durable prior evidence survives; lost RAM/continuity cannot be inferred from a manifest. |
| Hang | Available admission/partial outcome evidence; terminal completion remains UNKNOWN; no second invocation or automatic recovery. |
| Ambiguous issuance | Retain all available observations; never classify NOT ATTEMPTED merely because no complete result appeared; no retry. |

Kernel retention is independent of process crashes, not guaranteed against power
loss or kernel failure. No recovery algorithm, unload/reset or automatic retry is
part of this design. The existing reviewed recovery boundary remains separate.

## Offline qualification plan, not executed

Qualify the new observation/capsule producer from permitted primary source and
contracts. Tests alone cannot replace a structural proof that its inputs and
materialization have the claimed origin.

1. Proof/review of admission coverage and actual public call-to-backend association,
   including async/unmatched activity rejection; no hidden caller assumptions.
2. Freshness: blank initial state, session/software pending reset requirements,
   old-boot replay, stale source and delayed-event exclusion/UNKNOWN behavior.
3. Exactly one eligible admission: duplicate begin, concurrent operation, same-owner
   restart and context drift invalidate evidence without another execution.
4. Cross-lifetime ledger/phase/result/color mixing: every permutation must reject
   evidence; faithful valid snapshots must remain accepted after source release.
5. Hardware-origin enforcement: arbitrary software event/scalar input cannot create
   authoritative completion; preserve existing qualified acceptance/order rules.
6. All original color bytes, summaries and response comparisons; mismatched producer,
   image, field, response length and partial byte streams cannot certify success.
7. Crash/copy-out/producer/preservation failure injection at every transition;
   retain partial evidence, no seal upgrade, no retry and no false NOT ATTEMPTED.
8. Retention/materialization: immutable terminal payload, invalidity receipt,
   process-independent lifetime, stale-channel rejection, exclusive creation,
   write/sync/close/readback failure and originals-before-copy ordering.
9. C/D only: wrap/reuse/reset and identifier transport tests. B must instead prove
   non-rearmable observer lifetime; no unnecessary identifier suite for B.
10. Changed-artifact reproducibility, ABI/import/build qualification and exact
    identity records where dependency requires them; no unrelated requalification.

Synthetic qualification can establish implementation logic, not live hardware
completion, coherency, actual fresh guards or triangle existence. These remain
separate future observations under applicable authority.

## Dependency and review boundary

For the selected proposed producer, a future module implementation change is
required. This is an implementation choice of B, not a finding that the current
candidate has a defect or lacks some excluded emitter. The old qualified module
is untouched and remains qualified for its historical identity. A future new
module changes its identity; an image embedding it also changes. Requalification,
candidate-specific first-owner evidence and readiness/authority rebinding would
then apply only where required by those changed identities. No old PASS transfers
to changed bytes automatically.

The fixed triangle request/response layout need not expand. A supplemental
read-only evidence export interface is required by B; its representation/ABI and
permitted integration must be selected and reviewed before implementation. Thus
UAPI impact is CONDITIONAL, not an assumed free existing interface. Approved client
source/binary need not change; a separate non-executing evidence reader/checker is
new work. It must not become a substitute invocation/reporting wrapper.

Geometry, request construction, approved client's one-call/export qualification,
unchanged public layout, qualified acceptance/readback semantics, existing archive
code and guard predicates remain reusable within their scopes. New producer,
observer, export, integration and any new checker require their own qualification.
Existing first-owner records remain valid historical observations of the old build;
they are neither fresh guards nor observations of a future changed build.

The next review must assess this specification, contamination boundary, public
call-to-producer observation feasibility and proposed permitted integration before
granting any implementation scope. If feasibility requires restricted material or
replacement of a blocked operation, STOP that branch; this document grants no
exception. It is a reviewable sufficient architecture, not a demonstrated working
route on the existing Mini 12 or current module.

Design status: COMPLETE — READY FOR IMPLEMENTATION REVIEW, at specification level.
Implementation authorized: false. PRE07 live readiness: BLOCKED. Execution
authorization readiness: NO. Current Gate B/client decisions unchanged;
sgx_execution_authorized=false; SGX invocations0; hardware interactions0;
triangle NOT ATTEMPTED. No test, patch, build, restricted-source read or live action.
