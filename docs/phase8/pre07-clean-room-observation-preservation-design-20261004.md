# Clean-room PRE07 observation/preservation design — 2026-10-04

**DESIGN PARTIAL — BLOCKED ON DOCUMENTED INVOCATION-ATTRIBUTABLE KERNEL OUTCOME OBSERVABLE.**
**READY FOR IMPLEMENTATION REVIEW: NO** for a complete live procedure. The partial
requirements/specification sections are reviewable. No implementation/live use is
approved; no executor, collector code or target commands are supplied.

Authority: [new prospective design-only maintainer decision](pre07-clean-room-design-scope-maintainer-20261004-314e2f3b.json).
It binds the unchanged [request](pre07-clean-room-observation-preservation-design-review-request-20261004.json).
[Machine-readable contract](pre07-clean-room-design-contract-20261004-314e2f3b.json)
contains the exact context/client pins, full six-model proof matrix (15 questions per
model), confounders, destination/guard contracts and failure/claim tables.

## Clean-room limits

Only the approved client's public interface, public UAPI, current approved identity
and evidence requirements, offline archive/checker, exclusive file primitives,
unrestricted passive predicates and ordinary local Linux documentation informed this
specification. FORBIDDEN/UNKNOWN source content was not used. Ordinary documentation
is not target availability or live authority. No isolated-author/erased-memory claim
is made; past restricted knowledge is excluded as a specification.

Any dependency requiring restricted or unclassified material stops that branch.
No old wrapper operation ordering, paths, commands, correlation internals or kernel
entry source is consulted. A different name/codebase would not establish independence.
This design neither lifts a restriction nor recreates the blocked operation.

## Architecture and trust boundary

A. **Pre-action observation/preparation:** require actual fresh context, continuity,
ownership/health/history/privilege/recovery observations; independently bind protected
roles/sinks and observation readiness. Failure or missing required evidence means STOP.
These are requirements, not performed observations or a collector implementation.

B. **Separately authorized approved client:** ordinary externally controlled execution
of the existing approved binary is the invocation primitive. Its public source and
retained qualification bound one ioctl if reached, with no retry. This specification
does not launch it, construct another ioctl, add a launcher, trace/stop its execution
or alter request bytes. An independently permitted process/sink/outcome witness would
enter the observation boundary; its actual availability is not asserted.

C. **Post-action observation/preservation:** preserve every available original response,
image, stream, kernel record and process/capture/failure outcome before interpretation.
Capture missing roles, partials and continuity loss explicitly. No replay or retry.
Kernel HOLD is claimed only if directly observed, not inferred from an observer STOP.

D. **Offline verification/sealing:** checked original preservation precedes copies;
reread/hash/manifest/independent verification and full-image analysis operate on copies.
The existing archive's false `triangle_established` and UNKNOWN provenance remain
unchanged. A separate future evidence judgment—not a manifest—would evaluate the
formal attribution and image obligations. No new classifier code is provided. The approved client attempts raw preservation
before success summary/image handling; a raw-write failure may still leave image or
other evidence, which must be retained rather than discarded.

Assumptions must be explicit: independently reviewed observation provenance, trusted
OS/artifact access boundaries and exact original bytes. This is not authenticity
against a malicious privileged host or a power-loss-survival claim. Private evidence
storage does not establish exclusive GPU activity.

## Correlation proof obligation

Let C be the exact approved context, P an actually observed approved executable/process
instance, I its unique eligible ioctl, R the retained post-call response, O the image
if produced, K a kernel record and E the retained observations.

`Attr_E(K,I,R,C)` may be CONFIRMED only if E establishes:

1. Actual context and boot/owner continuity, not copied labels.
2. Approved executable/process-instance identity and the uniquely eligible actual I.
3. Producer/sink provenance linking R (and O) to that P/I plus unchanged original bytes.
4. Documented accepted-event semantics for K; a ledger bit or text match alone is not K.
5. Sufficient evidence completeness and exclusion of stale/unrelated/other-producer events.
6. A documented causal association K→I **or** an independently proved exclusive-current-
   operation association under the reviewed observation model.

There must be exactly one eligible invocation/event history consistent with the claim.
Every premise needs a permitted observation or interface specification. Missing premises
make attribution UNKNOWN, not success or demonstrated failure. A unique token is not
imposed; causal/exclusion proof is required regardless of representation. Mere temporal
proximity is not a deterministic attribution proof.

## Correlation models

| Model | Classification | What it contributes | Why it is insufficient or unresolved |
|---|---|---|---|
| A: one invocation + window | INSUFFICIENT | Bounds intent/order if observed | Does not exclude delayed/other events or establish actual producer |
| B: process identity + one invocation + window | INSUFFICIENT | Identifies observed process instance conditionally | PID/lifetime must be proven; asynchronous kernel events lack established causal bridge |
| C: process + clocks + window | INSUFFICIENT | Stronger ordering with known clock domains | Ordering is not causation; concurrent/delayed events remain possible |
| D: existing exported sequence/ledger | NOT AVAILABLE | Public tuple/event bits exist | UAPI has no invocation sequence; ledger is not identity; other unclassified interfaces excluded |
| E: existing accepted kernel marker | UNKNOWN | Could provide event semantics/provenance | No permitted current accepted-event schema and association guarantee is established |
| F: observer-only correlation metadata | INSUFFICIENT | Binds actual witnessed context/inodes/outcomes | Labels/receipts do not create missing accepted hardware observation |

Best conditional architecture: **F+B+C with a proved E observation/exclusion bridge**,
POTENTIALLY SUFFICIENT—NEEDS SPECIFIC PROOF. It is not an accepted route today.
No model is classified SUFFICIENT. Success and failure/HOLD capture adequacy remain
unestablished; all allowed models share the same event-provenance gap.

## Kernel observability and exact stopped branch

**SUCCESS ATTRIBUTION OBSERVABILITY: UNKNOWN.** The public response/ledger identifies
reported stages but cannot independently establish physical GPU completion. Ordinary
kernel-log documentation describes retained ring-buffer visibility, not the frozen
driver's accepted completion markers or their invocation association. Its read-all
interface is a bounded retained buffer; completeness/drop exclusion is not inferred
from taking before/after snapshots or a text-prefix match. No log interface was used.

The missing observable is **a permitted, documented kernel outcome observation that
can be distinguished as accepted TA/end-render/3D-memory-free or failure/HOLD of this
current operation, with provenance sufficient for a causal or independently proved
exclusive-current-operation association**. Its actual emission/schema/availability is
UNKNOWN. The design branch stops here; no marker, token, collector or kernel change
is invented. Whether a kernel/module change is necessary remains UNKNOWN.

## Confounders and exclusion obligations

| Confounder | Independently required evidence |
|---|---|
| Stale kernel records | Observed pre-action baseline and a documented way to identify records belonging to the current action, excluding delayed/pending earlier work. A text prefix alone is insufficient. |
| Unrelated kernel events | Accepted-event semantic classification and either causal association or proof no alternative producer can emit the accepted event in the interval. |
| Another process | Stable approved executable/process-instance witness and independently observed/excluded competing device activity. Private files do not prove exclusive GPU access. |
| Another ioctl or previous invocation | One-call client semantics plus complete observed invocation history and uniqueness of the actually observed eligible operation; no availability probe. |
| Previous boot or boot change | Actual same expected boot before, immediately before action and after collection, with evidence the collection interval has no unseen boot transition. |
| Missing completion | Preserve absence/timeout/incomplete return as UNKNOWN; no inferred event from success-looking tuple/image. |
| Incomplete/dropped/cleared logs | Documented source continuity/loss detection or otherwise sufficient retained event evidence; a partial interval may not support exclusion. |
| Clock discontinuity/reordering | Clock domain/source, monotonic ordering/resolution and continuity witness, or an independently sufficient non-time causal order. |
| PID reuse | Process-instance lifetime discriminator/stable witness, not bare PID; relate observations to the same lifetime. |
| Client crash | Independent process outcome and last directly observed action boundary; any possible ioctl means no retry and execution UNKNOWN unless narrower direct evidence exists. |
| Artifact preservation failure | Original bytes/partial bytes, actual syscall/write/sync/close/capture outcome and inode provenance, with failed/missing roles explicitly listed. |
| Artifact substitution or wrong producer | Trusted parent/sink binding, sole authorized writer, observed executable/lifetime and inode receipts plus reread bytes; hash alone proves no producer. |
| Ownership change | Observed module/PCI/device ownership through relevant interval; change or uncertain continuity stops attribution. |
| Duplicate or reordered event records | Preserved source ordering/identity and documented accepted-event semantics; duplicates do not count as additional completion and a later sample is not event history. |

These are proof obligations, not a claim the target exposes the required facts.
Their present availability/limits are recorded individually in the JSON contract.

## Protected destinations

Only logical roles are specified; actual Mini 12 paths are **null**. A future review
must bind trusted parent ancestry/identity/owner and each distinct basename, with no
untrusted writer or path alias. Evidence directories require private mode0700 and
regular evidence files mode0600/nlink1; current privilege predicate is euid0. Client
executable protection/permissions are separate from evidence-file permissions.

Use existing exclusive/no-overwrite/type/link/ownership primitives within scope.
Response/image files MUST remain absent before approved client creation; precreating
them would conflict with its exclusive writes. Stream/receipt sinks may be exclusively
prepared only when their binding to the externally authorized process is proved.
No shell redirection, launcher or destination is invented in this specification.

Retain inode/identity receipts where observable, check write/fsync/close and parent
sync, reopen without following links, verify regular/single-link/owner/inode/size and
reread bytes. Full response is4268; image is4096 at response offset172. Preserve all
available originals before copies; sync originals before derivation. Existing hashes
and seals establish integrity only. On failure retain partials/errors without deleting,
overwriting, reusing paths or manufacturing missing original files. Storage failure
may prevent a complete receipt; successful sync is not arbitrary power-loss proof.

## Fresh guard contract

| Class | When | Required observations | Role |
|---|---|---|---|
| PRECONDITION | Fresh after preparation and before action | Exact approved context/client/module Live, ownership/services/health/taint, unused history, privilege/operator/recovery, protected sinks, sufficient observation readiness, separate execution permission | Identity/history/capture support attribution; all also enforce readiness |
| CONTINUITY CONDITION | Through preparation/action/collection; immediately before action | Same boot/owner/process/sinks, ordering/source continuity, complete history and no competing eligible action | Attribution and safety; a copied snapshot cannot establish continuity |
| POSTCONDITION | After action/abnormal end; preserve before analysis | Actual process/capture outcome, original roles/bytes/inodes, response-image consistency, available kernel events/completeness and relevant current state | Evidence/outcome; no repeated full first-owner sweep mandated |
| RECOVERY CONDITION | Ready before action; review after failure if needed | Operator controls and existing separately authorized manual machine-boundary/STOCK procedure | Safety only; no auto reset, hot recovery or execution proof |

No observations collected. UNKNOWN/failed attribution prerequisites prevent a successful
claim; missing pre-action conditions prevent invocation. Post-action uncertainty never
licenses a retry. Actual hardware HOLD and observer STOP remain distinct.

## Failure and classification table

| Case | Evidence retained | Classification | Retry / recovery |
|---|---|---|---|
| Pre-action guard/destination/capture readiness failure before directly established invocation | Preserve failed guard and preparation/partial receipts. | NOT ATTEMPTED only if no client/ioctl issuance is directly established; otherwise EXECUTION UNKNOWN. | No automatic retry. Review failed readiness; no recovery action authorized. |
| ioctl negative return | Retain full raw userspace object if saved, stderr hex, syscall error/outcome, streams and available kernel records. | Failed syscall bytes UNAUTHORITATIVE; EXECUTION UNKNOWN unless independent events localize a boundary. | DO NOT RETRY. STOP; reviewed recovery only if needed; driver HOLD only if observed. |
| Response preservation failure | Retain partial raw file, write/sync/close errors, stdout/stderr and other available artifacts. | Complete response unestablished; EXECUTION UNKNOWN or narrower independently proved execution state; no triangle. | DO NOT RETRY if ioctl may have occurred. STOP; recovery review if execution may have occurred. |
| Image preservation failure | Retain complete response if available, partial image, streams/error/inode receipts and kernel observations. | Image may exist within response but standalone original export is incomplete; preserve unchanged evidence, no invented successful original or triangle. | DO NOT RETRY. STOP; recovery review if needed; no extraction/redesign approved here. |
| stdout/stderr collection failure | Retain stream partials, capture errors, process outcome, response/image and kernel records. | Missing failure/report evidence stays UNKNOWN; complete transaction not certified. | DO NOT RETRY if issuance possible. STOP; recovery review if needed. |
| Kernel record loss/incomplete observation | Retain before/after/partial records and explicit loss/uncertainty witness, plus client artifacts. | Completion attribution UNKNOWN; perfect ledger/image does not establish triangle. | DO NOT RETRY. STOP; review recovery need, no automatic action. |
| Process crash/abnormal termination | Retain external process/lifetime/outcome witness and every already-written byte/partial stream. | If ioctl may have occurred, EXECUTION UNKNOWN unless direct narrower outcome evidence exists. | DO NOT RETRY. STOP; reviewed machine-boundary recovery only if needed. |
| Completion absent/unobserved | Retain kernel/client records and timeout/end-of-capture witness without a fabricated event. | No observed completion; no inference that submission failed or never occurred. | DO NOT RETRY. STOP; recovery review as applicable. |
| Observed failure/HOLD | Preserve complete available response and original kernel HOLD/failure record plus association proof and streams. | HOLD only if directly observed; failure localized only as far as attributed evidence supports. | DO NOT RETRY. Only existing separately authorized recovery boundary; no reset/unbind/replacement. |
| Boot/ownership continuity uncertain or changed | Retain both identities, discontinuity evidence and all partial artifacts. | Attribution to reviewed context UNKNOWN; STOP even if image appears correct. | DO NOT RETRY. Requires review; no boot/reset/rebinding action granted. |
| Archive write/sync/close/reread/seal verification failure | Retain originals/partials and available incomplete receipts; never reuse failed destination. | Integrity/completeness UNKNOWN for failed roles; seal cannot certify a failed transaction. | DO NOT RETRY ioctl; no automatic transaction restart. Storage/recovery review separately scoped. |

## Triangle evidence policy

- **TRIANGLE ESTABLISHED:** Trusted current-context approved process/ioctl and artifact association; full response with necessary tuple operation_errno0/outcome2/phase9/events7/color_observed1; attributable accepted TA/end-render/3D-free; original4096 preserved and response[172:] identical; checked validation copy at all1024 pixels consistent with frozen white geometry/zero background/layout; independent archive verification; no unresolved material provenance/loss/edge contradiction. Exit/ledger/hash/nonzero count/reference image alone; reporting zeros alone.
- **READBACK PRESENT BUT TRIANGLE NOT ESTABLISHED:** Original candidate image bytes actually retained, with image presence directly established; missing completion/provenance, geometric contradiction or unresolved edge interpretation recorded. If producer provenance absent, say image bytes present and hardware-readback provenance UNKNOWN; do not imply GPU readback.
- **EXECUTION OBSERVED BUT RASTER RESULT UNKNOWN:** A directly attributable execution/submission/completion observation that establishes exactly the named stage; raster/image stage not adequately observed. Specify which stage is observed. TA accepted completion permits TA-fire INFERRED if not directly observed; does not establish raster completion.
- **EXECUTION UNKNOWN:** Possible ioctl with failed/ambiguous/lost/incomplete attribution, or no direct hardware execution proof. Can retain a complete response/image while actual execution remains UNKNOWN; neither implies successful GPU behavior.

These are future evidence-policy labels, not results from this session. Missing image
provenance must be described as bytes present, not successful hardware readback. All
1024 positions/values, geometry, zero background and layout are checked on a copy.
Sampling/edge convention remains unqualified; edge-only discrepancies require independent
analysis and cannot be hidden by a count/checksum. Exit/ledger/hash/nonzero count or a
synthetic reference alone never establishes execution or triangle.

## Review result and preserved state

Destination, guard, preservation and failure contracts are specified. The correlation
architecture is partial at the exact documented-event provenance observable above.
Complete implementation review readiness:NO. Partial specification review is possible;
implementation/live use/SGX still need separate applicable review/authorization.

Gate B and approved client remain unchanged. PRE07 live readiness remains BLOCKED.
`sgx_execution_authorized=false`; candidate SGX0; task hardware0; triangle NOT ATTEMPTED.
No new HOLD or recovery state exists. No source/binary/UAPI/module/image/raw evidence was
changed, no qualification repeated and nothing staged/committed. The prior pending
request remains historical; the new decision supplies design authority only.
