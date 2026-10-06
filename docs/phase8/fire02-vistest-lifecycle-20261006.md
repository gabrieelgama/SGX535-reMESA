# FIRE #2 — Xpsb VISTEST lifecycle and coverage finding

2026-10-06. **VISTEST is NOT QUALIFIED as a passive triangle-coverage witness for
the unchanged frozen scene.** This is an evidence boundary, not a finding of a
rendering defect or a claim that VISTEST can never be qualified.

The new evidence is the historical driver's actual query implementation, not
the generic Mesa default helpers considered in the
[previous bounded review](fire02-post-ta-coverage-boundary-20261005.md).
The defaults do not establish absence of a driver override: the retained DRI
binary installs its own callbacks. The software chain connects the eight MMIO
words to eight indexed query-result slots. It does not supply a revision-valid
hardware counter/reset/read contract or enable a query in the frozen draw.

[Structured finding](fire02-vistest-lifecycle-20261006.json).
[Offline extraction and consistency evidence](/home/gama/sgx535-offline/phase8-vistest-lifecycle-20261006T000531Z/).
All addresses below are **ELF virtual addresses**, not Ghidra addresses shifted
by 0x10000. CONFIRMED describes the specified software/byte fact; it does not
silently qualify its hardware interpretation.

## Historical lifecycle reconstructed

| Stage | Direct evidence and meaning |
| --- | --- |
| Install query callbacks | **CONFIRMED:** DRI `0x371f0`, calls `0x4902b` (which initializes Mesa defaults), then `0x250f9` installs private query callbacks. `_mesa_initialize_context`, `0x56f99–56fa9`, copies that table to context+0xf0. Begin dispatch `0xa08e3` at context+0x310 therefore selects private `0x25269`; End `0xa0169`, context+0x314, selects `0x2522f`. Generic `0x9fe20/0x9fe25` are not the complete driver query path. |
| Create/begin CPU query | **CONFIRMED:** `0x25294` allocates a 24-byte query object; `0x25428` its private state. Mesa Begin clears its 64-bit result/ready flag at `0xa08ac–a08ba`. Private Begin `0x25269→0x255cd` attaches a component when a scene exists, or retains the pending query for later scene binding. Scene creation `0x26f39–26f4c→0x253cd` binds that pending query. |
| Allocate result slots | **CONFIRMED:** `0x25333→0x25801` attaches a feedback packet to scene+0x18. It CPU-zeroes and uploads 1024 bytes (`0x258a2–258e4`), allocates indices 0–7 (`0x2591e–25939`), and records packet+slot in a component list. These are software result initialization and ownership, **not** demonstrated MMIO counter reset. |
| Enable/select query in emitted state | **CONFIRMED:** `0x253a4–253b1` sets context+0x13c84=1 and context+0x13c80=slot, and dirties state. ISP-state builder `0x2c230–2c254` packs `(enabled & 1)<<3 | (slot & 7)` into the second state word. A nonzero second word enables its presence via first-word bit0x200 at `0x2c294–2c2a1`. `0x2c54b–2c5b8→0x28712` stores that state into the scene. **INFERRED:** these are hardware visibility-query enable/index fields; the CPU relationship is exact, but no matching complete SGX535 bit definition was found. |
| End CPU query | **CONFIRMED:** private End `0x2522f→0x252f0`, `0x25300–25318`, disables context query state and resets the pending-query pointer. It does not erase completed query-component results. |
| Submit/render | **CONFIRMED:** scene finalization `0x2a496–2a533` forwards the feedback BO and a presence flag to `0x37b51`, retains the returned fence and marks the packet submitted. The [PSB command contract](../poulsbo-data/PSB_psb_drm_h.txt#L250) has feedback operation/handle/offset/breakpoint/size fields. [Feedback validation](../poulsbo-data/PSB_psb_sgx_c.txt#L1159) permits VISTEST only, rejects nonzero breakpoints and requires a mapped 32-byte result range. [TA done](../poulsbo-data/PSB_psb_schedule_c.txt#L421) marks the scene complete and schedules raster; it does not count or inspect geometry. |
| Existing ISP reset | **CONFIRMED:** [raster scheduling](../poulsbo-data/PSB_psb_schedule_c.txt#L339) asserts/deasserts the existing ISP reset at lines371–385. [PSB definitions](../poulsbo-data/PSB_psb_reg_h.txt#L65) identify register0x80/bit5. **UNKNOWN:** whether this resets all eight VISTEST words, their accumulation scope, and whether every supported rendering branch preserves the same baseline. No additional reset is proposed. |
| Completion/request/read | **CONFIRMED:** [raster done](../poulsbo-data/PSB_psb_schedule_c.txt#L474) requests VISTEST when the task has a feedback page, after the raster completion callback. [XHW](../poulsbo-data/PSB_psb_xhw_c.txt#L131) queues operation7 with copy-back and UIIRQ VISTEST. `XpsbThread` case7 (`0x328a–32ab`) reads MMIO+0x498+4*i, i=0..7, in ascending order, into reply+0x50+4*i. The loop has no explicit MMIO write, reset or work submission. |
| Publish feedback/interpret | **CONFIRMED:** [reply](../poulsbo-data/PSB_psb_schedule_c.txt#L635) selects `scheduler->feedback_task`, adds each word to matching `vt[i]` with software uint32 saturation, then reports FEEDBACK fence bit4. Private result getter `0x25451` flushes an unsubmitted packet, checks/waits for its fence; `0x25680` maps and caches all eight words then returns cached[slot]. `0x2550e–2552b` sums components into a 64-bit query result. This is CPU query-result interpretation, not a post-TA geometry decoder. |
| Reuse/cleanup | **CONFIRMED:** packet slot count is bounded by8; component references retain a packet through readout. `0x2579c` decrements references and releases its fence/BO at zero. CPU query/result initialization and query disable are traceable. **UNKNOWN:** hardware read-to-clear behavior, register lifetime across scenes/contexts, exact reset/overflow semantics and cross-render freshness. |

**Historical isolation caveat:** the scheduler comment at lines492–496 says
feedback returns before the next raster fire. Do not treat that comment alone
as a universally established barrier. `psb_raster_done` calls
`psb_schedule_raster` at line487, before assigning `feedback_task` at line499;
the check at line273 is in TA scheduling and compares one particular task.
Scheduling options at lines38–81 vary. The available source does not justify
claiming universal counter isolation under every queue/configuration. A future
qualified observer needs an explicit same-operation, no-intervening-reset/render
interval. This does not allege that an actual historical query was corrupted.

## What the eight words mean

| Offset | CONFIRMED software association | Hardware interpretation |
| --- | --- | --- |
| 0x498 | reply.feedback[0] → vt[0] → component slot0 | **INFERRED:** indexed visibility/occlusion result0; exact unit/official name **UNKNOWN** |
| 0x49c | reply.feedback[1] → vt[1] → component slot1 | Same finding for index1 |
| 0x4a0 | reply.feedback[2] → vt[2] → component slot2 | Same finding for index2 |
| 0x4a4 | reply.feedback[3] → vt[3] → component slot3 | Same finding for index3 |
| 0x4a8 | reply.feedback[4] → vt[4] → component slot4 | Same finding for index4 |
| 0x4ac | reply.feedback[5] → vt[5] → component slot5 | Same finding for index5 |
| 0x4b0 | reply.feedback[6] → vt[6] → component slot6 | Same finding for index6 |
| 0x4b4 | reply.feedback[7] → vt[7] → component slot7 | Same finding for index7 |

- **CONFIRMED:** these software consumers perform numeric accumulation and
  indexed query retrieval, not timestamp decoding, a register-state dump, eight
  breakpoint records or eight independently named pipeline-stage counters.
  Breakpoints are explicitly unsupported in this historical kernel.
- **INFERRED:** the words are hardware query counts intended for occlusion
  queries. The Mesa API includes target0x8914 and the PSB callbacks feed the
  query result. Exact counting units, sample multiplicity, contributing tests,
  pre/post-depth/stencil location, shader-discard effects, and independence from
  color export/PBE remain **UNKNOWN**. The integer addition does not prove
  hardware saturation, bit width beyond a 32-bit read, or counting semantics.
- **INFERRED:** the ISP/raster visibility machinery produces these results,
  since geometry state selects a query and feedback is read after raster.
  The precise hardware writing point and final-latch/publication guarantee are
  **UNKNOWN**. TA completion is not that guarantee.
- **CONFIRMED:** [historical register filtering](../poulsbo-data/PSB_psb_sgx_c.txt#L64)
  excludes0x498–4b4 from user register/value command writes. It does not document
  whether MMIO reads clear a counter or are otherwise side-effect-free.
- **CONFIRMED:** this is a Poulsbo Xpsb path. **INFERRED:** it is relevant to the
  SGX535 family. Applicability of its exact counting/reset/read semantics to
  **rev121 is UNKNOWN**; neither the read loop nor this kernel feedback struct
  demonstrates a rev121 counter contract or validation result.

## Exhaustion of existing definitions and references

The audit searched30 distinct retained local DDK reference tips, including
both old and newer tree layouts:149 header instances,8 distinct contents for
SGX520/530/531/535/540/544/545 (no additional selected543 header available).
There is one distinct retained SGX535 header content. None of these headers
defines the eight numeric offsets. All-ref semantic searches find
`ISP_VISIBILITY_FAIL` event/enable/clear definitions and VISTEST build options,
not the missing count/reset/read contract. Numeric masks from another core are
not assigned SGX535 meanings. The extracted EMGD SGX535 header, public PSB
register definitions and archived H535 header likewise do not supply it.

The retained Xpsb archive contains binaries, not `psb_query.c` source. Its
assertion-string xrefs locate the actual private result getter at0x25451;
callgraph, callback installation and byte disassembly close the CPU query
chain above. A full Xpsb instruction-operand search finds the indexed read
expression, with no direct stores naming these eight offsets. Dynamic accesses
and ISP reset cannot be converted into an absent hardware counter specification.
The DDK VISTEST-support option and EMGD advertised occlusion-query support do
not define a target counter. DDK allocator/HWPerf/control structures manage
memory/events; they do not decode SGX535-generated triangle region records.
See the audit's search inventories and extracted disassemblies for exact refs.

## Why it cannot answer FIRE #2 coverage

**CONFIRMED:** the frozen CPU serializer explicitly selects the **no-query**
variant: [state](../../tools/psb-dri-re/frozen_triangle_image.py#L38) has no second
ISP word/query-enabled form, and [serialized object policy](../../tools/psb-dri-re/frozen_triangle_image.py#L122)
says no query. The register list does not initialize/read these eight results;
the historical submission descriptor also has no feedback. This establishes
software selection, not a speculative claim about what an unconfigured
hardware counter would contain.

A passive MMIO reader cannot retroactively enable/count just this triangle.
Changing query/ISP state would change the rendering configuration and is outside
this task. Even for a future configured query, zero could mean no geometry,
disabled/wrong slot, visibility rejection, lost/stale result or a counter
lifecycle error. A positive count would prove selected-triangle raster visibility
only after selection, freshness and the exact counting predicate are qualified.
It would distinguish fragment/PBE failure only if that predicate is proved
independent of the suspected fragment export and store stages. Neither property
is established here. Background/bounds contributions must also be excluded.

Thus the precise VISTEST gaps are: **the rev121 selected-primitive counting
predicate; its enabled/index mapping for this draw; its fresh reset/latch/read
contract; and operation-exclusive readout before invalidation.** The frozen
no-query selection independently prevents qualifying an unchanged passive reader.
No VISTEST data was retained in FIRE #2, so the sealed attempt cannot be decoded
retroactively through this mechanism.

## Minimum alternative: decoded TA output plus CPU publication

The alternative remains a **bounded, SGX535 rev121-specific decoder/publication
contract**, not a generic arena dump. Two facts must be supplied together:

1. A typed current-scene record that identifies the selected triangle separately
   from bounds/termination/background, with valid primitive/region links,
   termination rules and enough defined region membership or geometry to prove
   surviving selected-triangle admission/coverage. Full vertex decoding is not
   mandatory if a smaller target-defined record proves that property.
   For a negative result, traversal must be complete; a zero/changed word, hash,
   page count or general tile event is insufficient.
2. A hardware-to-CPU publication guarantee at the accepted-TA/pre-raster boundary:
   the relevant output is in those pinned pages, with no still-private device
   cache/state or pending update, followed by the applicable CPU coherency steps.
   `clflush`/`dma_rmb`, sleeping and TA_FINISHED alone do not establish device
   publication. Existing GPU-to-GPU handoff is not a CPU publication contract.

**CONFIRMED:** [scene cookies](../../tools/psb-dri-re/frozen_kernel_contract.c#L320)
provide offsets and sizes; TA uses scene base in0x220 and base+0x1000 in0x21c;
[raster](../../tools/psb-dri-re/frozen_kernel_contract.c#L582) uses base+0x1000 in0x408.
**INFERRED:** the latter participates in scene handoff. The target definition of
that root, link encoding, primitive ownership and complete traversal is still
**UNKNOWN**. Historical `Xpsb_scene_info`/scene bind routines and
[PSB scene code](../poulsbo-data/PSB_psb_scene_c.txt#L110) allocate, clear, bind and
reuse pages; they do not parse the hardware-produced primitive/region format.
The related DDK sources do not supply that missing decoder/publication contract.
An input index packet is not proof of its surviving TA output.

The potential capture point is
[fixed_service_status_impl](../../tools/psb-dri-re/frozen_fixed_service.c#L208),
after qualified TA acceptance and before `session_begin_raster`/the existing ISP
bracket. Current before/after taps do not retain that checkpoint. A future
observation must retain an immutable snapshot of only the defined decoded range,
bound to the existing current-operation capsule and owner lifetime, with complete
range/chain/interference/loss validation. Release/reuse must never substitute
bytes. No new identifier is demonstrated necessary. No checkpoint or decoder is
implemented until both semantic facts above are known; capturing uninterpretable
bytes would not close coverage.

If decoded coverage is present, the next discriminator needs **independent
fragment color-export evidence versus destination-store/publication evidence**
for the same operation. VISTEST does not presently provide that distinction.
The suffix-only fragment concern remains **unproven**; PBE/store remains
**unresolved**. No shader, geometry, render register or PBE correction is justified
by this review.

## Preservation, verification and stopping boundary

- **21/21 CPU reference extraction/consistency checks PASS.** They verify ELF
  identities, dispatch, offsets, slot selection, enable packing, CPU initialization
  and frozen no-query selection. They are **not GPU semantics qualification**.
- No observation mechanism was justified or implemented. No native/UBSan/i386
  mechanism qualification, module build, image build, staging or live capture ran.
  Driver/observer/image/client/UAPI identities are unchanged.
- All68 FIRE #2 files match the initial SHA-256 preservation snapshot. Completion,
  retirement and common-operation provenance remain CONFIRMED; readback remains
  all zero and post-TA triangle coverage UNKNOWN.
- Only this report/structured finding and the handoff continuation are changed
  in the repository. Existing dirty/untracked work is preserved; index stays empty.
- Boundary hygiene: a broad file search inadvertently returned excluded current
  entry/reporting-source matches. Those matches were discarded and were not used
  to infer, reconstruct or specify any behavior; no finding above depends on them.

**Smallest next justified step:** obtain an independently permitted rev121
primitive/region record and pre-raster CPU-publication contract sufficient for
the bounded decoder above. A VISTEST route additionally requires a qualified
selected-triangle query configuration, which cannot be added under this task's
unchanged-rendering boundary. Do not produce a candidate for an uninterpretable
capture or FIRE again on the strength of these reference findings.

This task: **SGX invocations0; hardware interactions0;
sgx_execution_authorized=false; triangle NOT ESTABLISHED.**
