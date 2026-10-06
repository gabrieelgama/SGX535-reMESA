# FIRE #2 — scene/root GPU→CPU publication and stability, 2026-10-06

**C — GPU_CPU_PUBLICATION_UNRESOLVED.** Host ownership, mapping and CPU cache
maintenance are established. Historical event/fence transitions are established
as driver behavior. No permitted source establishes an event-specific postcondition
that the TA-produced normal-root bytes have reached owned system-memory backing
and remain unchanged through a CPU copy before internal reclamation. Therefore
no earliest safe snapshot, latest safe snapshot or stable interval is proven.
This is not classification D: no evidence proves that the current lifecycle
necessarily destroys the representation before any possible observation.

The smallest missing guarantee is:

> At a specified rev121 checkpoint before destructive DPM action/reuse, the
> TA-produced bytes in the owned `S+0x1000..S+0x1300` backing pages are externally
> committed and cannot receive later relevant writes through a bounded CPU copy.

A checkpoint satisfying that guarantee must also specify any required GPU
visibility operation. CPU cache maintenance is known and cannot supply the
missing GPU postcondition. The [structured finding](fire02-scene-root-publication-20261006.json) separates these
facts. Format is still **UNRESOLVED / PARTIAL**; both boundaries are independently
blocked. The exhausted root grammar was not searched again.

## Evidence and applicability

| Evidence | Tier | What it establishes / limit |
| --- | --- | --- |
| Existing rev121 Xpsb path, four bounded functions | DIRECT_REV121_EVIDENCE | Actual wait/save/read-lock behavior in applicable historical ELF; no root publication contract |
| [Historical scene owner](../poulsbo-data/PSB_psb_scene_c.txt), [scheduler](../poulsbo-data/PSB_psb_schedule_c.txt), [BO driver](../poulsbo-data/PSB_psb_buffer_c.txt), [fences](../poulsbo-data/PSB_psb_fence_c.txt) | POULSBO_DRIVER_BEHAVIOR | Cached scene allocation, fence types, reuse gating, separate render/deallocation stages; hardware postconditions not specified |
| [SGX535 event definitions](../archaeology-data/H535.txt#L189) | SGX535_EVIDENCE_REVISION_UNPROVED | Named TA, PBE, DPM store/free events; masks/names do not define root writeback or read stability |
| Current owner/service/backend/observer | POULSBO_DRIVER_BEHAVIOR, current clean-room implementation | Pinned pages, private VA, input publication, session ordering and release gates; not independent hardware semantics |
| Pinned TI DDK1.14 SGX/services files | SGX535_EVIDENCE_REVISION_UNPROVED for core branch; SERIES5_COMPARATIVE_EVIDENCE for common services | Separate CPU cache operations, GPU cache requests, sync/status and cleanup paths; firmware root publication implementation unavailable |
| Historical TTM/BO and exact antiX5.10.240 cache/GEM/barrier source | GENERIC_X86/TTM_BEHAVIOR | CPU mapping, pinning, cache maintenance and software waits; not SGX535 root visibility |
| Proposed checkpoint/postcondition composition | INFERENCE | A conditional future contract only; no proven snapshot mechanism |

[Inspected-input hashes](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/inspected-inputs.json),
[pinned TI commit/blob manifest](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/pinned-ti-manifest.json) and
[historical DDX extraction manifest](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/ddx-readback-manifest.json) preserve inputs.
TI commit is `cb46ba4d0c900f89f7ec0284f9803d476bfa98de`, the already established
rev121-capable Poulsbo target; shared services do not thereby become rev121
hardware guarantees. Historical TTM comes from pinned PSB commit
`98b5307e5158a9ac401b29128ddd1184ae06b4d7`. DDX0.32.0 exact revision pairing
is unproved. Current owner, service, backend and contract match the qualified
module build sources; observer matches its **separate observer build**, not
an inactive copy in the driver staging directory.

No restricted current entry/reporting implementation was inspected. No excluded
firmware/low-level source was recovered. Targeted public primary-source searches
provided services/header leads but no root-specific commit/stability guarantee;
non-primary/general GPU results were not used as technical proof.

## Scene/root ownership lifecycle

The target is the scene BO,8192bytes, with the bounded768-byte reservation in its
second page. The separate DPM allocator and parameter arenas are not aliases
of that BO. Host retention prevents Linux page/VA reuse; it does **not** prevent
internal DPM page reclamation or prove the contents survive it.

| Phase | CPU / ownership | GPU readers/use | Possible mutation | Host allocator reuse |
| --- | --- | --- | --- | --- |
| Allocation/initialization | Owns pinned pages; clears backing | Not yet admitted | CPU initialization | No while owner retains pages |
| Pre-fire publication | All BO pages CLFLUSH; input vmaps dropped | Mapped by SGX MMU cached PTEs | No intentional CPU payload writer after this boundary | No |
| TA active | No safe payload-read permission established | TA/DPM own execution; scene and separate arenas supplied | TA-generated writes; exact root vs on-chip staging unknown | No; owner unsafe release rejected |
| Accepted TA completion, before raster start | Component retains ownership and pages | TA_DONE accepted; raster not yet kicked at this internal point | No complete all-writers/root-materialization guarantee | No |
| Raster/ISP active | No safe root-read contract | Root supplied to ISP; DPM state/deallocation machinery active | ISP root reader; write-free guarantee unknown; DPM root mutations unknown | No |
| End-render, deallocation pending | Pages retained | Historical scheduler says deallocation may remain busy | DPM cleanup may still act; exact root effect unknown | No |
| 3D-memory-free, possibly before end-render acceptance | Pages retained | DPM allocation reclamation event; separate end-render needed | Whether original root/parameter representation survives unknown | No host free; internal DPM reclamation is separate |
| Scene completed / RETIRED | Software ledger7 then release USE reservation and retire; pages still held until release | Accepted TA/end-render/memory-free transitions satisfied | No target backing-memory stable-postcondition follows from phase assignment | No until owner teardown |
| Backend end / invalidation / owner release | Backend protection ends; MMU removal and page unpin/free on release | No retained-root contract | Reuse/remap/free possible; invalidation destroys prospective proof | Yes after teardown |

The current [owner construction](/home/gama/sgx535-gfx/kernel/sgx535_frozen/gma500_bo_owner.c#L288)
uses GEM shmem pages, zeros every page, and installs SGX MMU mappings. It creates
private CPU vmaps only through COLOR; SCENE_HW and TA arenas retain page arrays
without those views. The
[pre-fire flush](/home/gama/sgx535-gfx/kernel/sgx535_frozen/gma500_bo_owner.c#L156)
covers **every BO**, then drops input/color vmaps. This is CPU→GPU publication
and elimination of intentional later CPU writes, not proof of reverse visibility.

[Release](/home/gama/sgx535-gfx/kernel/sgx535_frozen/gma500_bo_owner.c#L92) rejects
unsafe post-issuance states, removes SGX mappings, drops reservations/mappings
and returns pages. A held page array prevents ordinary host recycling until
that teardown. It does not freeze normal-operation hardware writers.

## Possible writers, without assuming raster is read-only

| Agent | Classification | Scope / unresolved part | Evidence tier |
| --- | --- | --- | --- |
| CPU owner | WRITER | All owned pages initialized; no deliberate root writer after preflush; later teardown only | POULSBO_DRIVER_BEHAVIOR |
| TA | WRITER | TA-produced scene/parameter role established; exact root write timing and external commit unknown | INFERENCE from configured producer/consumer relationship |
| DPM | WRITER to allocation/context state; UNKNOWN for root payload | Separate control/state store events and context/arena setup; normal root mutation/postcondition unspecified | SGX535_EVIDENCE_REVISION_UNPROVED / POULSBO_DRIVER_BEHAVIOR |
| ISP/raster | READER; READER_ONLY not established | Configured root input consumed for rendering; absence of writes to the reservation not proved | DIRECT_REV121_EVIDENCE for programmed input; INFERENCE for memory access details |
| Microkernel/firmware | UNKNOWN for root payload | Comparative DDK manipulates sync/status/cleanup; not proof current path runs that firmware or root parser | SERIES5_COMPARATIVE_EVIDENCE |
| Reset/power/clear machinery | WRITER to state or invalidating transition; UNKNOWN direct root writes | Current initialization clears backing; later reset/power invalidates witness and cannot establish preservation | POULSBO_DRIVER_BEHAVIOR |

DPM control/state STORE and LOAD have distinct event bits in the SGX535 header
(lines227–238). This establishes that store-completion classes exist, not which
root bytes they publish. The current first-scene plans configure DPM/TA addresses
and load/clear/cache operations; they do not provide a proved post-TA root
writeback postcondition. [TA plan](/home/gama/sgx535-gfx/tools/psb-dri-re/frozen_kernel_contract.c#L506)
and [raster plan](/home/gama/sgx535-gfx/tools/psb-dri-re/frozen_kernel_contract.c#L582)
are data-only existing behavior, not instructions to run them. A store event
cannot be imported as a root publication primitive merely from its name.

## Events: execution, ownership, visibility and stability are different

| Event | Established source behavior | What is not established for root bytes |
| --- | --- | --- |
| Accepted TA completion | Current sequence-checked event adds TA ledger bit; historical `psb_ta_done` queues/schedules raster | All TA/DPM root writes committed to RAM; no later writer; CPU visibility |
| End-render | Historical handler adds RASTER_DONE; can report raster fence while cleanup remains busy | DPM finished; original root stable; any cache writeback |
| 3D-memory-free | Historical handler adds DEALLOC; required with RASTER_DONE for final raster completion | Original allocation/representation preserved; event itself publishes bytes; no later write |
| Retirement | Current ledger7→SCENE_COMPLETED, USE-release callback, then RETIRED software phase | Additional GPU drain, cache flush or root-preservation fence |

[Historical `psb_ta_done`](../poulsbo-data/PSB_psb_schedule_c.txt#L421) reports
TA_DONE and schedules raster. [Raster dispatch](../poulsbo-data/PSB_psb_schedule_c.txt#L834)
explicitly treats TA-memory deallocation as potentially still busy after raster
completion; only `RASTER_DONE | DEALLOC` reaches `psb_raster_done`. The
[IRQ dispatch](../poulsbo-data/PSB_psb_schedule_c.txt#L855) separates the hardware
classes. SCENE_DONE permits scene-context return/reuse
[after final raster processing](../poulsbo-data/PSB_psb_schedule_c.txt#L502).
These are strong driver lifetime facts, not a dump-stability promise.

Current [event acceptance](/home/gama/sgx535-gfx/tools/psb-dri-re/frozen_kernel_contract.c#L930)
and [retirement](/home/gama/sgx535-gfx/tools/psb-dri-re/frozen_kernel_contract.c#L1033)
perform sequence/order/phase checks. Retirement itself adds no memory operation.
FIRE #2 proves these accepted transitions occurred; it does not establish their
unwritten root-memory postconditions, and its completion/provenance result is
not reopened.

## Historical CPU access mechanisms

1. **Rendered pixmap access:** historical
   [EXA PrepareAccess](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/ddx-psb_accel.c#L647) uses `mapBuf` as synchronization
   even when a CPU address already exists. The
   [libmm wrapper](/home/gama/sgx535-offline/phase8-common-raster-abi-20261006T005131Z/xserver-xorg-video-psb-0.32.0/libmm/mm_drm.c#L155)
   calls drmBOMap. Historical
   [BO map](/home/gama/sgx535-offline/phase8-root-successor-20261006T015923Z/public-psb/drm_bo.c#L1191)
   waits for an attached fence and registers CPU mapping ownership. This
   supports CPU access to fenced external render buffers, not fidelity of an
   internal TA representation after reclamation. Unmap closes the CPU access.
2. **Scene reuse/clear:**
   [validation](../poulsbo-data/PSB_psb_scene_c.txt#L216) waits on scene BO before
   CPU clearing its specified prefix. Scene BOs request CACHED and SCENE flags;
   [fence selection](../poulsbo-data/PSB_psb_buffer_c.txt#L39) adds SCENE_DONE.
   This demonstrates a software overwrite/reuse boundary. It does not read
   TA-produced root contents or prove they remain in raster-consumable form.
3. **VISTEST feedback:**
   [post-raster request](../poulsbo-data/PSB_psb_schedule_c.txt#L489),
   [Xhw CPU copy-back](../poulsbo-data/PSB_psb_xhw_c.txt#L543), then
   [CPU accumulation and feedback fence](../poulsbo-data/PSB_psb_schedule_c.txt#L635).
   The feedback page is CPU-updated from register-read results. Its visibility
   cannot prove direct GPU-write publication of the scene BO. VISTEST format
   suitability is not reopened.
4. **Current color capture:**
   [retirement-gated readback](/home/gama/sgx535-gfx/kernel/sgx535_frozen/gma500_bo_owner.c#L206)
   CLFLUSHes one color page, orders CPU access, maps and copies it. It neither
   maps nor captures SCENE_HW. Its same-operation color provenance remains
   established; transferring the same helper to root memory would still require
   a root-specific publication/stability postcondition.
5. **DDK status/sync/cleanup:** common DDK prepares status destinations and sync
   counters; simulated CPU completion assignments in
   [sgxkick.c](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/pinned-ti/sgxkick.c#L813) are explicitly NO_HARDWARE. The
   [MEMREAD option](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/pinned-ti/sgx_mkif_km.h#L329) asks firmware to read device
   memory; it is not a passive CPU ownership grant and is absent from this
   qualified path. [HWRT flush](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/pinned-ti/sgxutils.c#L1889) invokes cleanup,
   not a retained scene-byte snapshot contract. No such command is proposed.

Bounded rev121 ELF inspection provides negative guarantees against misleading
helper names: [Xpsb_ta_mem_save](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/ta-mem-save.disasm.txt) at0x3d60 returns0
without saving memory; [ReadLock/ReadUnlock](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/kernel-bo-read-lock.disasm.txt)
at0xa830/0xa850 increment/decrement a host counter. Neither is a hardware drain
or cache publication primitive. [BOWaitIdle](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/bo-wait-idle.disasm.txt) delegates
to the BO manager; [kick/wait/clear](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/event-wait-helper.disasm.txt) operates
on MMIO events, not owned root backing. These specific functions do not expose
the required postcondition; that does not prove no historical helper elsewhere
could exist.

## Mapping and cache properties

Current pages are pinned shmem:
[exact GEM helper](/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/drivers/gpu/drm/drm_gem.c#L517).
No exported GEM handle permits an unrelated userspace CPU writer. GPU VAs are
mapped by the default SGX MMU context; existing GTT VA is excluded. SCENE_HW
is not the display scanout/stolen-memory BO.
[MMU PTE construction](../phase4-2-data/antix-source/drivers/gpu/drm/gma500/mmu.c#L151)
sets PTE_CACHED for the requested cached type; it is not a CPU cache-coherence
promise. Current initial vmaps use PAGE_KERNEL. A future kmap of the retained
pages would give a normal cached CPU mapping; no UC/WC conversion or DMA-API
ownership handoff is presently made for the scene.

Historical scene allocation requests cached MMU memory. The
[TTM bind policy](/home/gama/sgx535-offline/phase8-root-successor-20261006T015923Z/public-psb/drm_ttm.c#L395)
changes CPU page attributes for uncached placements; the
[PSB backend](../poulsbo-data/PSB_psb_buffer_c.txt#L322) sets SGX PTE cache flags
from placement. These are configuration facts. The
[PSB invalidate callback](../poulsbo-data/PSB_psb_buffer_c.txt#L196) returns0;
it is not evidence that all SGX caches are automatically coherent.

Three layers must remain separate:

* **SGX internal execution/storage:** TA/DPM parameter/control state may have
  stage-local lifetime. Named store/free events and GPU-to-GPU handoff do not
  define external materialization of this particular reservation. PDS/USE/MADD
  invalidate completions do not prove TA root writeback.
* **BIF / memory interface:**
  [MMU flush](../phase4-2-data/antix-source/drivers/gpu/drm/gma500/mmu.c#L74)
  services page-directory/PTE/TLB/data-cache control with writes and readback.
  No root-specific drain/preservation postcondition is supplied. A readback of
  a register orders MMIO processing, not a general guarantee that all root
  writes reached owned pages. The DDK
  [cache-control request flags](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/pinned-ti/sgx_mkif_km.h#L365) similarly require
  firmware processing; they cannot be substituted for the current lifecycle.
* **CPU cache:**
  [drm_clflush_pages](/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/drivers/gpu/drm/drm_cache.c#L60) flushes CPU cache
  lines with full barriers before/after. In this exact x86 source,
  [dma_rmb/dma_wmb](/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/arch/x86/include/asm/barrier.h#L54) are compiler
  barriers; they are not SGX flush requests. Pinned DDK
  [x86 invalidate](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/pinned-ti/osfunc.c#L4301) uses CLFLUSH writeback/invalidation
  too, not invalidate-only. A dirty CPU alias must therefore be excluded before
  post-GPU maintenance, to avoid writing stale CPU data over GPU data.

The pinned SGX535 feature branch does not select the later WRITEBACK_DCU feature.
This is a DDK feature-selection fact, not proof that every TA/DPM cache is
write-through or that all stores are visible at TA_DONE. No system-coherence or
root-cache publication guarantee is inferred from generic x86 behavior or
CPU→GPU flushing. Conversely, no missing flush is diagnosed as the cause of
FIRE #2 zero color.

## Candidate open/close boundaries

| Candidate | Writers/stability status | Lifetime / visibility | Result |
| --- | --- | --- | --- |
| A: accepted TA completion before raster | Later raster not kicked yet at internal point; all TA/DPM root writes externally complete unproved | Host pages held; CPU maintenance possible, GPU publication missing | Earliest candidate, not a safe point |
| B: end-render | DPM cleanup may remain busy | Host pages held; original root survival/visibility unproved | Not safe by evidence |
| C: 3D-memory-free | DPM reclamation happened; end-render can be separately pending | Linux page pinning does not establish original internal representation survival | Not safe by evidence |
| D: retirement | Stronger software completion/release eligibility, no new hardware-memory primitive | Before host release, but root fidelity after reclamation unproved | Not safe by evidence |
| E: additional explicit synchronization/cache operation | Named operations exist, no applicable root-specific commit/stable postcondition established | Cannot select one without new technical evidence; mutation/commands would require separate authority | No mechanism selected |

**Earliest safe point: NONE ESTABLISHED. Latest safe point: NONE ESTABLISHED.**
A host lifetime ceiling exists before remap/reuse/free; hardware representation
may lose fidelity earlier at DPM transitions. The closing edge of a proposed
interval must be the first relevant next GPU write/reclamation, reset/power
change, source/isolation loss, backend end, remap, host clear/reuse or release.
An established opening edge is absent, so this is not an established window.

## Current frozen compatibility and conditional composition

There is a theoretical control-flow checkpoint inside
[fixed_service_status_impl](/home/gama/sgx535-gfx/tools/psb-dri-re/frozen_fixed_service.c#L211):
after successful TA event acceptance and before `session_begin_raster` at226.
The existing [after-status observer](/home/gama/sgx535-gfx/tools/psb-dri-re/frozen_fixed_service.c#L251)
runs **after** that implementation, including raster reset/schedule/kick.
Its callback is not the desired pre-raster capture point. Before-status runs
before acceptance, so it is not a successful accepted-state checkpoint either.

[Retirement](/home/gama/sgx535-gfx/tools/psb-dri-re/frozen_fixed_service.c#L272)
leaves owner pages held; a theoretical copy-before-owner-release point exists
at the component level. The terminal observer captures state, not root bytes;
the immutable capsule contains color/results, not a scene snapshot. Source
lifecycle and capsule attribution can bind future evidence but do not imply
GPU-memory visibility. Their established correctness is not reopened.

No complete publication contract is established. The following is a **conditional
proof template**, not an approved mechanism or a request to implement it:

P1: owner pages/VA and current-operation identity remain held; input CPU writes
are finished; mapping is exact and bounded. **Established component facts.**

P2: relevant admitted-operation events and no-interference evidence bind the
same lifetime. **Reusable qualified semantics; future freshness required.**

P3: at checkpoint E, the TA-produced root backing is fully committed and not
modified/reclaimed through the bounded copy. **Missing rev121 postcondition.**

P4: CPU cache maintenance on those exact clean-owned pages, ordered mapping and
copy complete before the first invalidating transition. **Available CPU
mechanisms; ordering at a future hook would require qualification.**

Therefore a faithful root snapshot would follow **only if P3 and the actual
bounded composition were established**. Locks, fence counters, CLFLUSH and page
pinning cannot hide P3. No lock alone stops an already active GPU/DPM writer.

## Adversarial analysis

| Case | Required conservative result / reason |
| --- | --- |
| TA accepted but raster active | UNKNOWN: current after tap is already after kick; no root writer/visibility promise |
| End-render with cleanup pending | UNKNOWN: deallocation explicitly can remain busy |
| Memory-free before copy | UNKNOWN for original TA representation; page retention is not content retention |
| Allocator reuse | Reject lost ownership; DPM internal reuse separately requires proof even with pinned Linux pages |
| Delayed GPU write | UNKNOWN without all-writers commit/stability guarantee |
| Stale CPU cache | CPU CLFLUSH/order addresses CPU lines only after GPU commit; cannot force missing publication |
| Dirty CPU alias | Reject; writeback/invalidate could overwrite GPU-produced contents |
| Stale mapping / wrong PFNs | Reject; current owned page/VA correspondence must remain valid |
| Competing producer | Reject witness on source/isolation loss; isolation does not stop T's own pending DPM work |
| Reset / power transition | Reject; no reset to manufacture visibility and no assumption old on-chip state survives |
| Evidence loss / copy fault / truncated copy | UNKNOWN; preserve whatever originals exist, no coverage claim |
| Operation HOLD, timeout or partial ledger | UNKNOWN; unsafe-release guard retains pages but does not drain hardware |

These are reasoned failure obligations, not executed GPU tests or synthetic
proof of hardware publication.

## Disposition, next evidence and preservation

**Publication C — UNRESOLVED; format UNRESOLVED/PARTIAL: BOTH INDEPENDENTLY
BLOCKED.** New permitted rev121 evidence must establish the root-backing
postcondition at an actual event/synchronization checkpoint before destructive
reclamation, or identify a required same-path publication primitive. An ordinary
surface-readback fence or generic cache helper is insufficient. No claim of
physical impossibility, a rendering defect or lifecycle destruction is made.

No snapshot hook, decoder, cache command, driver/observer/client/UAPI change,
build, image or new candidate. A further live capture is not justified. FIRE #2
has no root dump and cannot be interpreted retrospectively. FIRE #3 remains
separately **UNAUTHORIZED**.

[Source consistency](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/source-consistency.json):48/48 PASS, not GPU
qualification. The original observer comparison error is preserved: the first
check used an inactive driver staging copy; comparison with the actual separate
observer build resolves it. No implementation mismatch was hidden or fixed.
[Final preservation receipt](/home/gama/sgx535-offline/phase8-gpu-cpu-publication-20261006T025919Z/final-consistency.json) checks all68 sealed
FIRE #2 files, pre-existing repository/input hashes, candidate SHA/Build IDs,
new links and empty index. Only this report, its JSON and the handoff continuation
change repository documentation; audit reference copies/manifests are external.

Unchanged candidate: driver`9d0b5b2fdb7d9881f2828f43fb89253176c38817`,
observer`f11d3abb072caa4e1d32836ef92ce201e9c9d126`,
image`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`.
`sgx_execution_authorized=false`; task SGX/client invocations0; hardware
interactions0; triangle **NOT ESTABLISHED**. FIRE #2 completion and color
provenance findings and all originals are unchanged.
