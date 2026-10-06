# FIRE #2: post-TA coverage evidence boundary

2026-10-05. **Endpoint B: a precise evidence boundary, not an identified rendering defect.**

The selected triangle's post-TA coverage remains **UNKNOWN**. This review found
a narrower historical observation candidate: eight VISTEST feedback words.
Their applicability to the current draw is unproved. Neither completion,
nonempty scene memory nor the name VISTEST qualifies a coverage observation.
No rendering state, shader, module, image or client was changed.

The [structured finding](fire02-post-ta-coverage-boundary-20261005.json) and
[offline audit](/home/gama/sgx535-offline/phase8-post-ta-coverage-20261005T234612Z/)
record the inspected sources, binary ranges, reference checks and preservation
checks. The [previous pixel-path investigation](fire02-zero-readback-investigation-20261005.md)
remains applicable; its serialization and completed qualification were not repeated.

## What is and is not recoverable from FIRE #2

The sealed [FIRE #2 evidence](/home/gama/sgx535-offline/phase8-dhost-hold-fix-20261005T224726Z/live-preparation/fire-one-authorized-call/)
establishes ledger7, retirement and same-operation response/capsule/color
provenance. Its 18 originals include response, capsule, color, source records,
context, streams and kernel logs. They contain no scene/page-table/parameter
capture and no visibility feedback. All 68 files under that evidence root
remain unchanged. The 4096-byte image remains entirely zero.

**CONFIRMED:** the qualified response and capsule serialize accepted events,
terminal state and color; they do not serialize generated primitives or fragment
coverage. See [capsule observer](../../kernel/sgx535_frozen/gma500_capsule_observer.c),
`sgx535_provenance_after` at lines356–365 and `evidence_open` at399–409.
No later offline decoding can reconstruct bytes that were never retained.
This does not contradict accepted completion or provenance.

## Exact TA-to-raster trace

1. **CONFIRMED:** the owner holds separate `SCENE_HW`, `TA_PAGE_TABLE` and
   `TA_PARAMETER` allocations. Qualified sizes are8192,33554432,33554432 bytes;
   see [contract](../../tools/psb-dri-re/frozen_kernel_contract.c), lines8–19.
   These are not the user-supplied vertex/state stream. Owner pages are private
   and mapped for hardware; the current retained evidence does not contain their
   hardware-produced contents.
2. **CONFIRMED:** `sgx535_frozen_scene_info32` at320–345 emits cookie offsets
   `0,1000,1300,1350,13e0`, logical scene size1420 and one clear page. The retained
   Xpsb `Xpsb_scene_info` at ELF3a40–3ce0 computes these from dimensions. This
   specifies CPU allocation/address arithmetic, **not** a primitive-record format.
3. **CONFIRMED:** TA setup uses the scene base/cookie in register220 and the
   scene base+1000 in21c; raster uses the same base+1000 in408. See contract
   `sgx535_frozen_ta_fire_plan` at515–579 and `sgx535_frozen_raster_fire_plan`
   at582–644. Retained `Xpsb_scene_switch_fire`4550–4ae8 corroborates these
   address calculations. No CPU interpretation of the resulting primitive links,
   transformed vertices or tile coverage was found in that path.
4. **INFERRED:** this shared address participates in the hardware scene handoff
   from TA to raster. The SGX535-specific types/fields behind it, allocation-chain
   encoding, terminal markers and triangle-to-region mapping remain **UNKNOWN**.
   Calling408 a decoded region-header register would exceed the available target
   definitions; names/encodings from SGX540/544 or Rogue were not substituted.
5. **CONFIRMED:** the service first accepts TA status, then immediately begins
   raster and performs the existing ISP bracket/schedule/fire. The precise
   potential checkpoint is in [service](../../tools/psb-dri-re/frozen_fixed_service.c)
   `fixed_service_status_impl`, after successful `session_observe_status` and
   before `session_begin_raster` (lines208–228). Current `before` tap precedes
   acceptance; current `after` tap follows raster fire. Neither is a retained
   post-acceptance/pre-raster scene capture.
6. **CONFIRMED:** the historical [PSB scene code](../poulsbo-data/PSB_psb_scene_c.txt)
   clears/maps/allocates the scene (30–74,111–147), loads TA allocation addresses
   (248–265), and manages lifetime. [PSB scheduler](../poulsbo-data/PSB_psb_schedule_c.txt)
   `psb_ta_done`421–467 marks the scene complete and schedules raster without
   inspecting generated geometry. This is lifecycle evidence, not coverage proof.

The stream also contains bounds and termination records
([serializer](../../tools/psb-dri-re/frozen_triangle_image.py),162–168).
Consequently an allocation, nonzero word, changed hash, tile event or generic
primitive count need not identify the intended white triangle.

## Existing observation branches exhausted in the available records

| Branch | Direct result | Why it does not answer coverage |
| --- | --- | --- |
| FIRE #2 originals/current capsule | Complete lifecycle/color records; no TA scene or feedback bytes | Missing observation cannot be recovered retrospectively. |
| SGX535 EMGD and PSB event definitions | TA_FINISHED, END_RENDER, MEM_FREE, END_TILE and VISIBILITY_FAIL names/bits | None specifies the selected triangle's surviving geometry. Lack of a fail bit is not positive coverage. |
| Available SGX535 header branches/history |13 retained remote-ref tips and4 path-change commits, one identical739-line header, SHAcb2a3e9119d65b421a16744137b1114b0ebd6982cb27549b4fce1d8405bd41c3 | No primitive/region format or coverage counter contract in these definitions. This covers available header tips/path-change history, not external sources. |
| Scene/parameter allocation APIs | Scene cookies, BO addresses, parameter-buffer management | Allocator metadata does not decode GPU-produced scene contents. |
| DDK HWPerf | Circular-buffer entry structure and start/end event types | Separate DDK/microkernel facility; current frozen producer has no such recorder. Group/bit selectors do not document a SGX535 triangle counter. |
| DDK debug register dumps | Fault/clock/event readback; optional other-register names | Names are not target counter semantics. MP-core definitions do not establish an SGX535 observable. |
| Retained Mesa query exports | `_mesa_begin_query`9fe20 does no work; `_mesa_end_query`9fe25 marks a CPU object ready | These default helpers are not GPU coverage readers. This does not claim every possible driver query override is absent. |
| Raw post-TA scene/page capture | Capture point and owner lifetime can be identified | No qualified record decoder or CPU-publication guarantee at that point; opaque bytes cannot distinguish triangle, bounds and housekeeping. |
| Historical VISTEST feedback | Exact read loop identified below | Promising smaller candidate; semantics/current-draw applicability still missing. |

Primary target headers:
[EMGD](../poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt),137–197;
[PSB](../poulsbo-data/PSB_psb_reg_h.txt),78–100;
[retrieved SGX535 header](../archaeology-data/H535.txt).
The DDK branch inventory and bounded disassemblies are in the offline audit.
The public DDK `sgx_mkif_km.h`440–467 describes HWPerf storage, not coverage
semantics. SGX535 feature selection is at `sgxfeaturedefs.h`63–73.
No excluded ioctl-entry/reporting source or unavailable specification was accessed.

## New concrete lead: eight VISTEST words

**CONFIRMED CPU/reference chain:**

- [Public PSB contract](../poulsbo-data/PSB_psb_drm_h.txt),280–291, declares
  `drm_psb_vistest.vt[8]`; line357 identifies operation7.
- [PSB XHW source](../poulsbo-data/PSB_psb_xhw_c.txt),131–143, requests that operation.
- Retained `Xpsb.so` SHAda531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f:
  `XpsbThread` jump table at ELF d0a0 maps case7 to328a. At3290–32a9 it reads
  `SGX mapping + 498 + 4*i`, i=0..7, into feedback words. This loop has no explicit
  MMIO write or work submission. It is not the restricted current caller.
- [PSB scheduler](../poulsbo-data/PSB_psb_schedule_c.txt),489–501, requests
  feedback after raster completion when a feedback page exists;635–676 associates
  the response with `feedback_task` and accumulates the eight words with saturation.

**INFERRED:** these are visibility-related hardware result words and could be a
smaller coverage witness than an opaque memory dump. **UNKNOWN:** what increments
each word, whether the current triangle selects/enables one, whether bounds or
background work contributes, read side effects, baseline/reset behavior, stale
retention and when values become authoritative. The current register/state
packing does not supply a qualified answer to those questions.

The historical feedback transport runs **after raster**, not at the TA boundary.
A qualified triangle-specific positive raster visibility result could establish
that geometry survived into raster; zero could establish absence only if counting
and reporting completeness were also proven. Zero without those premises remains
UNKNOWN. The historical read loop alone proves neither conclusion for FIRE #2.

## Minimum missing capability and capture contract

The missing capability is **a revision-valid, triangle-specific geometry/coverage
witness with a proved observation/publication contract**. Not another completion
token, UUID, event journal, shader replacement or larger ABI.

The smallest promising implementation route is the existing VISTEST word(s),
**only after** independently establishing their target semantics and current
triangle selection. It would require a passive immutable observation bound to
the already-qualified current operation, with a proved fresh baseline and a
complete read before lifetime loss. No new rendering state may be assumed needed.
If those counters cannot provide that property without changing the draw, the
alternative is a SGX535 scene/parameter decoder plus a CPU-visible post-TA snapshot.
No available evidence chooses a safe minimum byte range or proves the GPU's
relevant internal data has reached CPU-readable memory at that checkpoint.
CPU `clflush`/`dma_rmb` alone would not establish that device publication.

A qualified memory witness would need to traverse complete current-scene links,
validate their target-owned ranges/termination, identify this triangle separately
from bounds/termination/background records, and establish nonempty target
intersection under the actual clipping/culling/viewport state. Present means
surviving raster input, not merely three submitted vertices; absent requires a
complete trustworthy traversal. Truncation, stale/reused owner evidence,
interference, incomplete device publication or lost evidence must remain UNKNOWN.
Capture must occur after accepted TA and before the existing raster transition,
without clearing events, performing an extra context-store/reset or generating work.

**No new hardware observation implementation is claimed qualified.** A speculative
reader would not resolve the missing semantics and could not be proven passive.
A dump of both32MiB arenas would add volume without proving coverage. This is why
the review stops at endpoint B rather than constructing an unqualified live candidate.
This is not a proof that the hardware lacks such a capability.

When a legitimate semantic provider exists, focused offline qualification must
cover a selected triangle present, fully decoded empty/culled triangle, bounds-only
scene, stale/reused snapshot, wrong-owner links, incomplete traversal/counter
reporting, competing activity, evidence loss and immutable retention. Synthetic
tests would qualify decoder/preservation behavior, not manufacture hardware meaning.

## Fragment export and PBE discriminator

**CONFIRMED:** the selected compiler/link CPU path yields zero main instructions,
zero compiled attributes and the linked words00000000/f8040140. The same words
are emitted by a separate retained helper at2648d–26493. That corroborates bytes,
not a NOP/export ISA interpretation. The [compiler finding](../phase7/psb-dri-re/frozen-fragment-exact-output.md),
[link finding](../phase7/psb-dri-re/frozen-fragment-link-progress.md), and
[launch count analysis](../phase7/psb-dri-re/pds-launch-count.md) distinguish the
compiled attribute count, PDS data extent and resource quotient. Their zero values
do not prove what hardware does with implicit inputs or outputs.

Missing explicit compiled color export remains the strongest **specific concern**,
not an established defect. No shader/count/register change follows from it.
The PBE/address/device-visibility branch also remains UNKNOWN: sealed zero bytes
cannot distinguish no store from a store of zero. Same-operation color provenance
does not measure the upstream store destination/value.

After coverage is positively established, the next discriminator must establish
whether that triangle's fragment path produces the intended nonzero color at the
pixel-export/PBE boundary. A qualified SGX535 decode/execution contract for the
actual linked launch/program could prove or refute export; an attributable output
observation could also distinguish it. If correct nonzero export is established,
the next facts are actual PBE destination/value and visibility in the owned color
allocation. Coverage counts, program bytes alone and correct-looking addresses
cannot supply those facts. None of these observations exists in FIRE #2.

## Disposition

Six focused static reference checks pass (dispatch, eight offsets, public opcode,
feedback width, frozen-write-list exclusion and header inventory). They are
**not native/UBSan/i386 qualification of a coverage mechanism**; no such mechanism
was implemented and no build/test suites were rerun. Analysis also verifies links,
unchanged original evidence and preservation of pre-existing Git work.

Driver9d0b5b2fdb7d9881f2828f43fb89253176c38817, observer
f11d3abb072caa4e1d32836ef92ce201e9c9d126 and image
ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d
remain unchanged. No new candidate or staging readiness is claimed. A fresh live
discriminating capture is justified once a valid witness is defined and qualified;
blind repetition of the unchanged draw does not close this evidence gap.
FIRE #3 requires separate explicit authorization. This task: SGX invocations0,
hardware interactions0; historical invocations2; `sgx_execution_authorized=false`;
triangle **NOT ESTABLISHED**.
