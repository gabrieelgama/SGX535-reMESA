# Same-owner transaction property: compositional finding

Finding: **PROPERTY NOT ESTABLISHED FROM PERMITTED EVIDENCE**.

This is the maintainer-requested offline property review of
[the original bounded question](accepted-outcome-export-property-review-request-20261004-314e2f3b.json).
The current instruction permits this semantic analysis only. No existing rule
requires another approval record merely to read these permitted interfaces;
no new maintainer decision, property approval or live authority is issued here.
The original pending request and all authority/qualification records remain unchanged.

This finding narrows the uncertainty to an exact same-owner transaction
postcondition. It neither asserts that the actual qualified caller violates that
postcondition nor inspects/reconstructs the excluded caller/reporting implementation.

## Proof notation and classified edges

I is the approved client's ioctl, R its local post-call public object, O a kernel
owner, S=O.session, V=O.fixed_service, and B the backend used for the operation.
These are mathematical names for permitted objects, not proposed identifiers/ABI.

PROVEN means a source/qualification guarantee of the named component, subject to
its documented successful-call preconditions; it does not mean live execution.
CONDITIONALLY PROVEN names an additional unestablished caller/runtime premise.
OUTSIDE PERMITTED EVIDENCE is an excluded implementation edge. MISSING denotes
an absent permitted public postcondition/evidence. No edge is DISPROVEN for the
actual candidate transaction.

| Edge | Classification | Exact evidence or additional premise |
| --- | --- | --- |
| E01. Exact approved source/binary implements the public client | PROVEN | [Exact approval](response-export-client-substitution-maintainer-20261004-314e2f3b.json) and retained client qualification; no pins recomputed. |
| E02. Approved process reaches one intended ioctl | CONDITIONALLY PROVEN | [Client](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c#L65) has one call site and no ioctl retry; actual execution, valid CLI, successful device open and observed executable are premises. No execution occurred. |
| E03. That call passes the public local object R | PROVEN | [Client lines60/75-77](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c#L60); retained ABI size4268/image offset172. |
| E04. I selects a fresh O and calls its construction/initialization for this operation | OUTSIDE PERMITTED EVIDENCE | No permitted owner/contract signature takes an ioctl/process/DRM-file identity or enforces this outer selection. |
| E05. Successful owner construction initializes O and S freshly | PROVEN | [Construction](../../kernel/sgx535_frozen/gma500_bo_owner.c#L234) zeroes O at269, clears pages at312 and calls session_begin at345. |
| E06. Constructed V points to S | PROVEN | [Explicit call](../../kernel/sgx535_frozen/gma500_bo_owner.c#L431) passes &O.fixed_service and &O.session; [bind](../../tools/psb-dri-re/frozen_fixed_service.c#L48) stores that session pointer. |
| E07. Successful backend begin binds B.owner=O and checks V.session=&S | PROVEN | [Backend begin](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L383), including pointer check392. |
| E08. The service argument used for I is V, while the callback/status context is B bound to the same O | OUTSIDE PERMITTED EVIDENCE | [run_once signature](../../tools/psb-dri-re/frozen_fixed_service.h#L128) accepts service/context independently. Matching numeric sequences are not pointer-identity proof. |
| E09. Actual backend samples hardware and associates captured words with B's active device/owner | PROVEN | [sample_and_ack](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L58) checks active/power/sequence/current IRQ owner, then reads and consumes status. |
| E10. Those words are current-operation events rather than stale/competing hardware activity | CONDITIONALLY PROVEN | Requires B initialization/lifetime, current baseline, ownership/isolation and continuous unused history; no fresh guards collected. |
| E11. Service accepts supplied status into its own session pointer | PROVEN | [service_status](../../tools/psb-dri-re/frozen_fixed_service.c#L195) passes service->session to contract acceptance at210. |
| E12. That accepting session is S used by B | CONDITIONALLY PROVEN | Follows from E06-E08 if same-owner pairing is established; equality of sequence values alone is insufficient. |
| E13. Fresh S reaches ledger7 only through acceptance of all three classes under qualified helper transitions | PROVEN | [Reset](../../tools/psb-dri-re/frozen_kernel_contract.c#L841), [fire reset](../../tools/psb-dri-re/frozen_kernel_contract.c#L899), [acceptance](../../tools/psb-dri-re/frozen_kernel_contract.c#L926); requires no out-of-contract writes. |
| E14. Completion retires that same service session; accepted ledger survives | PROVEN | [service_retire](../../tools/psb-dri-re/frozen_fixed_service.c#L256) calls session_retire(service->session); [retire](../../tools/psb-dri-re/frozen_kernel_contract.c#L1029) changes phase only. |
| E15. Successful capture_color(O) reads O's color BO under O's retirement gate | PROVEN | [capture_color](../../kernel/sgx535_frozen/gma500_bo_owner.c#L204): initialized/retired/power checks, O.objects[COLOR], cache handling, summary and page copy. |
| E16. The public response fields/image are a faithful projection of S and capture_color(O), with result interpretation from that same operation | OUTSIDE PERMITTED EVIDENCE | No permitted inspected helper accepts sgx535_fixed_ioctl* or specifies this reporting/copy-out mapping. Arbitrary output pointers do not establish it. |
| E17. Existing public contract guarantees E04/E08/E16 and preserves that owner lifetime until response construction | MISSING | UAPI specifies layout/direction, not this same-owner postcondition; reviewed permitted records do not supply it. |
| E18. Returned local R is preserved as the complete raw object | PROVEN | [Client raw export](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c#L100), conditional on successful file preservation; negative ioctl bytes explicitly UNAUTHORITATIVE. |
| E19. Client image export comes from R.color_bytes | PROVEN | [Image write](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c#L119), conditional on success checks and complete file preservation. |
| E20. Retained external files belong to the actual process/I/context | CONDITIONALLY PROVEN | Requires independently observed producer, protected sinks, actual file outcomes and continuity; offline declarations/equality cannot supply live provenance. |

## Smallest inaccessible proposition

**P(I,R): the existing exact qualified boundary enforces one fresh owner lifetime
for I, uses its embedded service with its matching backend, and faithfully returns
that same lifetime's terminal ledger/phase/result and color readback to R.**

This has two precise unresolved clauses:

- G01: the operation for I uses V=&O.fixed_service, V.session=&O.session and B.owner=O, with fresh initialization and valid uninterrupted lifetime.
- G02: R.observed_events and R.phase faithfully report O.session; outcome/operation_errno represent that operation's actual results; R.color_bytes/summary come from capture_color(O), before release/reinitialization can invalidate the association.

This is a semantic postcondition, not a claim about hidden assignments/control
flow. It is not honest to say that only one literal events-field assignment is
missing: the API also leaves the actual service/context pairing to its caller.
The missing public guarantee covers exactly those two links around a proven
component chain. Nothing permitted demonstrates that the actual caller mismatches
them. Failure to prove P is not PROPERTY DISPROVEN.

## Ownership, freshness and ledger proof

O embeds exactly one relevant session and one service. A successful constructor
passes their exact addresses together. Backend begin checks the embedded service's
pointer. Each service acceptance/retirement mutates its supplied session pointer;
there is no lookup of an unrelated global session in these helpers.

However, service/context are independently typed arguments. Generic service binding
can be given an explicit session pointer; exposed C structures are not capability
protection against arbitrary kernel callers. The proof is about qualified helper
transitions and correct pairing, not immunity to memory corruption or unspecified
caller writes. No invocation/process identity is attached by those signatures.

Successful session_begin and enter_fire set observed_events=0. Within the permitted
qualified transition functions, the only nonzero ledger writer ORs an accepted
class bit; duplicate acceptance is rejected. Starting at0, reaching7 therefore
requires successful acceptance of bits1/2/4. TA must precede raster-start gating;
end-render/free require that gate and may appear in either order. Generic phase
advance cannot bypass the completion path: it stops at DEVICE_MAINTAINED.

Retirement leaves the ledger unchanged. Release sets initialized=false and phase
EMPTY, but does not clear every old scalar: an inactive released object's ledger
may still physically equal7. That is **not a freshly initialized valid session**.
Readback rejects an uninitialized/non-retired owner. A successful new constructor
memsets the owner, and a successful session_begin clears the ledger again. Thus
old7 cannot survive into a correctly freshly initialized session without new
acceptance or an out-of-contract write. Whether reporting reads before/after
release, and whether current I selects such a fresh session, are G01/G02.

Service binding resets diagnostics/started via its zero candidate; prepare rejects
a sequential second prepare on the same started service. This is not proof of
atomic ioctl serialization, arbitrary concurrent helper exclusion, or per-module
history enforcement by the excluded caller. Backend pending-status initialization
is also not supplied by session ledger reset; an old status source must not be
confused with a freshly cleared ledger.

## Exclusivity limits

| Competing source | Classification | Scope |
| --- | --- | --- |
| Second successfully claimed frozen owner while first claim held | EXCLUDED BY DESIGN | owner_claim_device is serialized and rejects a second claim. Reuse after release is a different lifetime. |
| Second active fixed IRQ backend owner | EXCLUDED BY DESIGN | backend_begin checks fixed_irq_owner under lock. |
| Second embedded session in this O | EXCLUDED BY DESIGN | O contains one embedded session. Other standalone session/service objects are not globally prohibited. |
| Another standalone session as service argument | NOT EXCLUDED | Generic helper signature permits an explicit session pointer; correct pairing must be established for I. This is not evidence of an actual mismatched call. |
| Another ioctl on same owner / concurrent reporting | UNKNOWN | started prevents sequential preparation reuse; excluded caller serialization/history/copy-out not inspected. |
| Another process sharing a DRM file | UNKNOWN | Owner/backend contain no DRM-file/PID binding; outer file/process policy unavailable. |
| Ordinary eligible GPU producers | NOT EXCLUDED | Frozen-owner claim is not universal GPU producer exclusion; current isolation/ownership/history remains a guard requirement. Normal 2D/aggregate bits are filtered. |
| Stale IRQ/poll words | EXCLUDED ONLY WITH FRESH GUARDS | Internal baseline/source initialization and lifetime must be attributable to I; session reset alone is insufficient. |
| Previous boot or previous invocation artifacts | EXCLUDED ONLY WITH FRESH GUARDS | Requires actual context/history/producer continuity; sequence1 or matching image bytes is not uniqueness. |

## Response fields traced backward

| Public field | Permitted producer/state and validity | Exact stopping point |
| --- | --- | --- |
| observed_events | S.observed_events, reset on begin/fire, acceptance updates, retained through retire | No permitted reporting helper guarantees R.observed_events equals this S at export. Old released storage can retain a number; reporter lifetime/read is not specified. |
| phase | S.phase; service/contract transitions use the supplied session pointer; retirement then release changes it again | Faithful sampling into R.phase before/at correct lifetime is G02. |
| outcome | No corresponding outcome field or UAPI outcome producer in permitted owner/service types | Public expectation2 is a success criterion, not source-to-response mapping. Mapping of actual service/readback results to outcome is unproved. |
| operation_errno | Backend/service/readback return values exist, and service may fold errors; no public field writer in permitted helpers | The actual result-to-field mapping and syscall-versus-service distinction remain unproved. |
| color_bytes | capture_color(O) selects O's color BO, checks O.session retirement/power/pages, handles cache and copies PAGE_SIZE to supplied raw_bytes | Output is u8*, not a typed UAPI field; whether the chosen O is I's O and destination is R.color_bytes requires G01/G02. |
| color_observed/checksum/counts/rows | capture_color's summary output describes the same mapped page as its raw copy on success | Mapping into R and association with its ledger/outcome requires G02. Summaries do not prove GPU provenance. |

Image lineage is locally stronger than a generic buffer copy: helper selection is
explicitly same-owner and retirement-gated. It still cannot identify its caller's
owner or response destination. Permitted helpers could be supplied another valid
owner/output pointer; no claim is made that actual entry code does so. One live
claimed owner constrains simultaneous constructed owners, but does not specify
which retained lifetime is reported. Pixel agreement cannot prove this mapping.

UAPI and session are distinct structures, not aliases. Same field names do not
make a public response refer to embedded session storage. A faithful projection
postcondition is needed; neither layout direction nor the client pointer implies it.

## Permitted cross-checks exhausted

- [Service tests](../../tools/psb-dri-re/test_frozen_fixed_service.c#L143) explicitly bind a local session; their [status source](../../tools/psb-dri-re/test_frozen_fixed_service.c#L32) supplies synthetic event bits. The [build inputs](../../tools/psb-dri-re/test_frozen_fixed_service.py#L14) exercise service/contract, not kernel ioctl reporting.
- [Contract tests](../../tools/psb-dri-re/test_frozen_kernel_contract.c#L386) cover sequence/order/retire/HOLD invariants. The copied-session test at [service test line212](../../tools/psb-dri-re/test_frozen_fixed_service.c#L212) rejects a mismatched numeric sequence; it does not prove unique pointer identity for equal sequence values or response mapping.
- [Response exporter tests](../../tools/psb-dri-re/test_frozen_triangle_response_export.py#L66) inject the public response in controlled_ioctl, then test byte preservation. Perfect synthetic fields do not originate in a kernel owner. Retained paired evidence proves post-call exporter fidelity and unchanged pre-call semantics, not G01/G02.
- ABI qualification proves4268 bytes, offsets and ioctl number. It says nothing about owner selection or semantic field origin.
- The offline scene-owner harness is explicitly separate from ioctl integration; source-manifest scene_owner_in_module=false. Its owner/lifecycle tests cannot establish the candidate kernel reporting edge.
- Candidate source/qualification metadata pins the components but is not a substitute for an interface postcondition. Existing zero-invocation70/70 first-owner records contain no kernel response from this action and cannot supply G01/G02.

Existing records reused, not requalified:
`/home/gama/sgx535-offline/request-byte-decoding-20261002T222753Z/source-manifest.json`;
`/home/gama/sgx535-offline/phase8-response-export-20261004T092750Z/final/client-qualification.json`;
`/home/gama/sgx535-offline/phase8-response-export-20261004T092750Z/final/SYNTHETIC-paired-evidence/results.json`.
The [earlier proof review](accepted-outcome-export-proof-review-20261004.md) and
[clean-room scope](pre07-clean-room-review-scope-20261004.md) define evidence and
exclusion boundaries. No restricted implementation/integration tests, caller
source, disassembly or alternate representation were consulted.

## Narrow continuation, not implementation

[The reduced property request](session-response-binding-property-review-request-20261004-314e2f3b.json)
asks only whether P, specifically G01 and G02, is guaranteed by independently
permitted evidence for this exact artifact. It requests no hidden assignments,
source/control flow or forbidden access. No current property-confirmation capability
is demonstrated; an authorized assertion without technical support is insufficient.

Because the available permitted proof sources do not supply P, a separate
[contingent design-only request](session-response-binding-independent-design-review-request-20261004-314e2f3b.json)
prepares the permitted fallback if no legitimate property confirmation becomes
available. It is pending, not automatic approval or proof of a necessary patch.
Its sole proposed subject is an independently specified same-owner outcome/response
postcondition and whether permitted interfaces can realize it; no replacement ioctl
entry, executor, restricted reporter, raw journal, new token or ABI is selected.

Actual property DISPROVEN: NO. Module/UAPI/new token necessary: NOT YET DEMONSTRATED.
Raw STATUS history: not inherently required if qualified acceptance and P plus
current hardware/producer continuity are established. No semantic correction is
justified for implementation. A future proof of inadequacy would require separate
design review of faithful same-session projection; changed components alone would
invalidate their own byte/semantic qualification. Unchanged client/UAPI/geometry,
module/image and historical70/70 retain their original scopes now.

Design PARTIAL; READY FOR IMPLEMENTATION REVIEW NO for a complete design; PRE07
live BLOCKED; READY FOR EXECUTION AUTHORIZATION NO. Property closure alone would
still require applicable current-context guard/transaction design and live evidence.
Gate B and exact client approval unchanged; sgx_execution_authorized=false;
SGX invocations0; task hardware interactions0; triangle NOT ATTEMPTED; no new
HOLD/recovery. No patches, builds, tests, hashes, staging, contact or invocation.

Only this finding and its two new pending requests are created. No earlier record,
qualified artifact, handoff, authority or raw evidence is edited. Files remain
untracked; no stage/commit. Documentation consistency checks are separate from
qualification or live proof.
