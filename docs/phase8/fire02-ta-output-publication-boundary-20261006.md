# FIRE #2 — bounded TA-output and Vita comparative evidence

2026-10-06. **Endpoint B: target record decoding and CPU publication remain
unestablished. No rendering defect, decoder or new live candidate is claimed.**

The earliest unresolved pixel-path question is still whether the selected
triangle survived TA into raster-consumable output. The independent Xpsb
CPU parameter writer supplies a narrower format lead. Permitted Vita sources
supply comparative allocation and synchronization relationships, not the
missing SGX535 rev121 TA-emitter/publication contract.

[Structured finding](fire02-ta-output-publication-boundary-20261006.json).
[Offline source/search/disassembly audit](/home/gama/sgx535-offline/phase8-ta-output-boundary-20261006T003026Z/).
The [VISTEST finding](fire02-vistest-lifecycle-20261006.md) remains closed: the
unchanged frozen scene selects no query. This task does not reopen completed
completion/provenance, Gate B, lifecycle, client or rendering-state reviews.

## Evidence levels and boundaries

**CONFIRMED** below means the cited source/data-flow/CPU byte fact, not an
uncited hardware interpretation. **INFERRED** means the hardware role is
consistent with that evidence. **UNKNOWN** means the target guarantee has not
been established. Every Vita-derived GPU relationship starts as
**FAMILY-LEVEL INFERENCE — SGX543**; none is promoted to SGX535 merely by a
matching name, constant or pipeline concept.

Only permitted project source, public historical Poulsbo references and
independently permitted open-source comparative sources were used for the
findings. The excluded current ioctl-entry/reporting implementation was not
inspected or reconstructed. One fetched Vita low-level-header branch labeled
itself Strictly Confidential. Its preamble and a combined search returned in
the same call; the definitions were excluded from interpretation, design and
cross-correlation. See the audit's `vita-comparison/excluded-branch.json`.
Public accessibility is not treated as unrestricted input permission.

## Actual frozen memory path

Let `S`, `P`, `Q`, `V` and `C` be the current owner's GPU VAs for SCENE_HW,
TA_PAGE_TABLE, TA_PARAMETER, VERTEX_TA and COLOR. They are variables, not newly
observed FIRE #2 addresses. The sealed attempt did not retain a runtime BO map.

| CPU object / owned pages | Qualified size, alignment, VA domain | TA producer / raster consumer relationship | CPU observation and evidence level |
| --- | --- | --- | --- |
| VERTEX_TA | 4096 B / 4096; MMU, starting at 0x40000000 | CPU input vertices, indices and TA stream; shader/PDS references are relocated into this owner | **CONFIRMED** CPU-authored input; an input echo is not TA-output survival |
| SCENE_HW | 8192 B / 4096; MMU | Scene cookie selects `S` for write 0x220; `S+0x1000` for TA write 0x21c and raster write 0x408 | **CONFIRMED** common software address handoff; **INFERRED** raster-input region/list root; exact entry semantics **UNKNOWN** |
| TA_PAGE_TABLE | 32 MiB / 4096; MMU | `P` is written to 0x600/608/610/618; DPM/free-list load state uses these addresses | **CONFIRMED** distinct allocator object; not the SGX MMU's own PTE table; entry format/produced contents **UNKNOWN** |
| TA_PARAMETER | 32 MiB / 1 MiB; RASTGEOM, 0x30000000–0x3fffffff | `Q & 0x0fffffff` defines the selected parameter-page interval, 8192 4-KiB pages; loaded for TA/raster/host data masters | **CONFIRMED** software arena and page bounds; **INFERRED** primitive/parameter storage; exact primitive addressing/layout **UNKNOWN** |
| PDS / USE | 128 KiB / 4096; 32 KiB / 32 KiB; 0x20000000–0x2fffffff | CPU programs and references feeding vertex/fragment/event processing | **CONFIRMED** input objects, not a retained generated-primitive witness |
| BACKGROUND | 4096 B / 4096; RASTGEOM | Separate CPU-authored background object used by raster state | **CONFIRMED**; must not be mistaken for the selected triangle |
| COLOR | 4096 B / 4096; MMU | Selected surface address; retirement-gated capture copies this owner's color page | **CONFIRMED** FIRE #2 retained color provenance; pixels all zero, not geometry evidence |
| CONTROL / XHW_COMM | 4096 B each; LOCAL | CPU control/command descriptors; no allocated GPU VA | **CONFIRMED** not parameter/region output |

Direct sources:
[requirements](../../tools/psb-dri-re/frozen_kernel_contract.c#L8),
[scene cookie](../../tools/psb-dri-re/frozen_kernel_contract.c#L320),
[TA allocator cookie](../../tools/psb-dri-re/frozen_kernel_contract.c#L376),
load plan at441–505, TA plan at545–556, raster plan at611–618;
[owner allocation](../../kernel/sgx535_frozen/gma500_bo_owner.c#L289) through378.
The MMU upper bound is the platform's `mmu_gatt_start`; the private VA allocator
excludes the GTT aperture. These objects are individually pinned shmem/GEM pages,
mapped into the default SGX page directory with `PSB_MMU_CACHED_MEMORY`. The
TA allocator is not interchangeable with that page directory. Only roles
through COLOR receive private CPU vmaps; SCENE/PT/PARAM retain pages but no such
view. No exported GEM handle creates a userspace TA-output reader.

Independent historical counterpart:
[PSB scene allocation](../poulsbo-data/PSB_psb_scene_c.txt#L110) lines129–143
allocates `scene->hw_data` from Xpsb's reported size; `scene->hw_scene` is a
different software context. Lines248–265 pass `ta_mem->ta_memory` and
`ta_mem->hw_data` separately to TA memory load. Lines432–482 allocate distinct
MMU and 1-MiB-aligned RASTGEOM arenas. Similar labels are not collapsed into one
object.

Historical Xpsb `scene_info` ELF0x3a40–3ce1 derives a 16-unit screen grid,
power-of-two allocation and a 12-times-grid-area term. For32×32 the selected
cookie is `0,10,01004004,01004004,10,1001,1f01f,1000,0,1300,1350,13e0,0,0,0,0`,
reported size0x1420 and one initial clear page. The qualified owner rounds the
allocation to0x2000. **CONFIRMED:** these CPU calculations. **INFERRED:** a
relationship to tiling. **UNKNOWN:** physical region dimensions, whether the
12-byte allocation term is a region-header stride, and the contents/types of
each scene subrange. It cannot qualify an empty-region decoder.

## New independent primitive/list lead: CPU-generated Xpsb quads

The historical `Xpsb.so` SHA-256 is
`da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f`.
All following addresses are ELF addresses. Bounded excerpts and float constants
are retained in the audit; they come from the public historical binary, not the
excluded current entry implementation.

| Relationship | Direct CPU evidence | Applicability to TA-generated selected triangle |
| --- | --- | --- |
| Block construction | `XpsbResetParamContext`0x74c0; `XpsbAddQuad`0x7b50 copies four vertices and flushes at three quads; `XpsbFlushParamblock`0x7560 emits destination bytes | **CONFIRMED** CPU quad writer; not a hardware TA-output reader |
| Primitive/index-like fields | 0x7608–7654 records `4q`, `2q`, masks and a flag in 28-byte **software metadata**; 0x767f/7686 emits0x07e80000/0x02000000; 0x76a0–76d7 packs paired index fields | **INFERRED** hardware primitive-block interpretation; the metadata is not itself a region header |
| Coordinates | 0x7738–7780 converts x/y with `(coord+1024)*16+0.5`, truncation, x in upper16 bits/y lower16; adjacent word1.0. ELF constants0xd4d4/d8/dc are1024/16/0.5 | **CONFIRMED** writer encoding. Frozen CPU background words independently agree with this packing. TA-emitted triangle coordinates/depth/interpolation format **UNKNOWN** |
| Object references / end | `XpsbFlushParamContext`0x7c90 emits two-word references from the metadata, with a relocation at0x7dda; terminator0xc0000000 at0x7df6 | **CONFIRMED** CPU list writer. TA empty/continuation/end tags and pointer address-space selection **UNKNOWN** |
| ISP consumption | Composite finish0x5c81→0x7560 and0x5c89→0x7c90;0x5e52 calls raster builder0x5830; builder0x59d5 writes0x408 with a relocated parameter-stream address;0x5ea5 calls `XpsbFlush3D` | **CONFIRMED** direct CPU-composite-to-raster path; **INFERRED** related ISP parameter/list concept, not proof of TA region output identity |

For q=1..3, i=4×quad index, the CPU index word is
`0x42104210 | i | (i<<16) | ((i+1)<<26) | ((i+3)<<21) | ((i+3)<<10) | ((i+2)<<5)`.
The first reference word is
`0x40200000 | primitiveMask | (1<<22) | (vertexMask<<8) | ((vertexCount-1)<<25)`;
the second includes a shifted/masked relocation and `(primitiveCount-1)<<27`.
This establishes relationships worth comparing against a legitimate TA format
provider. It does **not** establish a SGX535 rev121 TA-emitter layout, region
root type, block chaining, clipping/culling semantics, plane equations or complete
reference traversal. The software writer can bypass TA for composited quads.
Its two-word entries, 28-byte host metadata and scene allocation's12-byte term
describe different objects; treating them as one format would be an error.

## Selected-triangle signature: known input, no qualified output signature

[Frozen serialization](../../tools/psb-dri-re/frozen_triangle_image.py#L101)
contains vertices `(8,8,0.5,1)`, `(24,8,0.5,1)`, `(8,24,0.5,1)`, white attributes,
stride32 and indices0/1/2. **CONFIRMED mathematical input:** area128 square
pixels, bounds8..24, edges x=8, y=8, x+y=32. These distinguish the intended draw
from the full-surface bounds and background inputs.

**Conditional hypothesis only:** if TA uses the demonstrated CPU x/y packing,
the three packed coordinate words would be0x40804080,0x41804080,0x40804180.
The CPU quad writer's adjacent1.0 differs from the selected input z=0.5; no target
TA depth/plane layout is established. Matching these words or copied floats
would not prove typed primitive identity, reachability, freshness or coverage.
The false-positive risk is unbounded without those conditions: stale parameter
pages, an input echo, incidental words or another primitive remain possible.

If regions were16×16, interiors would intersect three grid cells, with boundary
and conservative-binning rules affecting references in a fourth. If32×32, all
geometry is in one tile. Neither assumed membership is an ABSENT criterion.
The actual target region/list rules must be proved before using this geometry
as a signature. No numerical false-positive probability is claimed.

## Vita / SGX543 comparative corpus and cross-generation limits

Pinned sources/commits and bounded searches are preserved under
`vita-comparison/`. The following GPU findings are all initially
**FAMILY-LEVEL INFERENCE — SGX543**, even though the source-level API facts are
directly readable. They are hypothesis generators, not target layout providers.

| SGX543 comparative evidence | Likely family concept | Independent SGX535/Poulsbo counterpart | Confidence / unresolved difference |
| --- | --- | --- | --- |
| VitaSDK `gxm.h`49–65: PB location/shared-buffer flags and parameterBufferSize; libvita2d244 sets the size | Separate scene/parameter allocation | Distinct scene, allocator and RASTGEOM arena in PSB and frozen owner above | Relationship independently supported; Vita sizes, sharing, LPDDR/CDRAM policy not transferred |
| VitaSDK renderTargetParams1386–1395: width/height/scenes and uncached driverMemBlock for GPU structures; CreateRenderTarget1888–1891 | Per-render scene metadata distinct from color | `scene->hw_data` allocation from Xpsb scene info | Concept supported; opaque Vita object exposes no parameter/region encoding; target current CPU mapping is cached |
| VitaSDK tile macros1290–1293:32×32 | Screen partitioning | Xpsb computes a16-unit screen grid at0x3a51–3a5a | Literal transfer **rejected**; SGX535 physical tile vs allocation-grid distinction still UNKNOWN |
| BeginScene/MidSceneFlush/EndScene1641–1643 carry vertex/fragment synchronization or notifications | Distinct vertex/fragment progression | PSB TA_DONE vs RASTER_DONE/SCENE_DONE fence bits; fixed service accepts TA before beginning raster | Stage relationship supported; no CPU-publication guarantee for TA arenas follows from API signatures |
| libvita2d creates GXM context/rings and opaque render target, then finishes before teardown | GPU-owned scene lifetime and mapping | Pinned owner pages, gated retirement/release | Useful lifetime comparison; neither source decodes a TA-produced primitive |
| Vita3K CreateRenderTarget2055–2086, draw path and Finish2697–2717 translate GXM to its host renderer | API-level scene/draw relation | Xpsb hardware path is independently traced | Host OpenGL/Vulkan data is **not** SGX543 TA output; GetRenderTargetMemSize2858–2864 is a64-KiB stub, not a hardware allocation formula |
| vitaGL `source/gxm.c`, NanoVG-GXM draw wrapper, SDL GXM backend | Homebrew depends on the GXM scene/draw API | Frozen path programs owned hardware objects directly | No primitive/region decoder or pre-raster publication primitive found in these bounded implementations |

Primary source links:
[VitaSDK header](https://github.com/vitasdk/vita-headers/blob/e66ebe90b73d1fa4cce005a5b2072cec27322544/include/psp2/gxm.h),
[libvita2d implementation](https://github.com/xerpi/libvita2d/blob/a8f15ab09d5233f0a4e4ad0e8f6ade0da888cbed/libvita2d/source/vita2d.c),
[Vita3K GXM implementation](https://github.com/Vita3K/Vita3K/blob/29ffbcfe727aa74d509135a4861b5bc4752140d4/vita3k/modules/SceGxm/SceGxm.cpp),
[vitaGL](https://github.com/Rinnegatamante/vitaGL/blob/c9bcd7423438163290fb888cc44ab30032ca2dc8/source/gxm.c),
[NanoVG-GXM](https://github.com/xfangfang/nanovg-gxm/blob/ffbe2a0d4938bd5d90bba424d9f28b57bcb49cd5/src/nanovg_gxm.h),
[SDL GXM backend](https://github.com/libsdl-org/SDL/blob/21528641fc0b543c1e24c131e4a5869bcd35ee24/src/render/vitagxm/SDL_render_vita_gxm.c).

VitaSDK/libvita2d MIT, Vita3K GPLv2, vitaGL GPL/LGPL and NanoVG/SDL license
records were checked. The samples and psvgxp leads supply no needed format and
their input permission was not independently established here; neither was
used as a semantic provider. The SGX543 wiki could not be retrieved through its
anti-bot page; architecture summaries do not qualify bitfields. The excluded
low-level PVR_PSP2 branch was not pursued. This is exhaustion of identified,
available, permitted relevant sources, not a claim that every possible private
or future reference has been searched.

**Result:** Vita materially clarifies what can and cannot be compared, but does
not close either target decoder gate. No permitted SGX543 TA-output record layout
was found to test against the current scene's address/alignment/size constraints.
No Vita fact was promoted into an SGX535 bitfield definition. ARM uncached GXM
memory and host-emulator notifications do not establish Poulsbo GPU publication.

## GPU-to-CPU publication: exact unresolved contract

| Candidate boundary / mechanism | Established fact | What is still not established |
| --- | --- | --- |
| Host initialization / pre-fire publication | Owner clears new backing pages, flushes all BO pages and executes dma_wmb before use | This is CPU→GPU publication, not writeback of generated TA output |
| Accepted TA checkpoint | [Fixed service](../../tools/psb-dri-re/frozen_fixed_service.c#L211) accepts status, then begins raster at226; historical [TA done](../poulsbo-data/PSB_psb_schedule_c.txt#L421) reports a fence and schedules raster | Which scene/parameter data is materialized in CPU-readable backing RAM; whether TPC/OTPM/DPM state remains resident; stability during snapshot |
| CPU mapping / CLFLUSH / dma_rmb | CPU can map pinned pages; existing color capture uses these after retirement | CPU cache invalidation/order alone does not force device-internal TA data into these pages; the color contract is not automatically a TA-arena contract |
| Historical scene mapping | [Scene clear](../poulsbo-data/PSB_psb_scene_c.txt#L30) maps and zeroes pages | This writes initialization; it is not a read/decoder of GPU-produced scene data |
| Historical cache/fence helpers | [psb_invalidate_caches](../poulsbo-data/PSB_psb_buffer_c.txt#L196) returns0; scene fences gate reuse | No explicit TA-arena GPU→CPU drain/invalidation guarantee found |
| MMU flush | [BIF/MMU flush](../poulsbo-data/PSB_psb_mmu_c.txt#L141) operates translation state | Not proof of primitive/region output writeback |
| Xpsb context load/store | ELF0x4208–4277/0x4318 onward programs scene subranges and DPM load/store controls | DPM context-store completion is not shown to publish every necessary primitive/list byte; no extra store/flush/reset is proposed |
| End-render / memory-free / retirement | FIRE #2 completion and color provenance are established; owner pages remain pinned until release | No guarantee that TA parameter/list contents remain unchanged rather than being reclaimed/reused internally; later snapshot may be too late |

The closest **prospective** checkpoint is after accepted TA and before
`begin_raster`. It is not yet a proved safe immutable CPU snapshot point. The
minimum missing publication primitive is a rev121-valid guarantee that the
specific raster-input scene/list/parameter bytes have reached their owned
backing pages and will remain stable through a bounded CPU copy, without an
extra submission, completion clear/reset or changed rendering semantics.
This does not assert that such a passive guarantee is architecturally impossible.

## Retrospective limit, decoder/capture gates and next action

**FIRE #2 post-TA coverage cannot be resolved retrospectively.** Its18 originals
retain the response, color, capsule, source/context/streams/kernel evidence, not
SCENE_HW, TA_PAGE_TABLE, TA_PARAMETER or region/primitive snapshots. The capsule
contains terminal provenance and color, not those arenas. All68 sealed files
remain byte-identical; no unpreserved bytes were inferred or synthesized.

Two independent minimum gates remain:

1. **Format/reachability:** a SGX535 rev121 interpretation connecting the selected
   `S+0x1000` raster root through valid region/list/parameter pointers to a typed
   triangle record, including empty/end/continuation rules and enough geometry/
   state information to distinguish this triangle from bounds/background.
2. **Publication/lifetime:** the specific bytes above are CPU-visible and stable
   at a passive, attributable checkpoint before loss or recycling.

The first missing arrow is the raster root's entry/reference interpretation;
the independent second missing arrow is GPU-internal TA state→stable backing
bytes. Allocation sizes, completions, quad encodings and family analogies do not
supply either guarantee. No speculative decoder or opaque dump was implemented.
Native/UBSan/i386 decoder qualification is **NOT RUN: decoder gate not met**;
no module/observer/image/client/UAPI change or rebuild is justified.

Once both gates have permitted primary support, the minimum witness must:

- Bind owned address ranges and layout metadata to the fresh capsule/session;
  snapshot only the required reachable hierarchy at the proved publication point.
- Preserve immutable raw originals before interpreting them, before owner release,
  and without advancing raster concurrently with a snapshot that requires stability.
- Return SELECTED_TRIANGLE_PRESENT only for a typed, reachable selected triangle
  with consistent geometry/state/region associations. Presence is raster input,
  not proof of a colored pixel or fragment execution.
- Return SELECTED_TRIANGLE_ABSENT only after complete valid traversal with no such
  triangle. Truncation, bad alignment/ranges, ambiguous tags, cycles/duplicates,
  stale/reused ownership, interference or publication/evidence loss return UNKNOWN.
- Qualify positive, absent, unrelated/bounds-only, malformed, wrong-region,
  wrong-owner/stale, truncated, invalid-pointer, duplicate/incomplete-list,
  terminator/alignment and publication-loss fixtures offline before capture support.

**Smallest next justified action:** obtain a permitted target-specific account
of that raster-root→primitive relation and its publication boundary, starting
from the independent Xpsb CPU parameter/list relationships retained here. Do
not acquire excluded definitions through a different route. Another live capture
is not yet technically justified: it would retain uninterpretable/unpublished
bytes. FIRE #3 remains separately unauthorized.

If coverage is later established, the next discriminator is qualified
current-operation evidence of fragment color production/export at the USE→PBE
boundary, followed by target-valid PBE destination/store/visibility evidence.
No such observable is identified as qualified here. The suffix-only fragment
program/zero compiled attributes remain suspicious, **not a proven defect**;
PBE/store and legitimate zero output remain unresolved. No shader or rendering
register change is justified by this review.

## Verification and preserved authority

The audit's **40/40 PASS** consistency checks validate cited CPU/source
relationships, input hashes, local links, unchanged sealed evidence and scoped Git changes. They are
reference checks, **not hardware or TA-format qualification**. Implementation
and candidate identities remain unchanged:

- Driver Build ID: `9d0b5b2fdb7d9881f2828f43fb89253176c38817`.
- Observer Build ID: `f11d3abb072caa4e1d32836ef92ce201e9c9d126`.
- Image SHA-256: `ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`.

Only this report, its structured finding and the continuation handoff are
changed/created in the project; audit/reference files reside outside it.
Pre-existing dirty work is preserved; index remains empty; no staging/commit.
Git status is19 modified tracked entries and120 untracked entries; the task
started with19/118 and adds only the two reports plus a handoff edit.
Task SGX invocations0; hardware interactions0; sgx_execution_authorized=false;
historical approved-client calls2; both authorizations consumed; triangle
**NOT ESTABLISHED**. No new live-readiness claim is made.
