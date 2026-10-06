# Authoritative Phase 8 handoff — 2026-10-05

## Current continuation — known-good triangle, 2026-10-06

**KNOWN_GOOD_TRIANGLE_READBACK / Triangle ESTABLISHED in FIRE #3.**
Continue from the [2026-10-06 authoritative handoff](CODEX-HANDOFF-20261006.md),
[permanent reproduction tutorial](FIRE3-TRIANGLE-REPRODUCTION.md) and
[full-hash reproduction manifest](FIRE3-TRIANGLE-REPRODUCTION.json).
The successful rendering bytes/evidence are frozen. The
[display-only publication path](display-publication-20261006.md) is prepared and
offline qualified; live display mutation needs separate explicit authorization.
No further SGX execution is authorized. The earlier sections below remain
historical checkpoints and do not override this milestone or confer authority.

## Latest continuation — one explicit fragment experiment, 2026-10-06

[Stage A/B report](fire02-single-fragment-experiment-20261006.md) and
[offline qualification](fire02-single-fragment-experiment-qualification-20261006.json)
classify **EXPERIMENTAL_HYPOTHESIS_PREPARED**. The FIRE #2 cause is **not
established**. The one selected hypothesis is missing effective nonzero
foreground fragment color; no TA/PBE/geometry change accompanies it.

New bounded historical evidence: DRI clear constructor39ac4→30ffd→30d15(dst1,
packedARGB)→30fb7 emits a single immediate instruction and the same zero-input
PDS launch shape. Independently, rev121-applicable Xpsb table-index0→d110
copies`00000000 fca40001`, precisely the zero-immediate variant. The experimental
build replaces only eight bytes at USE+0 with`001f00ff fca7f1f1` (opaque magenta
`ffff00ff`). Explicit compile option`SGX535_EXPERIMENTAL_CONSTANT_FRAGMENT=1`;
normal builds retain the original suffix-only bytes. This is supported CPU
construction, **not** hardware proof or a confirmed defect.

Native/UBSan/static i486+QEMU control and experiment checks PASS;1032 immediate
roundtrips per execution; complete six-backing construction/failure no-write;
five synthetic address layouts in native/UBSan show exactly eight changed
fragment bytes. Scoped contract and7 capsule-service cases PASS in both modes.
ELF32/import/CRC/vermagic PASS; forced driver rebuild and two image builds
byte-identical. Observer/client/UAPI and opaque entry source/object unchanged.

New **offline experimental** candidate: driver Build ID
`85ec06b428c99fac7f9127919b7a488d204f4a77`, SHA256
`c925caedebcc0699aef53227d29e64b47bf2833efd9d2e611cba276dd3a08ab3`;
observer`f11d3abb072caa4e1d32836ef92ce201e9c9d126` unchanged; image SHA256
`3d9eb6baa3d821b4ff98e60ea83f1a94022b5fb2881038d2ad91b95cdfb7448e`.
[Manifest](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/experimental-candidate.json).
This candidate has **not** been staged, booted or live qualified. Prior live
PRE07 does not bind these changed bytes. Next: separately authorized STOCK
preflight/staging/manual FIRSTLOAD, passive fresh PRE07; then separately
explicit FIRE #3 authorization. No execution card or FIRE authority is created.

Predictions are recorded before execution: attributable expected magenta
supports one-change sufficiency; allzero falsifies that sufficiency prediction
under the historical encoding assumption but does not establish fragment
invocation or eliminate other causes; HOLD is separate; ambiguity means STOP,
no retry. Original white-reference evidence is never silently relabeled.

All68 sealed FIRE #2 files and original driver/image bytes remain immutable;
FIRE #2 post-TA coverage still UNKNOWN, color4096bytes allzero. TA-output
format/publication searches remain exhausted. Original driver`9d0b5b2f...`,
observer`f11d3abb...`, image`ef7e01cb...` are preserved, not superseded in any
historical record. No hardware contact, client or SGX invocation in this task.
FIRE #3 **UNAUTHORIZED**; `sgx_execution_authorized=false`; task SGX0,
hardware0; triangle **NOT ESTABLISHED**.


## Final offline convergence — TA-output witness, 2026-10-06

[Final disposition](fire02-ta-output-witness-final-convergence-20261006.md) and
[structured ledger](fire02-ta-output-witness-final-convergence-20261006.json)
reach **TA_OUTPUT_WITNESS_OFFLINE_EXHAUSTED** for the identified available
permitted evidence graph. Fourteen routes are closed to repetition. This is
not a global/private-source absence or architectural-impossibility claim.

Distinct routes completed: exact TA_FINISHED-to-raster ordering, explicit
PDS/MADD consumer cache preparations, three trace-dictionary variants and
RENDERHALT/stringifier provenance, twenty independent Samsung/OMAP GPL source
files, all-ref TI event/history metadata, applicable binary neighborhoods,
PSB4.41.1 comparison and Xpsb0.18-5 package comparison. Six relevant PSB files
and both Xpsb/DRI ELF members are byte-identical to the analyzed versions.
BIF invalidation waits outstanding READS, not a proven TA payload write drain.
TA-to-raster intended GPU handoff is strongly INFERRED; external backing and
CPU publication remain UNKNOWN. Completed FIRE #2 raster lifecycle does not
prove a nonempty root or selected-triangle coverage.

Format remains successor UNRESOLVED / decoder PARTIAL / common ABI B.
Publication remains C — UNRESOLVED. No safe OPEN/CLOSE snapshot window.
The exact missing cuts are the normal rev121 first-position state/width/
successor transform and checkpoint-specific root external commit/stability
through CPUcopy before destructive DPM reuse. Obtain genuinely new permitted
consumer/same-mode encoder and event/memory contracts; repeated static passes,
uninterpretable live capture and speculative decoder/capture are not justified.

No implementation, rendering change, decoder, build or new candidate.
All68 sealed FIRE #2 files, implementation inputs and candidate bytes remain
unchanged. Driver`9d0b5b2fdb7d9881f2828f43fb89253176c38817`, observer
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`, image
`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`.
FIRE #3 **UNAUTHORIZED**; `sgx_execution_authorized=false`; task SGX/client
invocations0; hardware interactions0; triangle **NOT ESTABLISHED**.

## Latest continuation — scene/root publication, 2026-10-06

[Publication report](fire02-scene-root-publication-20261006.md) and [structured finding](fire02-scene-root-publication-20261006.json) reach
**C — GPU_CPU_PUBLICATION_UNRESOLVED**. Pinned host pages, cached SGX mappings,
CPU maintenance and driver completion/reuse ordering are established. No rev121
postcondition proves the TA-generated root is externally committed and stable
before DPM reclamation through a bounded CPU copy. No earliest/latest safe point
or stable window is established; this does not prove lifecycle destruction.

Accepted TA→before raster is the earliest candidate internal checkpoint. The
current after-status observer is later, after raster kick. End-render can precede
busy DPM cleanup; memory-free/retirement do not prove original root preservation.
CLFLUSH maintains CPU cache lines; exact x86 dma_rmb is a compiler barrier.
VISTEST feedback is CPU accumulation, not direct scene-memory publication.
The same-operation FIRE #2 color provenance/completion result remains intact.

The exact missing guarantee is checkpoint-specific external materialization and
cessation of relevant writes to the owned`S+0x1000..S+0x1300` backing through the
copy before destructive DPM action/reuse. Format remains UNRESOLVED/PARTIAL;
**both boundaries independently blocked**. Obtain new permitted rev121 event/
publication evidence. No hook, implementation, decoder, build or live capture.
48/48 CPU source consistency checks PASS; not GPU qualification. All68 sealed
FIRE #2 files and pre-existing implementation/candidate bytes remain unchanged.
Driver`9d0b5b2fdb7d9881f2828f43fb89253176c38817`, observer
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`, image
`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`.
FIRE #3 **UNAUTHORIZED**; `sgx_execution_authorized=false`; task SGX/client
invocations0; hardware interactions0; triangle **NOT ESTABLISHED**.

## Latest continuation — independent normal-root evidence, 2026-10-06

[Independent-source report](fire02-independent-root-evidence-20261006.md) and [structured disposition](fire02-independent-root-evidence-20261006.json)
reach **C — PRIMARY_EVIDENCE_NOT_FOUND** for the consumer/encoder rule, despite
finding primary headers and a rev121-capable TI build target. Successor remains
UNRESOLVED; decoder PARTIAL; common ABI B. Independent TI/EMGD/Samsung SGX535
headers do not define the relevant0x21c/0x408 range. The DDK debug dictionary
supplies EMPTY_TILE/STREAM_LINK/NEXT_RGN_BASE vocabulary, not root fields or
implementing predicates; its0xAD codes must not be confused with MMIO offsets.
ClearClip region-header handles are behind non-rev121 feature gates; Napa/Rogue
layouts do not transfer. No independent explanation of the three-word allocation
or same-mode normal-root encoder was recovered.

The identified permitted static set is exhausted. The exact missing fact remains
the selected normal rev121 first-position state predicate/consumed width and
successor-address derivation at`S+0x1000`. New permitted consumer/same-mode encoder
evidence is required; parser implementations behind the debug labels are a
specific candidate only with core/mode/root-payload provenance. Do not repeat
old Xpsb searches. Publication remains a separate unresolved boundary. No decoder,
capture, implementation, build or new candidate; live capture is not justified.

30/30 source consistency checks PASS, not GPU qualification. Preservation receipt
verifies all68 sealed FIRE #2 files, existing implementation and candidate bytes.
Current driver`9d0b5b2fdb7d9881f2828f43fb89253176c38817`, observer
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`, image
`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`
unchanged. FIRE #2 coverage cannot be reconstructed retrospectively. FIRE #3
**UNAUTHORIZED**; `sgx_execution_authorized=false`; task SGX/client invocations0;
hardware interactions0; triangle **NOT ESTABLISHED**.

## Latest continuation — bounded normal-root successor, 2026-10-06

[Successor-rule finding](fire02-root-successor-rule-20261006.md) and
[structured result](fire02-root-successor-rule-20261006.json) classify
**C — SUCCESSOR_RULE_UNRESOLVED**. Decoder envelope remains PARTIAL; common
ABI remains B.64 is the product of scaled allocation axes8×8;12 is the
allocation factor and participates in whole-slice selection`12*cookie[1]`.
No individual twelve-byte hardware header stride, EMPTY predicate, first
reference extraction or continuation/termination rule is established.
Matches in CPU prefixes/host BO lists are not normal-root payload accesses;
the communication-BO loop is not used as TA-output evidence.

New pinned historical TTM source shows fresh kernel backing allocated with
`__GFP_ZERO`, supplementing the previously known explicit prefix clear and
current owner all-page clear. Zero initial backing does not define ROOT EMPTY:
historical reuse clears only the prefix, and intervening hardware setup has
no exposed root-word postcondition. Per-word TA mutations remain UNKNOWN.
Additional public EMGD parameter/root-resource paths retain handles, not a
payload encoder/parser.38/38 CPU/source consistency checks PASS, not GPU
qualification. The precise missing fact is the selected normal rev121 ISP
first-position state predicate, consumed width and successor-address transform
at`S+0x1000`. Obtain that permitted consumer/same-mode encoder evidence before
any further primitive investigation. Publication remains separately unresolved.

No implementation, decoder, capture, build or new candidate. All68 sealed
FIRE #2 files and implementation/artifact bytes remain unchanged. Driver
`9d0b5b2fdb7d9881f2828f43fb89253176c38817`, observer
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`, image
`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`
remain current. Live capture is not justified. FIRE #3 **UNAUTHORIZED**;
`sgx_execution_authorized=false`; task SGX invocations0; hardware interactions0;
triangle **NOT ESTABLISHED**. FIRE #2 coverage cannot be resolved retrospectively.

## Latest continuation — normal TA-root grammar, 2026-10-06

[Normal-root investigation](fire02-normal-ta-root-grammar-20261006.md) and
[structured finding](fire02-normal-ta-root-grammar-20261006.json) reach
**DECODER_FORMAT_PARTIAL**; common ABI remains **B — STRONGLY SUPPORTED BUT
INCOMPLETE**. The rev121 scene-info calculation establishes a4096-byte prefix,
then a768-byte reservation at`S+0x1000`, allocated as64 twelve-byte units,
then state areas at`S+0x1300/1350/13e0`, total requested size`0x1420`.
Both TA`0x21c` and normal raster`0x408` receive`S+0x1000`. Historical explicit
clearing covers only the first page, not root headers; current all-page
zeroing is separate clean-room policy and does not define an empty region.
Cookie metadata is not the scene BO payload. The apparent eight-byte host
loop operates on a separate communication BO, not TA region references.

The precise first missing format fact is the normal rev121 **root-unit
successor rule**: how the unit beginning at`S+0x1000` distinguishes empty,
populated or continuation state and yields its next raster-reference address.
The allocation unit does not define its three words. Physical tile dimensions,
region indexing and downstream TA/CPU primitive equivalence remain unproved;
the successor rule is the minimal proof cut, not a promise that it closes all
downstream semantics. No decoder or live capture is justified. Publication
remains separately unresolved and was not investigated.33/33 CPU reference/
layout checks PASS; these are not GPU or decoder qualification.

All68 sealed FIRE #2 files and implementation/artifact bytes remain unchanged;
FIRE #2 has no TA-output capture and cannot resolve coverage retrospectively.
No new candidate: driver`9d0b5b2fdb7d9881f2828f43fb89253176c38817`, observer
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`, image
`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`.
FIRE #3 **UNAUTHORIZED**; `sgx_execution_authorized=false`; task SGX invocations0,
hardware interactions0; triangle **NOT ESTABLISHED**. Next: permitted rev121
normal-root successor semantics, then downstream comparison; no live retry.

## Latest continuation — rev121 CPU encoder/common raster ABI, 2026-10-06

[CPU producer/consumer trace](fire02-xpsb-common-raster-abi-20261006.md) and
[structured finding](fire02-xpsb-common-raster-abi-20261006.json) classify
**B — COMMON_RASTER_ABI_STRONGLY_SUPPORTED_BUT_INCOMPLETE**. The applicable
Xpsb path is **SGX535 REV121 HISTORICAL EVIDENCE**: the pinned binary's revision
selector maps raw `0x00010201` to121 and selects the normal initialization
branch. This is static applicability, not a revision-exclusive build or a new
hardware observation. The applicable CPU encoder and normal TA path converge
on the same raster scheduler/root-register family/kick; independent DRI
software-background records corroborate the CPU encoding subset. Revision
identity strengthens the comparison but does not prove hardware TA emission.

The CPU encoder's block/attribute/index/coordinate formulas, shifted pointer
relocations,64-byte group/root alignment,28-byte host metadata,8-byte flat
references and single-word terminator are now explicitly mapped. Its flat
list and the normal scene root at `S+0x1000` reside in different owned objects;
raster-control values also differ. The **one remaining equivalence fact** is
the normal rev121 root→region/reference→primitive grammar agreeing with the
CPU grammar for the selected triangle class. Neither equality nor difference
of those hardware-produced records is established. The independent
GPU→CPU backing-payload publication/stability guarantee remains unproved;
CPU mapping, host→GPU fences and later retirement cannot replace it.

No TA decoder, capture instrumentation, rendering change or new candidate was
created; native/UBSan/i386 TA-decoder qualification is not justified. Checks
are CPU reference/document/preservation consistency only. FIRE #2 retained
no TA output, so post-TA coverage cannot be resolved retrospectively. All68
sealed FIRE #2 files and implementation bytes are unchanged. Driver
`9d0b5b2fdb7d9881f2828f43fb89253176c38817`, observer
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`, image SHA-256
`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`
remain current. FIRE #3 **UNAUTHORIZED**; `sgx_execution_authorized=false`;
task SGX invocations=0; task hardware interactions=0; triangle **NOT ESTABLISHED**.
Next step is permitted normal-root/typed-primitive semantics, then independent
CPU publication; no speculative live capture or retry. Earlier findings below
remain historical and retain their original scope.

## Latest continuation — TA-output/publication and Vita comparison, 2026-10-06

[Bounded TA-output review](fire02-ta-output-publication-boundary-20261006.md)
and [structured finding](fire02-ta-output-publication-boundary-20261006.json)
reach endpoint B. The actual owned scene/DPM/parameter objects and TA→ISP address
handoff are mapped. Independent public Xpsb CPU quad-compositing code establishes
parameter-block, coordinate, reference-list and terminator encodings, but does
not establish that rev121 TA emits that layout for the selected triangle.
The first missing format arrow is `S+0x1000` raster root→typed reachable selected
triangle, including valid region/list/parameter references. The independent
publication gate is proof that those bytes are materialized in backing pages
and stable through a passive CPU copy before recycling. Accepted TA, mapping,
CPU CLFLUSH and later retirement do not automatically supply that contract.

VitaSDK/libvita2d/Vita3K/vitaGL/NanoVG-GXM/SDL are now explicit comparative
sources. All Vita GPU facts remain **FAMILY-LEVEL INFERENCE — SGX543** unless
independently corroborated; none supplies a transferable target TA-output
layout or publication guarantee. Vita tile constants, ARM uncached memory and
emulator buffers/stubs are not assigned SGX535 semantics. A low-level Vita
header branch marked Strictly Confidential was excluded, not used to derive a
decoder or recovered through another route. Source pins/search scope and the
exclusion record are retained in the linked audit.

FIRE #2 captured no scene/parameter output; post-TA coverage cannot be decoded
retrospectively. No speculative decoder, opaque dump, capture instrumentation,
shader/register change or new candidate was produced. Native/UBSan/i386 decoder
qualification was not run because the decoder gate is unmet; audit checks are
40/40 PASS CPU reference/document/preservation consistency, not GPU qualification.
All68 sealed FIRE #2 files and implementation bytes remain unchanged.
Driver `9d0b5b2fdb7d9881f2828f43fb89253176c38817`, observer
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`, image SHA-256
`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`
remain the last candidate identities. Completion/retirement/common-operation
color provenance remain CONFIRMED;4096 color bytes all zero; coverage UNKNOWN;
fragment export/PBE hypotheses unresolved; triangle NOT ESTABLISHED.

Next: obtain permitted target-specific format/reachability and publication
support at the precise gates above. No live dump/retry is justified yet.
FIRE #3 remains separately unauthorized; sgx_execution_authorized=false.
This task: SGX invocations0, hardware interactions0. Both historical FIRE
authorizations remain consumed; no Mini 12 contact or change to live authority.

## Latest continuation — VISTEST lifecycle review, 2026-10-06

[VISTEST lifecycle review](fire02-vistest-lifecycle-20261006.md) and
[structured finding](fire02-vistest-lifecycle-20261006.json) close this local
reference investigation: VISTEST is **not qualified for passive coverage of the
unchanged frozen scene**. The actual historical DRI query callbacks allocate
eight indexed result slots, emit enable/index state, wait for feedback, and
accumulate selected results. Generic Mesa default helpers did not exhaust this
path. Xpsb operation7 reads the eight words at498–4b4 after raster feedback.
The frozen CPU state selects **no query**. Exact rev121 counting predicate,
counter reset/lifetime, read side effects and complete readout isolation remain
unestablished; zero is not proof of absent geometry. The historical ISP reset
and CPU-zeroed feedback BO must not be conflated with proved counter freshness.

The minimum alternative needs a **typed selected-triangle TA output format plus
pre-raster GPU-to-CPU publication contract**. Scene allocation/cookies and the
GPU-to-GPU handoff do not supply either missing semantic fact. No opaque dump,
speculative reader, shader/geometry/PBE change or new candidate was implemented.
21/21 CPU reference consistency checks PASS, **not GPU qualification**; searches
covered30 distinct retained DDK ref tips/149 header instances/8 distinct contents
without definitions for these offsets. All68 FIRE #2 files are unchanged;
completion, retirement and provenance remain CONFIRMED, pixels all zero,
post-TA coverage UNKNOWN, triangle NOT ESTABLISHED. Module/image/client/UAPI
bytes unchanged. No Mini 12 contact or FIRE #3 in this task; SGX invocations0,
hardware interactions0, sgx_execution_authorized=false. Continue only at the
precise semantic boundary in the linked report; do not retry to gather
uninterpretable data.

## Previous continuation — post-TA coverage evidence boundary

[Bounded coverage review](fire02-post-ta-coverage-boundary-20261005.md) and
[structured finding](fire02-post-ta-coverage-boundary-20261005.json) reach
endpoint B. No existing retained artifact answers whether the selected triangle
survived TA. New reference evidence identifies Xpsb VISTEST operation7 reading
eight words at SGX offsets498–4b4, with historical kernel feedback after raster.
Its current-draw selection, counting, baseline and read semantics are unproved;
it is a candidate witness, not qualified coverage evidence. The alternative
scene-memory path lacks a target-specific decoder and CPU-publication guarantee.
The exact missing capability is a qualified triangle-specific geometry/coverage
witness, distinguishable from bounds/termination/background and completions.
No speculative register reader, shader fix or raw arena dump was implemented.
All68 FIRE #2 files and all implementation/artifact bytes remain unchanged.
No new candidate; no staging; no FIRE #3 authority. This task: SGX invocations0,
hardware interactions0; triangle NOT ESTABLISHED. Full source/path findings and
the minimum future observation contract are in the linked review.

## Previous continuation — offline FIRE #2 pixel-path analysis

[Zero-readback investigation](fire02-zero-readback-investigation-20261005.md) and
[structured finding](fire02-zero-readback-investigation-20261005.json) preserve
the confirmed completion/provenance result below. No concrete causal defect is
established: produced TA coverage, fragment color export and actual PBE
store/visibility are not distinguished by the preserved zero image. Exact
compiled scene/relocation data match the qualified source; focused native and
UBSan checks pass across five address matrices. No implementation/artifact
bytes changed; no new candidate or live action occurred. The minimum next
distinguishing fact is qualified post-TA/pre-raster triangle coverage, followed
by fragment/store evidence if coverage exists. Raw changed scene bytes are
insufficient. Do not guess a shader/register patch or repeat the unchanged FIRE.
The two historical authorizations remain consumed; task SGX invocations0,
hardware interactions0, `sgx_execution_authorized=false`, triangle NOT ESTABLISHED.


This is the current-state continuation handoff requested by the maintainer. It supersedes the current-state guidance in [the 2026-10-02 handoff](CODEX-HANDOFF-20261002.md), without rewriting historical evidence or granting implementation/live/execution authority. This document was prepared offline; no new boot-continuity observation was collected.

## Latest current state — second authorized FIRE completed; zero readback

The maintainer explicitly authorized ONE invocation on corrected boot
`a76f3b25-31ae-4507-891b-474e912dd356`, under the prepared 9d0b5b2f/ef7e01cb card.
Minimum immediate bindings42/42 passed. Exactly one ordinary approved-client
invocation occurred (PID22126); ioctl return0, client exit0, operation_errno0,
outcome2, phase9 RETIRED, accepted ledger7. Capsule CLOSED4, invalid0, terminal1,
result0, events7, color_present1. Source CLOSED3/reasons0 with retained valid startup
lifecycle and no competing/reset/power/lost-evidence invalidity. Same boot and
exact loaded notes retained after call. No timeout, receipt/preservation failure,
retrieval error or second dispatch. Authorization CONSUMED.

CONFIRMED: accepted current-operation TA completion, end-render and 3D-memory-free;
retirement; full4268 response and4096 readback agree with valid same-operation
capsule. TA submission/acceptance is INFERRED from attributable accepted completion;
no separate acceptance marker/timestamp is retained. The full color/readback is
**all zero**, nonzero_pixels0, and matches neither complete triangle reference.
**TRIANGLE NOT ESTABLISHED.** Completion is not a raster-image success claim.

All18 original files preserved/readback/hash verified and sealed BEFORE
interpretation; supplemental capsule/source/response binding and report sealed.
[Result](/home/gama/sgx535-offline/phase8-dhost-hold-fix-20261005T224726Z/live-preparation/fire-one-authorized-call/outcome-report.json);
[sealed originals and binding](/home/gama/sgx535-offline/phase8-dhost-hold-fix-20261005T224726Z/live-preparation/fire-one-authorized-call/sealed-evidence/).
Earlier failed FIRE/HOLD and preparation records remain unchanged.

STOP: `sgx_execution_authorized=false`; this authorization calls=1; historical
approved-client calls=2; NO RETRY. The consumed capsule/card/boot cannot support
another invocation. No automatic reboot/reload/rearm/reset or additional SGX.
The previous PRE07 live PASS below is historical pre-invocation readiness,
not authority/readiness for another call. Any next investigation needs a new
maintainer task; any future invocation needs separate explicit authorization.

## Current continuation — first FIRE HOLD fixed; fresh candidate preparation

The only authorized FIRE was consumed on boot `7122635d-5754-4be7-ad68-f89a30cba17f`:
client calls=1, ioctl return0, client exit1, operation result-1, outcome3, phase11
HOLD, ledger0, no retirement/readback/triangle established. Capsule issued=1 alone
means possible issuance. Originals and historical decisions are preserved.
[Root cause and minimum correction](dhost-load-hold-fix-20261005.md);
[scoped offline qualification](dhost-load-hold-fix-qualification-20261005.json).
The new load plan waits for all four DPM loads before acknowledging them (0xf;
old0x7 omitted DHOST0x8), verifies consumed status before TA and rejects residual
STATUS2 at baseline. Scene completion acceptance is unchanged. Service validation
failure now labels stage21. Offline checker corrected producer CLOSED=4, not6;
this does not change the failed attempt's outcome.

New driver Build ID `9d0b5b2fdb7d9881f2828f43fb89253176c38817` (250720 bytes),
SHA-256 `ddb4fe1fc121945d0a0ab137d310328fc10d41f5bdcfaa39baf86a3783255071`.
Unchanged observer Build ID `f11d3abb072caa4e1d32836ef92ce201e9c9d126`.
New image SHA-256 `ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`
(50816610 bytes). Repeated driver and image builds are byte-identical.
Native/UBSan focused component regressions and new8-case load/executor model pass;
6 checker and13 capsule evidence tests pass; statici386 compiles and actual module
ABI/import/CRC checks pass. New i386 runtime execution was NOT PERFORMED because
the prior temporary emulator is unavailable. Approved client/UAPI/workload,
observer/capsule/source-lifecycle, archive/readback remain unchanged/reusable.
[Exact prospective package](/home/gama/sgx535-offline/phase8-dhost-hold-fix-20261005T224726Z/one-shot-package.json).

Maintainer permitted hardware preparation and then requested fresh passive guards
on the successful corrected FIRSTLOAD. Exact new boot
`a76f3b25-31ae-4507-891b-474e912dd356` verified corrected driver and unchanged observer.
Fresh first-owner77/77, protected preparation56/56, final independent209/209 PASS.
Source startup witness valid: boot3/reset assertion127, zero pending/busy/BIF/autonomous
startup fields, source prepared UNUSED, reasons0, no retained competing/reset/power/
evidence-loss invalidity. Source/capsule remain unchanged through final guards;
capsule UNUSED; current client not executed. Health PASS with bounded stock diagnostics.
Root-readable source/capsule/context/kernel preparation originals preserved and
read back; new protected exclusive destinations ready. Normal STOCK remains saved/default,
historical image/HOLD/FIRE evidence preserved. No hot replacement/rearm/retry.

**PRE07 LIVE PASS. READY FOR EXECUTION AUTHORIZATION: YES**, bound only to this boot,
exact corrected image/driver, unchanged observer/client/UAPI and prepared destinations.
[Fresh evaluation](/home/gama/sgx535-offline/phase8-dhost-hold-fix-20261005T224726Z/live-preparation/pre07-live-evaluation.json);
[exact future one-shot card](/home/gama/sgx535-offline/phase8-dhost-hold-fix-20261005T224726Z/live-preparation/one-shot-execution-card.json).
STOP: explicit NEW FIRE remains required; no client/SGX invocation in preparation.
Before any separately authorized invocation recheck exact boot/loaded notes, valid
continuous source witness, UNUSED capsule, health and protected file/directory receipts.
Any unknown/invalidation means STOP, no reset/rearm/reload or automatic retry.
`sgx_execution_authorized=false`; historical authorized client/SGX attempt count=1;
this corrected boot/new candidate calls=0; triangle on this boot NOT ATTEMPTED,
historical triangle attempt NOT ESTABLISHED.
Earlier sections below are historical checkpoints, including their old zero counts.

## Current offline-qualified source-lifecycle candidate

The latest autonomous offline authorization produced a concrete delayed-prior-work
provider: retained **existing first initialization reset history**, no earlier eligible
operation/autonomous wake, and continuous covered producer/power/reset/IRQ-source history.
[Implementation and causal proof](source-lifecycle-quiescence-20261005.md);
[exact qualification](source-lifecycle-quiescence-qualification-20261005.json);
[new prospective package](/home/gama/sgx535-offline/phase8-source-lifecycle-20261005T064455Z/one-shot-package.json).
It adds no reset/submission/ACK/clear, and retains failed observations rather than
repairing them to PASS. Clean status alone remains insufficient.

Native **183/183**, UBSan **183/183**, adapter **8/8**, decoder **5/5 PASS**; synthetic
checks are not hardware proof. New driver Build ID
`f243e1b417e8a44fae3b5be1798b40b6e4e4b090`, observer
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`, image SHA-256
`c621ea61622bb5c15e83e0d1ad657f2ba96ce23dc277ed6c7ac007d615f76f5d`
(50,816,585 bytes; two byte-identical builds with finished-image hook/module checks).
Client/UAPI/capsule/accepted-event/readback contracts remain unchanged.

The root-read SGXSOURCE2 witness arms a passive pre-authorization source interval;
admission resamples before publishing the backend, and competing/reset/power/lost
source activity invalidates evidence through public-call closure. No raw STATUS
journal/UUID/exported sequence is added. Both PRE07 source requirements are now
**technically satisfiable** under the documented lifecycle and exact-producer/live
isolation premises. **Actual PRE07 remains BLOCKED** pending fresh live qualification
of this new candidate. READY FOR EXECUTION AUTHORIZATION: NO. Next step is separately
authorized reviewed STOCK recovery/healthy baseline, exact staging and one FIRSTLOAD
boot with passive source/identity/fresh guards, not another design inquiry or FIRE.

No Mini 12 contact, reboot, load/unload, client/ioctl execution or SGX occurred.
Latest preserved running boot/image/module identities below remain unchanged and
lack this provider; do not hot-replace them. `sgx_execution_authorized=false`;
SGX invocations=0; task hardware interactions=0; triangle=NOT ATTEMPTED.
The older fail-closed guard and original checkpoint below remain historical evidence.
Original 17/96 Git counts refer to that checkpoint, not the latest offline changes;
the new qualification/preservation receipt records the latest counts separately.

## Earlier authorized offline source-guard implementation

The maintainer subsequently authorized offline design/implementation only. The
[source-guard implementation and focused qualification](source-quiescence-guard-20261005.md)
records a passive pre-admission read/check, retained reasons and narrow existing
2D/reset-path observation taps. **The native delayed-work exclusion premise remains
UNPROVEN; even clean reads refuse admission. PRE07 remains BLOCKED.** Two changed
modules build offline; no new image was built, staged or loaded. The running boot
and image/module identities below remain unchanged and lack this new guard.
No hardware contact, ioctl or SGX invocation occurred. The following original
checkpoint and its historical Git counts remain preserved; new changes are listed
in the linked qualification record and must not be attributed to earlier preparation.

## Exact successful corrected candidate

Latest preserved candidate boot: **`ad1c8ae6-0c52-4ed0-b117-bcd7473bffb1`**, Dell Inspiron 1210 / Mini 12, `5.10.240-antix.1-486-smp`, i686. Action: **`MINI12-SGX535-REV121-FROZEN-32x32-SEQ1`**.

| Artifact | Bytes | Exact identity |
| --- | ---: | --- |
| Corrected FIRSTLOAD image | 50,811,746 | SHA-256 `b19838c188e9257834ed470f1c401f6ce3938da46f8dae3b7822e46839db7142` |
| `gma500_gfx.ko` | 246,972 | Build ID `cd9b947371f18c2d19af05bfda69fbf9462c2e62`; SHA-256 `400b16b14a7fd648fe219842e3b0d5994ee8eec910e5a7894f31c373098e8b54` |
| `sgx535_provenance.ko` | 15,524 | Build ID `143b1284fc643ea9a7ddd4aeeefa05cdc682718b`; SHA-256 `6842b21b4ab0905e198ca95e07bee7ff52f0d225c8992e9ef18921055482a11d` |
| Approved response-export client | 775,264 | SHA-256 `2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835` |
| Fixed UAPI | 760 | SHA-256 `04dd2080deeb0056a366546fe27faddbdeff6c1657a2471c96e39749dc080420` |

Loaded driver note SHA-256: `0a72fda39f4521baf6815d39c2b97734f2b38dbf263ca1a44c498170ecbd5df7`. Loaded observer note SHA-256: `9f0fe23322fda884612dc5b539230e75fbc1f892275e149cbe6709d9369db9fc`. Both were verified Live in the preserved candidate preparation.

Identity basis: [Architecture B qualification](current-operation-result-provenance-qualification-20261005-cd9b9473.json), [corrected package](/home/gama/sgx535-offline/phase8-firstload-hash-fix-20261005T040456Z-cd9b9473/one-shot-package.json), and [actual preparation result](/home/gama/sgx535-offline/phase8-corrected-live-preparation-20261005T041729Z-b19838c1/pre07-result.json). Exact client approval remains [the recorded maintainer decision](response-export-client-substitution-maintainer-20261004-314e2f3b.json). Gate B remains PASS within its established scope; no historical authority is silently rebound.

## Preserved qualification and live preparation

- **39/39 PASS, zero skips:** [FIRSTLOAD construction correction qualification](firstload-hook-binding-fix-qualification-20261005-b19838c1.json). Finished-image hook/hash binding passes; two builds are byte-identical. Malformed image `5001a64f…` and its HOLD evidence remain preserved.
- **74/74 PASS:** [candidate first-owner capture](/home/gama/sgx535-offline/phase8-corrected-live-preparation-20261005T041729Z-b19838c1/candidate-first-owner/decoded.json), including loaded identities and complete successful loader trace.
- **50/50 PASS:** [protected preparation](/home/gama/sgx535-offline/phase8-corrected-live-preparation-20261005T041729Z-b19838c1/protected-preparation/decoded.json).
- **196/196 PASS:** [independent final passive snapshot](/home/gama/sgx535-offline/phase8-corrected-live-preparation-20261005T041729Z-b19838c1/independent-final-guards/decoded.json). These checks do not establish the two remaining source predicates.

Capsule: **UNUSED**, 4,152 bytes; SHA-256 `0d24c306b7e1fcd4eee1d2dc77637fe1eafa91816e849fd5c134e683bf3f3504`, unchanged across the preserved preparation captures. The approved client was staged as protected data and **never executed**. No triangle ioctl or SGX invocation was performed by this work.

Protected evidence destination: `/root/sgx535-frozen-seq1-cd9b947371f18c2d19af05bfda69fbf9462c2e62-b19838c1/evidence-ad1c8ae6-0c52-4ed0-b117-bcd7473bffb1`. Preparation originals exist; future response/image/stream/capsule outputs were verified absent. [Preparation consistency](/home/gama/sgx535-offline/phase8-corrected-live-preparation-20261005T041729Z-b19838c1/final-consistency.json) preserves the evidence boundary.

**sgx_execution_authorized=false; SGX invocations=0; triangle=NOT ATTEMPTED; completion=UNOBSERVED; readback=NONE. PRE07=BLOCKED. READY FOR EXECUTION AUTHORIZATION: NO.**

## Exactly two remaining PRE07 requirements

1. **Pre-admission pending-source quiescence / delayed-prior-event exclusion.**
2. **Continuous relevant competing-producer isolation**, connected to that established quiescence boundary and maintained through the required transaction boundary.

These are existing requirements, not new gates: corrected package `guard_requirements[2]`; [design contract](current-operation-result-provenance-design-contract-20261005-314e2f3b.json) freshness requirements, lines 88–94; [design specification](current-operation-result-provenance-design-20261005.md), lines 170–183. A **clean raw status snapshot alone is insufficient**, as explicitly stated in that specification. UNUSED proves observer admission history, not absence of hardware pending state or delayed prior work.

## Reconstructed tracing conclusion and minimum next addition

The previous conversational KPROBE analysis **was not found as a preserved report** in the inspected Phase 8 documentation/current evidence. Do not cite an invented report or treat the conversation as a preserved qualification artifact. The following conclusion is **reconstructed from current permitted source/design evidence**, not attributed to missing material:

Existing tracing can **potentially** support loss-aware continuous observation of relevant competing producer/reset paths, but **cannot itself establish the missing passive pre-admission source-quiescence boundary**. Kernel configuration supports KPROBES/KPROBE_EVENTS; native target registration, complete coverage and safe timing impact have not been qualified. Source evidence:

- [Qualified kernel configuration](/home/gama/sgx535-offline/antix-kbuild-successor-20260930/output/.config), CONFIG_KPROBES and CONFIG_KPROBE_EVENTS; [probe hit/miss accounting](/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/Documentation/trace/kprobetrace.rst), lines 146–151; [trace-buffer loss counters](/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/Documentation/trace/ftrace.rst), lines 684–714.
- [Backend](../../kernel/sgx535_frozen/gma500_fixed_backend.c), lines 42–100 and 289–300: IRQ capture only retains sampled events; the sampler requires possible fire and acknowledges events; the baseline is inside the operation. None supplies a passive pre-admission delayed-work exclusion certificate.
- [Observer](../../kernel/sgx535_frozen/gma500_capsule_observer.c), lines 114–121, and [capsule serialization](../../tools/psb-dri-re/frozen_capsule.c), lines 125–137: fresh software admission checks exist, but the exported capsule has no hardware-source quiescence witness.
- [Normal 2D path](/home/gama/sgx535-offline/phase8-provenance-implementation-20261005T005304Z/build/module/accel_2d.c), lines 38–59, 70–119 and 290–301: framebuffer work uses a separate lock and can reach a reset affecting TA/USE/ISP. Frozen-owner protection is not universal source isolation. Ordinary 2D completion is filtered by the backend; Xorg holding a DRM file is not itself proof of competing triangle submission.

The supported minimum implementation addition is a **passive pre-admission source-quiescence guard** with justified hardware/source semantics and retained evidence. It must exclude relevant pending state and delayed prior work, then connect that boundary to continuous relevant producer isolation. Trace silence, matching sequences, a zero status word or a claimed validity flag cannot substitute for this proof.

**Do not manufacture quiescence through submission, completion clearing/acknowledgement or reset.** Do not invoke the frozen operation to obtain a preflight observation. No implementation change or new build is authorized by this handoff; any future change needs its own authorization and scoped qualification. Preserve the current successful boot. Restricted material remains excluded; no reconstruction/bypass is permitted.

## Git preservation and continuation boundary

Immediately before creating this handoff: **17 modified tracked files, 96 untracked entries, empty index**. The complete porcelain status exactly matched the preserved [post-preparation Git snapshot](/home/gama/sgx535-offline/phase8-corrected-live-preparation-20261005T041729Z-b19838c1/git-after.txt). This dirty state predates this takeover; none is attributed to writing this handoff. Counts describe Git status entries, not individual files inside untracked directories. This new untracked handoff adds one entry, making 97; existing tracked/untracked contents must remain unchanged.

Do not clean, reset, stash, discard, stage or commit existing work. Do not reopen Gate B, Architecture B, provenance/property reviews, client qualification or FIRSTLOAD qualification. No target contact, reboot, module load/unload, client execution or SGX invocation is part of this documentation task. CONFIRMED means directly supported by preserved evidence; reconstructed technical conclusions are identified above; the two unmet live properties remain UNKNOWN. Stop at this boundary.
