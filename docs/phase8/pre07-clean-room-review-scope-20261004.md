# PRE07 clean-room design-review scope — 2026-10-04

This is an evaluation and pending review request, not a live procedure or permission
 to design, implement, collect guards, stage or invoke SGX. The maintainer authorized
preparing the request only. The original restriction remains intact.

[Structured request](pre07-clean-room-observation-preservation-design-review-request-20261004.json)
contains the exact context, source inventory, observable contract, independence
questions, exclusions and proposed documentation clarification. Decision, authority
and decision timestamp are null. No previous decision is repurposed as this approval.

## Evaluation

**PARTIAL — conceptually reviewable observation/preservation scope.** Requirements
can be specified without historical wrapper internals. It remains UNKNOWN whether
permitted kernel observations can prove the necessary live invocation attribution.
That is an unresolved feasibility/proof question, not evidence that the old wrapper
must be recovered or that a new kernel mechanism is necessary.

The allowed client itself documents a single ioctl at
[frozen_triangle_one_shot_response.c:77](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c#L77).
Its preserved qualification establishes one mocked call and identical request/device
semantics; the exact maintainer substitution approval is already recorded. It is
therefore the independently documented invocation primitive, subject to separate
execution authorization. No additional restricted invocation primitive is demonstrated
necessary to issue it. This observation neither executes the client nor certifies the
complete live evidence path.

The requested review concerns only observation and preservation requirements for
separately authorized activity. Client launch/executor design is excluded. A reviewer
must still evaluate whether a concrete future proposal would indirectly substitute
for the original blocked operation. Different code or naming is insufficient.

## Permitted evidence basis

- [Readiness decision](gate-b-maintainer-readiness-20261004-314e2f3b.json): PRE01–PRE08,
  original-before-copy evidence standard, attributable accepted TA/end-render/3D-free,
  STOP/no-retry and separate execution permission.
- [Exact response-client approval](response-export-client-substitution-maintainer-20261004-314e2f3b.json)
  and retained qualification: unchanged client/UAPI/artifact pins, one-call semantics,
  raw response and image roles.
- [Approved client source](../../tools/psb-dri-re/frozen_triangle_one_shot_response.c)
  and [UAPI](../../kernel/sgx535_frozen/gma500_fixed_uapi.h): documented public interface
  and response layout only. The UAPI contains no exported invocation token/sequence.
- [Archive/checker](../../tools/psb-dri-re/frozen_evidence_bundle.py),
  [file primitive](../../tools/psb-dri-re/frozen_first_load_stage.py) and unrestricted
  first-owner predicates: existing component requirements and limits only.
- Ordinary local Linux man pages: documentation may inform later review; availability
  here grants no target access and proves no Mini 12 capture capability.
- Completed restriction-provenance finding/original user instruction: scope only;
  restricted tool/source payloads and historical implementation detail excluded.

## Correlation boundary

The requirement is actual association of the one approved process/ioctl, returned
artifacts and accepted kernel completion OR failure/HOLD observations with the bound
context, excluding stale/unrelated/partial evidence. No particular token, sequence,
PID format or log transport is mandated by the permitted records. A before/after
interval, process/boot identity and exactly-one-call semantics are supporting evidence;
their sufficiency for actual accepted-event attribution remains UNKNOWN.

The proof obligation can enter independent review. An acceptance scheme cannot be
asserted sufficient before review of actual permitted observability. No historical
correlation machinery, event emitter or restricted kernel-entry material is a source.
No new kernel mechanism or target collector is proposed here.

## Proposed clarification only

Distinguish the explicit original ban on reconstruction/indirect bypass from later
inheritance describing all new orchestration as unavailable. The structured request
contains proposed clarification wording. It is not applied to the handoff, authority
records or historical evidence. Considering a new independent proposal does not lift
a real external restriction or approve a disguised substitute.

## Boundary retained

PRE07 remains BLOCKED; protected live destinations and fresh observations are not
established. Gate B and exact client approval are unchanged. Execution authorization
is false; SGX invocations and task hardware interactions are zero; triangle NOT
ATTEMPTED. The next decision is the bounded design-scope review, including the
independence/anti-substitution determination; no live step follows automatically.
