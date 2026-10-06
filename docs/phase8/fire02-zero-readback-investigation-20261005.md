# FIRE #2: all-zero readback investigation, 2026-10-05

No concrete causal defect is established by the available retained evidence.
No implementation correction, new module or new image is justified yet.
[Structured finding](fire02-zero-readback-investigation-20261005.json) records
checks, ranked hypotheses and the distinguishing observations. This is offline
analysis, not another execution or a revision of the sealed outcome.

## Preserved facts

The [sealed FIRE #2 record](/home/gama/sgx535-offline/phase8-dhost-hold-fix-20261005T224726Z/live-preparation/fire-one-authorized-call/)
retains successful ioctl/client/operation results, phase 9, ledger 7, retirement,
and same-operation response/capsule/image provenance. Those findings stand.
The color payload is exactly 4096 zero bytes (SHA-256
`ad7facb2586fc6e966c004d7d1d16b024f5805ff7cb47c7a85dabd8b48892ca7`).
Its all-zero initialization and a later write of zero are indistinguishable in
this evidence. Completions establish the accepted operation lifecycle; they do
not report generated primitives, covered fragments, shader output values or
PBE store addresses. Triangle remains **NOT ESTABLISHED**.

All 18 originals match their sealed manifest. All 68 files under the FIRE
root were hashed before analysis and compared afterwards; none was changed.
The consumed authorization is not renewed.

## Full pixel path and limits

| Stage | Existing evidence and implemented path | Finding |
|---|---|---|
| CPU geometry | `frozen_triangle_image.build`: three position4f/color4f records, white RGBA, 96 bytes; indices 0/1/2; vertex PDS stride32 and width8 | CONFIRMED emitted input. Width/format match P7G-006/007; actual hardware vertex output not retained. |
| State upload | `bounds_state`, `triangle_state`, `terminate_state`; masks54c5/0f41/2000; 11/14/2 dwords; generated USE/PDS and qualified relocations | CONFIRMED serialized bytes match P7H-043. CPU matching does not certify every PDS launch input or hardware state effect. |
| TA stream | 68 bytes: bounds state/draw, triangle state/draw, termination state/end; triangle packet81400003, final word04000203 | CONFIRMED matches P7G-009 and actual retained encoder at ELF3b95f–3b969,3ba4b–3ba83. Primitive code0 here is the reference triangle code; replacing it with GL enum4 is unjustified. |
| TA scene output | Scene cookie, hardware VA and same context retained; TA completion accepted before raster | CONFIRMED lifecycle. FIRE #2 contains no post-TA region/primitive output snapshot. **Earliest unobserved production edge:** initialized inputs → triangle coverage actually produced in scene. |
| ISP/raster | Service accepts TA, begins raster, brackets ISP reset, writes26 raster pairs then20 fire actions; no-depth list matches retained2ab70 and old scheduler ordering | CONFIRMED intended action path, not a retained register readback or coverage measurement. ISP/front state01d00000 and smooth/no-cull10000 are reference CPU encodings, not newly decoded hardware flags. |
| Fragment | Primary PDS56 bytes, data count12; secondaryaf000000; middle launch30000; linked USE00000000/f8040140 | Exact current emission CONFIRMED. Existing FG-01 trace removes four virtual color MOVs and returns zero compiled attributes; link adds suffix only. The suffix's ISA/output semantics and implicit forwarding remain unestablished. **Do not call it a NOP without evidence.** |
| Surface/color | cpp4/pitch32/stride128; target wordsf8000,GPU(color),0,1f01f; background references same color allocation | CONFIRMED against retained392ab/4b710 and shared-surface evidence P7H-048. Full address is retained for color; background geometry strips high region bits under BIF_3D_REQ_BASE30000000; PDS uses base20000000. No host address mismatch found. |
| Event/PBE | Event USE, helper and PDS; rastera5c references event PDS,a60=4,a64=4fff | CONFIRMED reference emitter correspondence. The EMGD public header identifies completion bits, not these pixel-store instructions or a written-pixel count. Actual PBE destination/value is not in the retained evidence. |
| Ownership/MMU | Owner allocates private pages, maps each descriptor VA with no read-only flag, initializes/relocates, flushes and drops user CPU mappings | No concrete alias, wrong mapping or later CPU-clear defect found. Host correctness does not independently establish every device cache/format interpretation. |
| Retirement/readback | Same owner's COLOR page, phaseRETIRED, one mapped page; clflush/dma_rmb/kmap, summary/copy/capsule from same bytes | CONFIRMED 4096-byte same-operation capture. Neither CPU cache maintenance nor completion alone independently measures the preceding PBE write/visibility. Zero is not attributable to a row/channel permutation. |

Primary sources:
[scene serializer](../../tools/psb-dri-re/frozen_triangle_image.py),
[BO/relocation model](../../tools/psb-dri-re/frozen_triangle_bo.py),
[kernel contract](../../tools/psb-dri-re/frozen_kernel_contract.c),
[service](../../tools/psb-dri-re/frozen_fixed_service.c),
[backend](../../kernel/sgx535_frozen/gma500_fixed_backend.c),
[owner/readback](../../kernel/sgx535_frozen/gma500_bo_owner.c).
The excluded entry/reporting implementation was not inspected or reconstructed.

The strongest retained reference joins are
[P7G draw formats](../phase7/psb-dri-re/frozen-draw-closure.md),
[P7H scene specification](../phase7/psb-dri-re/frozen-triangle-spec.md),
[shared target](../phase7/psb-dri-re/frozen-triangle-burn-down.md),
[fragment compiler result](../phase7/psb-dri-re/frozen-fragment-exact-output.md),
[fragment link](../phase7/psb-dri-re/frozen-fragment-link-progress.md),
[launch resources](../phase7/psb-dri-re/pds-launch-count.md),
[EMGD SGX535 register definitions](../poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt),
[PSB scheduler](../poulsbo-data/PSB_psb_schedule_c.txt), and
[PSB driver bases](../phase4-2-data/antix-source/drivers/gpu/drm/gma500/psb_drv.c).
No semantics were imported from SGX544 or Rogue. Nine bounded retained DRI
encoder disassemblies are preserved with the analysis.

## Hypotheses ranked by specific direct evidence, not probability

1. **Intended fragment color export is missing or misconfigured.** The exact
   suffix-only program and zero attribute count are a concrete suspicious
   configuration, not proof of a hardware defect. A valid SGX535 decoding/output
   contract or attributable fragment-output observation must distinguish it.
   Guessing a replacement MOV, changing resource counts, or calling the suffix
   a no-op would exceed the available evidence.
2. **No surviving triangle geometry/coverage reaches raster.** TA completion is
   compatible with processing an empty/rejected scene. Serialized geometry is
   correct relative to the reference, but produced scene/coverage was not saved.
   No concrete wrong packet, cull flag or state count was found.
3. **Target/event/PBE store or device visibility fails despite terminal events.**
   Actual store destination/value is absent. Reference-matching descriptors and
   correct host-page capture weigh against simple host address/copy mistakes,
   without excluding device interpretation or GPU cache behavior.

The old evidence cannot select among these. It also cannot distinguish no
store from stores of zero. An all-zero image stays zero under every coordinate,
channel or tiling permutation: changing layout alone cannot restore white.
The sealed byte agreement excludes a short export or an offline decoder that
silently omitted nonzero pixels.

## Minimum distinguishing observation

The earliest production checkpoint is **after accepted TA completion and before
ISP reset/raster scheduling or later scene deallocation**. Independently retain
and interpret whether the current scene contains the intended triangle's region/
primitive coverage, bound to its published inputs and addresses. A raw nonzero
scene hash alone is insufficient: the interpretation must be qualified. This
splits empty/missing geometry from the downstream fragment/store branch without
altering the workload. If triangle coverage is present, the next fact needed is
valid fragment color export and its PBE destination/value/visibility.

No existing FIRE #2 artifact contains those facts; they cannot be recovered
retroactively from ledger7 or zero pixels. This note does not pretend that a
named read-only counter or a verified scene decoder already exists. A blind
third FIRE is not justified. A narrowly qualified production checkpoint or a
verified output-encoding correction would justify a fresh candidate and fresh
passive live qualification; execution still needs separate authorization.

## Focused offline checks

[Analysis and reproducible audit](/home/gama/sgx535-offline/phase8-zero-readback-20261005T233046Z) contain:

- Seven permitted source files equal exact qualified build inputs.
- Extracted **data symbols only** from the exact module:232 initial stores
  (2784 bytes) and49 relocation records (1960 bytes), both byte-identical to the
  permitted includes. No excluded caller/control-flow data was inspected.
- Five distinct synthetic address matrices, including USE offsets20000,38000,
  78000 and a moved base: all six complete C-initialized/relocated user backings
  equal the independently constructed Python images, in native and UBSan modes.
  Geometry, index/PDS, target/background same-color and event-reference checks
  pass. **60 complete backing comparisons**, no sanitizer errors.
- The initial zero target fails both full triangle references. This checks claim
  discipline, not a hardware simulation of the missing pixel mechanism.

These checks narrow software serialization/addressing hypotheses; they neither
reproduce a GPU zero-output mechanism nor establish undocumented ISA semantics.
Existing unrelated passing qualification was not rerun. Qualified implementation,
client, UAPI, module and image bytes are unchanged; no qualification is invalidated.
Driver Build ID remains `9d0b5b2fdb7d9881f2828f43fb89253176c38817`;
observer Build ID remains `f11d3abb072caa4e1d32836ef92ce201e9c9d126`;
image remains SHA-256 `ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`.

This task: hardware interactions0; SGX invocations0; no retry; no client execution.
Historical authorized client invocations2. `sgx_execution_authorized=false`.
Triangle **NOT ESTABLISHED**. No new candidate was produced.
