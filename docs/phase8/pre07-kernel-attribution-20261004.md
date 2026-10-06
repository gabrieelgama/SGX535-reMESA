# PRE07: permitted kernel observability and invocation attribution

Scope: offline evidence/source investigation on 2026-10-04. No kernel-entry or
restricted invocation-wrapper source was reopened, reconstructed or replaced.
No target contact, staging, boot, ioctl, SGX, artifact change or qualification rerun.
This does not change Gate B, its whitelist or execution authorization.

## Exact approval and context

The [new maintainer response-client decision](response-export-client-substitution-maintainer-20261004-314e2f3b.json)
records APPROVE_EXACT_CLIENT_SUBSTITUTION at 2026-10-04T10:23:56Z.
The original pending proposal and previous decisions remain unchanged.
Source SHA-256: `919c2611e048e4a660ecae3542264adf5a5367d9b0fbe7fbceff2f45f90adedf` (5,514 bytes).
Binary SHA-256: `2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835` (775,264 bytes).
UAPI SHA-256: `04dd2080deeb0056a366546fe27faddbdeff6c1657a2471c96e39749dc080420` (760 bytes).
Approval binds boot `89fc7306-6dac-4ab9-af62-360b7152ef22`, Build ID
`314e2f3b37195dc56df7c57dd938d78377a5ea8e` and
`MINI12-SGX535-REV121-FROZEN-32x32-SEQ1`; it authorizes none of the live operations.
Existing module/image bytes and historical 70/70 first-owner evidence remain unchanged.
The archive binder accepted these exact approved identities without rebuilding or rehashing artifacts.

## Existing mechanisms: separate visibility, attribution, correlation, preservation

| Mechanism / source | Observed event or data | Established scope / availability | Attribution and response correlation | Failure retention / passive collection / archive |
| --- | --- | --- | --- | --- |
| [Backend IRQ capture](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L41), [IRQ patch](../../kernel/sgx535_frozen/patches/antix-fixed-irq.patch#L22) | Raw STATUS1/2 accumulated before existing IRQ handling/clear; normal 2D and aggregate bits excluded | Existing candidate backend source is pinned in the preserved source manifest; event mapping/capture reviewed in fixed-one-shot review line 393. Available internal observer, not an external capture interface. | Active/fire-possible backend and matching device; lock serializes IRQ/polling. No emitted token/log/tracepoint in this inspected facility. These guards alone do not establish current physical event delivery. | Internal pending words are consumed by sampling. Not an append-only diagnostic file. No available passive export from this facility itself; cannot feed internal memory directly to archive. |
| [Status source](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L58), [owner establishment](../../kernel/sgx535_frozen/gma500_fixed_backend.c#L383) | Locked sample/ack returns pending words and exclusive_owned=1 | Existing reviewed backend; one IRQ owner, active/power-held/fire-possible and deadline checks | Requires session.sequence == supplied sequence and correct fixed_irq_owner. Internal association to the active scene, not an externally authenticated process/ioctl identifier. | Read/sample/ack is an existing active service operation, not a passive diagnostic probe. No new call is proposed. Raw words are not a public userspace export. |
| [Session status acceptance](../../tools/psb-dri-re/frozen_kernel_contract.c#L953), [service loop](../../tools/psb-dri-re/frozen_fixed_service.c#L288) | TA bit13; end-render bit18; 3D-free bit0; fault/unknown/duplicate/order checks; ledger1/2/4 | Existing reviewed core; service header line82 explicitly says a raw register snapshot alone is not attribution | Same active sequence and exclusive ownership; TA must precede raster. Ledger supports accepted observation bookkeeping; it is not independent hardware proof. | Errors/HOLD retain state under the reviewed contract. The latest diagnostic sample is overwritten in the loop; it is not a retained stream of every accepted TA/raster status. |
| [Service diagnostic](../../tools/psb-dri-re/frozen_fixed_service.h#L66), [permitted diagnostic audit](diagnostic-gate-b-technical-audit.md#L102) | Last reached stage, failure stage/source, callback result, observation words; documented kernel HOLD report | Internal fields and audit description available. Restricted kernel-entry/reporting implementation not reopened. | UAPI omits the internal tuple. Successful kernel report contents/markers and invocation association are UNKNOWN from available permitted records. | Kernel HOLD report is described by the audit; emission/delivery is not guaranteed during hang/lost report. No current applicable capture route demonstrated. If captured legitimately, original log bytes can enter archive unchanged. |
| [UAPI](../../kernel/sgx535_frozen/gma500_fixed_uapi.h#L10), [approved response client](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c#L77) | Complete4268-byte post-call object; image offset172;4096 bytes; tuple/ledger/rows | Qualified/approved byte exporter, exactly one ioctl in the established paired evidence | No sequence/PID/timestamp/transaction UUID/boot/Build ID field. Exact raw/image equality is evidence consistency, not independent kernel provenance. | Raw export precedes success/image processing; negative-ioctl raw remains UNAUTHORITATIVE. Stream capture and protected parent paths still need the enclosing permitted transaction. No execution here. |
| [Preserved first-owner records](../hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/candidate-first-owner/decoded-records.json) | Hook BEGIN/FILES-VERIFIED/PASS; kernel log; boot/module/ownership/health | Actual historical capture on this boot; 70/70 remains applicable in that historical scope | Hook identifies first load, not an ioctl or completion. Kernel log has no frozen invocation/completion markers; zero invocation makes this absence expected. It does not establish that future emitters do not exist. | Already preserved bytes and timestamps are reusable historical evidence, not fresh guards or an invocation capture interval. |
| [Passive capture API](../../tools/psb-dri-re/frozen_first_load_capture_v2.py#L38) | Supplied reviewed child output, status/timeout, start/end boot and clocks | Available reviewed transport/preservation primitive. Requires a separately reviewed root program, scope/readiness and applicable authorization. Experimental/recovery modes have a first-owner clock. | Timing bounds its own supplied passive capture only. It does not observe an approved client ioctl or authenticate supplied completion text. No applicable current-context invocation/guard root program established. | Keeps partial stdout/stderr; no automatic retry. Using a newly invented root program or removing/repinning the first-owner conditions would substitute for the unavailable layer. |
| [Offline archive](../../tools/psb-dri-re/frozen_evidence_bundle.py#L130), [verifier](../../tools/psb-dri-re/frozen_evidence_bundle.py#L208) | Already supplied originals, exact response/image equality, context, bytes, seal | Complete within its offline role; established tests reused | UUID identifies an archive only; log-prefix continuity and declared producer equality are supporting consistency. Attribution/freshness remain UNKNOWN, triangle_established=false. | Protected exclusive copies and checked sync/readback preserve supplied logs unchanged; originals precede validation copy. Cannot create absent runtime observations. |

No new diagnostic operation, dynamic tracer or tracepoint was configured.
The permitted backend/service sources inspected contain no printk/pr_info/dev_info,
TRACE_EVENT or trace_printk export. That statement is limited to those sources;
it does not infer the contents of the restricted entry implementation.

## Sequence and the minimum proof

CONFIRMED: the internal session stores a caller-supplied nonzero sequence;
[enter_fire](../../tools/psb-dri-re/frozen_kernel_contract.c#L899) initializes the ledger,
and the selected action is sequence1. The historical review lines330-334 describes
one attempt per module instance. Sequence1 is not globally unique across boots or
module instances and is not returned in the UAPI. A TA-memory allocation cookie
is not an invocation nonce.

No available permitted interface demonstrates an exported invocation-unique token.
Whether the restricted kernel report carries additional identifiers is UNKNOWN.
No inspected evidence rule requires a new token: the historical fixed-one-shot
review lines447-460 instead combines fresh identity/state, independent log capture,
sequence1, one client invocation and retained ownership/no-retry bounds. Its old
hot-transition steps and artifact identities are historical, not current instructions.

Exactly-one-call code and an unused one-attempt module can narrow association to
one scene, but require actual current continuity/history evidence. Timing or
before/after dmesg alone cannot prove which statuses were accepted, exclude gaps,
or bind the captured bytes to the producing client. The historical capture requirement
does not itself supply the missing exact current-context capture procedure.

The smallest unavailable attribution capability is an applicable reviewed, bounded
capture/association of the existing accepted-status and failure/HOLD evidence with
the exact single client ioctl and returned artifacts. The capture must preserve the
needed evidence and establish its same boot/module/scene/process scope and completeness.
This is a missing evidence boundary, not a demonstrated need for a nonce or new GPU operation.
Whether existing successful emissions contain the required observations remains UNKNOWN;
a collector cannot recover observations that are not emitted. No existing available
reviewed route was demonstrated. No alternate executor was constructed.

A kernel source change is NOT established as necessary. The existing kernel already
has the internal scene/status association; successful external emission details are
unavailable. Naming or modifying a restricted emission function would reconstruct
that boundary. If a later permitted review proves an export change necessary, changed
kernel bytes would require new module Build ID/hash, offline module qualification and
image reconstruction/identity, plus applicable runtime identity/first-owner evidence.
Unchanged client/UAPI/workload qualification would remain reusable if truly unaffected.
No such change, build or requalification is justified or performed by this investigation.

## Composition after attribution

**If kernel attribution were solved, PRE07 would still not be composable from the
currently available reviewed pieces.** Two other independent requirements remain:

- An applicable current-boot/build/client fresh guard source. The [prior bounded decision](prospective-client-substitution-maintainer-20261004-314e2f3b.json#L209) explicitly records that absence. Historical first-owner/old-context passive programs cannot be repinned or repeated as a substitute. Guard predicates exist; fresh collection/provenance does not.
- An applicable protected target transaction: reviewed parent/path bindings and client/output staging checks, stream capture, preservation/transport failure handling and ordering. [PRE04/PRE07](gate-b-maintainer-readiness-20261004-314e2f3b.json#L612) and the [prior decision](prospective-client-substitution-maintainer-20261004-314e2f3b.json#L207) record this gap. File creation/sync/readback primitives exist, but they require verified target directory descriptors and applicable parameters/authority.

These may share an enclosing implementation; available evidence does not prove that
kernel attribution alone supplies them. They are not new review gates. The exact new
client approval resolves only the forward identity decision; it grants no staging,
contact or guard permission. Generic copy and offline archive algorithms remain reusable.

PRE07: BLOCKED; existing matrix counts remain5 satisfied/15partial/2unavailable.
Protected live preservation: NO. Current fresh guard collection: NO. Staging: NO.
READY FOR EXECUTION AUTHORIZATION: NO. sgx_execution_authorized=false.
SGX invocations0; hardware interactions0; triangle NOT ATTEMPTED; no new HOLD/recovery.

## Evidence and consistency checks

Metadata pin checks and archive bind_context acceptance are in:
`/home/gama/sgx535-offline/phase8-kernel-attribution-20261004T102356Z/forward-binding-check.json`.
The permitted candidate source pins and exact module identity were reused from:
`/home/gama/sgx535-offline/request-byte-decoding-20261002T222753Z/source-manifest.json`
and `qualification/identity.json`. No expensive evidence was recomputed.
Existing response-client qualification/integration records remain in:
`/home/gama/sgx535-offline/phase8-response-export-20261004T092750Z/final/`.
A preserved PRE07 matrix before this scoped update, original handoff, Git snapshots,
inspected-path inventory and final consistency checks are in the new investigation directory.
No test, build, capture or qualification was repeated. Authority/tooling records remain
untracked/uncommitted; nothing staged, committed, discarded or reset.
