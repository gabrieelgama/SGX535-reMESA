# Frozen triangle: historical kernel submission and minimum port

Scope: retained source analysis and an offline fixed-contract validation core. No
Mini 12 contact, kernel installation, MMIO, or submission. The candidate PSB
kernel is a compatible historical family, not an authenticated binary pair with
the retained `psb_dri.so`/`Xpsb.so`. The antiX gma500 source is a retained
snapshot; its identity with the currently installed target module is unproved.

## Controlled bring-up publication decision

The operator accepts the remaining CPU-to-SGX payload publication uncertainty
for **one exact frozen-triangle attempt**. This makes publication **PASS FOR
THIS CONTROLLED BRING-UP BY OPERATOR RISK ACCEPTANCE**, not a confirmed SGX535
cache/coherency rule. The historical order remains final CPU writes and
relocations, CPU mapping teardown, PTE/MMU maintenance, BIF maintenance,
selected Xpsb invalidation phases, then TA/raster consumption. Whether that
order publishes every selected payload is architecturally unproved. This
decision changes neither L12/FT-AUX nor bootstrap, USE ownership, service,
completion, recovery or Gate B, and authorizes no target operation by itself.

## Selected historical call graph

1. Retained `psb_dri.so` prepares its BO validation list, relocation BO, TA
   register list, raster register list, scene request and fence storage, then
   issues private DRM index 0 with a 144-byte i386 argument. Retained `Xpsb.so`
   independently issues index 0 from `XpsbFlush3D`. Cross-artifact pairing is
   inferred from index, size and field agreement, not observed execution
   (`docs/phase7/xpsb-re/xhw-bootstrap-and-submission.md`).
2. Candidate `psb_drv.c:85–109` registers index 0 `DRM_PSB_CMDBUF` with
   `psb_cmdbuf_ioctl`, index 1 `DRM_PSB_XHW_INIT` with `psb_xhw_init_ioctl`,
   and index 2 `DRM_PSB_XHW` with `psb_xhw_ioctl`. The 144-byte argument
   includes four userspace pointers, BO handles/offsets/sizes, relocation
   handle/offset/count, engine, flags and optional feedback/video fields
   (`psb_drm.h:221–273`). For the fixed TA path, the validation pointer,
   scene pointer, fence pointer, TA/raster/control handles and offsets,
   relocation count, TA engine, and scene flags matter. Clip rectangles,
   damage, feedback and video fields are unused in the selected no-feedback
   path. Exact paired-build ABI compatibility remains unknown.
3. Candidate `psb_sgx.c:1237–1430` takes the BO read lock and CMDBUF mutex,
   validates the chained BO operations with `psb_validate_buffer_list`, applies
   `drm_psb_reloc` records with `psb_fixup_relocs`, looks up command/TA/OOM BOs,
   allocates or looks up a scene pool, validates its scene/TA memory, queues
   `psb_cmdbuf_ta`, then copies fence/BO status back. The BO chain must contain
   `drm_bo_validate` operations (`psb_sgx.c:410–450`). The relocation record
   is ten 32-bit words/40 bytes (`psb_drm.h:143–165`).
4. `psb_apply_reloc` checks source/destination indices, offset bounds and
   relocation operation, obtains target BO offsets or a USE base reservation,
   then writes the mapped destination (`psb_sgx.c:636–781`). USE relocations
   restrict the data master to pixel or vertex. This is kernel validation of
   user-controlled operands; it is not a complete proof of the selected scene.
   `psb_submit_copy_cmdbuf` copies register pairs after `psb_memcpy_check`
   rejects offsets at/above `0x1000` and the listed MMU/control ranges
   (`psb_sgx.c:55–128,516–575`). The subsequent `psb_reg_submit` writes those
   pairs to SGX MMIO (`psb_sgx.c:487–505`). A new frozen interface should
   accept no user-supplied register list at all.
5. `psb_cmdbuf_ta` copies fixed-size TA/raster command lists into a task,
   holds a scene reference, advances a TA fence sequence, queues TA work and
   returns fence state (`psb_schedule.c:1150–1295`). The scheduler writes TA
   register pairs, then calls `psb_set_scene_fire` →
   `psb_xhw_scene_bind_fire` (`psb_schedule.c:226–317,141–203`). The XHW
   request contains scene cookie, hardware context, scene BO offset, engine,
   flags and optional OOM commands (`psb_xhw.c:146–176`). This is the selected
   transition from validated CPU work to possible SGX execution.
6. XHW is a request bridge to closed X-server code, not just init. `XHW_INIT`
   maps a shared 0x84-byte communication BO and registers one client
   (`psb_xhw.c:416–470`). `psb_xhw_add` queues requests; index 2 dequeues one
   into shared memory; the retained Xpsb thread dispatches scene-info,
   scene-bind/fire, TA-memory-load and other operations; replies/interrupts
   return via `psb_xhw_handler` and scheduler paths (`psb_xhw.c:42–63,
   543–625`; `docs/phase7/xpsb-re/xhw-init-frozen-draw.md`). The historical
   Xpsb module also performs direct SGX MMIO during initialization and fire.
   This bridge cannot be replaced by a single CMDBUF ioctl.
7. SGX IRQ status enters `psb_sgx_interrupt` → `psb_scheduler_handler`.
   TA-finished and pixel-backend/end-render events advance task state;
   deallocation and OOM events are separate. Fence reporting distinguishes
   execution, TA, raster and scene completion (`psb_irq.c:106–123`,
   `psb_schedule.c:403–475,784–885`, `psb_fence.c:31–80`). Acceptance of a
   command is not proof of completed rendering.

### Candidate private ABI field disposition

The candidate header defines private indices `DRM_PSB_CMDBUF=0x00`,
`DRM_PSB_XHW_INIT=0x01`, and `DRM_PSB_XHW=0x02`
(`PSB_psb_drm_h.txt:338–340`); registration additionally requires `DRM_AUTH`
for CMDBUF and `DRM_ROOT_ONLY` for both XHW calls
(`PSB_psb_drv_c.txt:85–109`). These are **candidate historical ABI facts**,
not interfaces in retained current gma500.

| `drm_psb_cmdbuf_arg` field(s) | Frozen-path disposition | Candidate consumer / unresolved detail |
| --- | --- | --- |
| `buffer_list` | Required historically | Linked `drm_bo_validate` operations and ownership; the proposed fixed request replaces this user pointer with kernel-owned descriptors. |
| `scene_arg` | Required historically | Scene size/handle request, then scene pool and XHW bind/fire. |
| `fence_arg`, `fence_flags` | Required for attributable completion | Fence reply and selected user-visible fence types; precise final policy still needs service integration. |
| `ta_flags`, `ta_handle`, `ta_offset`, `ta_size` | Required historically | TA register-pair list and first/last-pass flags. |
| `cmdbuf_handle`, `cmdbuf_offset`, `cmdbuf_size` | Required historically | Raster register-pair list for TA engine; the same fields also serve other engines. |
| `reloc_handle`, `reloc_offset`, `num_relocs` | Required historically | Canonical 49-record relocation wire; fixed port must generate and validate it internally. |
| `oom_handle`, `oom_offset`, `oom_size` | Conditional | OOM path is separate; required exact selected value/payload remains a service integration condition. |
| `engine` | Required historically | Selected TA engine is `PSB_ENGINE_TA=3`; unrelated 2D/video paths excluded. |
| `clip_rects`, `damage` | Legacy unrelated to the selected no-damage path | Other display/2D use; no fixed request field. |
| `feedback_ops`, `feedback_handle`, `feedback_offset`, `feedback_breakpoints`, `feedback_size` | Optional, absent from selected no-feedback path | Feedback setup only when `feedback_ops` is nonzero. |
| `sVideoInfo` under `PSB_DETEAR` | Legacy unrelated | Conditional build-specific video detear payload; contributes to the candidate i386 struct size. |

`drm_psb_reloc` has ten 32-bit fields: `reloc_op`, `where`, `buffer`,
`mask`, `shift`, `pre_add`, `background`, `dst_buffer`, `arg0`, `arg1`
(`PSB_psb_drm_h.txt:142–154`). The candidate kernel interprets the indexed
source and destination through its validated BO list; therefore a raw
userspace-selected relocation cannot be admitted by the new fixed ABI.

The relevant XHW operations have different completion contracts. Scene-info
and TA-memory-info/load set `copy_back=1` and wait for the shared reply, with
bounded one- or three-`DRM_HZ` waits (`psb_xhw.c:79–112,268–335`). Normal
scene-bind/fire sets `copy_back=0` unless the XHW OOM flag is present and is
queued asynchronously (`psb_xhw.c:146–176`). The scheduler treats the later
SGX TA and raster/deallocation events as separate evidence of progress;
`psb_fence_wait` can time out (`psb_fence.c:238–279`). A minimum port therefore
needs distinct **request accepted**, **TA complete**, **raster complete**,
**scene complete**, **fault**, and **timeout/held** states. A successful XHW
queue operation or a signaled execution fence is not by itself a triangle.

## BO, SGX VA and publication ownership

The candidate legacy driver uses DRM TTM buffer objects and memory managers,
not modern GEM. `psb_init_mem_type` assigns GPU offsets to distinct PDS
(`0x20000000`), RASTGEOM (`0x30000000`) and MMU (`0x40000000`) managers
(`psb_buffer.c:73–139`); `drm_psb_tbe_bind` computes
`(mm_node->start << PAGE_SHIFT) + manager->gpu_offset` and inserts the same
pages into the default SGX MMU at that VA. It adds GTT PTEs for the TT type
(`psb_buffer.c:290–372`). Thus a CPU pointer, physical page, GTT address and
SGX VA are different concepts. The selected relocations consume the validated
BO's SGX VA and sometimes a PDS-relative or USE-register encoding. The default
SGX MMU uses page mappings; the retained managers and BO plan require 4 KiB
pages, stronger alignment for some BOs and no 32-bit truncation.

Current retained gma500 has an empty private ioctl table (`psb_drv.c:101–105,
503–516`). Its dumb GEM allocator calls `psb_gtt_alloc_range` at `PAGE_SIZE`
alignment even though its helper accepts an alignment parameter (`gem.c:50–63`).
A CPU mmap fault pins pages into GTT and the default SGX MMU at
`gatt_start + offset` (`gem.c:135–158`, `gtt.c:235–259`). The GTT resource is
not a replacement for the legacy PDS/RASTGEOM manager selection. The actual
target GATT base and installed-module correspondence are unverified. Its unpin
wait covers the display blitter, not a selected SGX TA/raster fence
(`gtt.c:267–300`). Existing SGX MMU helpers are potentially reusable inside
the owning DRM driver, but no live BO provider, USE reservation, scene
lifetime or target GPU VA is established.

Historical `psb_invalidate_caches` returns zero without doing work
(`psb_buffer.c:178–181`). The legacy register submit uses `wmb`; the MMU
implementation flushes page tables/TLB (`psb_mmu.c:73–123`). Those facts
separately establish host order and translation maintenance, not a complete
payload visibility postcondition for every selected BO. Architectural R1
remains unproved; the operator has accepted this uncertainty for the exact
controlled bring-up described above.
The old driver initializes MMU contexts, heap managers, scheduler, XHW bridge,
USE base manager, PDS/3D request bases and IRQ path (`psb_drv.c:545–680`).
Current gma500 initializes display power/GTT/MMU and the base registers
(`psb_drv.c:275–365`), but the selected XHW/scene/TA service ready state is
not present in that source. R2 remains BLOCKED.

The historical watchdog can query XHW lockup and reset/reload SGX state
(`psb_reset.c:253–350`). Its existence does not validate recovery on the
Mini 12 while gma500drmfb drives the active display. Fault reporting names a
requester and GPU VA, not PDS operand sources (`psb_reset.c:66–101`). Recovery
and L12/FT-AUX remain unproved.

The retained gma500 IRQ handler can report a generic SGX MMU fault and clear
SGX event bits (`psb_irq.c:203–250`), but it has no selected TA scene/fence
scheduler equivalent. Reusing its IRQ entry point alone would not provide
completion attribution. The historical reset work waits on XHW lockup,
resets the scheduler, retries TA-memory load, and can reset SGX; this depends
on the absent service and has no validated active-display recovery behavior
on the Mini 12 (`psb_reset.c:253–350`). A timeout must therefore hold all
possibly consumed BOs, as the offline lifecycle core models, until a
separately proven recovery or teardown path exists.

The fixed offline session now requires one nonzero service sequence and a
nonzero timeout interval before entering a call that may fire work. A future
backend must arm the bounded timer and enter this state under its exclusive
service lock **before** the first possibly side-effecting call. It rejects a
second fire. An attributed TA-finished event must precede the raster events;
both pixel-backend end-render and DPM 3D memory-free must be seen before it
marks the scene completed. These are the candidate historical scheduler's
normal-path inputs to the scene-done fence (`psb_schedule.c:421–541,
802–859`). A timeout or attributed SGX MMU fault moves the session to a
held state; neither retry nor retirement is allowed. The future backend must
actually arm a bounded timer and prove event-to-task attribution. Host tests
of this ledger are not IRQ, fence, or live recovery evidence.
Any error after fire entry, including a call that returns failure after it
may have queued work, holds the BOs and USE reservation. Only a proven
pre-fire rejection can release them.

## Historical-to-current gap table

| Required component | Retained current gma500 | Reuse and required change |
| --- | --- | --- |
| CMDBUF/XHW ioctl | Empty private table | New fixed-scene ABI within owning DRM driver; no arbitrary command bytes |
| BO allocation/handles | Dumb GEM, page-aligned GTT range | Potential backing primitive; explicit domain/size/alignment/owner validation needed |
| SGX MMU | Default PD and page insertion | Potential mapping primitive; distinct PDS/RASTGEOM/MMU placement and lifetime needed |
| Relocation/USE bases | No selected service | Fixed 49-site relocation and reserved USE registers required |
| Command validation | No SGX TA command path | Kernel-generated exact TA/raster lists; reject all request payloads |
| XHW/scene/TA | No counterpart | Restore qualified service behavior, including scene and TA tables |
| Publication | MMU flush helpers | Prove final payload and translation visibility before first consumer |
| Submission | No SGX CMDBUF handler | Kernel-owned exact TA/raster transition after all preconditions |
| Completion | Display IRQ/2D handling | SGX task/scene fence with attributable terminal state needed |
| Recovery | Display driver PM path | Reviewed target-specific hang/display recovery remains unproved |

## Minimum port contract and offline implementation limit

The proposed public request is **only** `{abi_version=1, operation=1,
flags=0, reserved=0}` with a fixed 16-byte size. It has no pointer, BO handle,
GPU address, MMIO address, register offset, command-stream byte, relocation or
variable count. The kernel owner must create/retain exactly the ten BO roles
from the frozen plan, validate exact sizes/domains/alignments and unique
ownership, build the known CPU images, resolve the fixed relocations, and
retain everything through scene completion. A future implementation may reuse
GEM pages and SGX MMU insertion but must supply the missing address managers,
service ready-state proof, publication mechanism, interrupt attribution and
recovery. The fixed request is not installed as an ioctl by this pass.

`tools/psb-dri-re/frozen_kernel_contract.[ch]` is an offline, portable C
validation core for the fixed request and internal BO descriptors. It rejects
wrong size/version/operation, unknown flags, absent/duplicate roles, wrong BO
size/domain/alignment, address overflow, overlap and aliasing. Internal BO
descriptors are supplied by a prospective **kernel owner**, never by the
untrusted request. It also compares all 49 ten-word relocation records against
the canonical frozen wire stream, and checks the exact seven TA and 26 raster
register *offsets* in order. The register values still require correct fixed
construction and resolved relocations; offset equality alone cannot authorize
MMIO. Passing this core is necessary but not sufficient for a
real provider. It performs no allocation, mapping, publication, MMIO or
submission and cannot promote FT-BO, FT-SERVICE, R1, R2 or Gate B.

The C core now also applies the fixed 49 records to internal CPU views after
validated BO assignment. It stages all resulting 32-bit words before mutation,
uses explicit little-endian loads/stores, rejects duplicate CPU-view base
pointers, and writes the canonical relocation wire to the fixed control-BO
offset `+264`. Python golden comparisons cover all six user BO byte images at
two distinct synthetic VA layouts. `frozen_scene_owner.[ch]` acquires all ten
objects, reserves all eight GPU VA ranges, validates the complete descriptor
set **before any mapping callback**, then maps. On construction failure it
unwinds in reverse order; tests inject failure at all 26 acquisition,
reservation and mapping steps. It validates all six controlled CPU views and
zeros their full allocations before mapping; the later canonical object
stores remain a separate producer obligation. A failed mapping callback must
remove its own partial PTE insertions through the supplied unmap contract.
A post-submission failure holds the BOs. The owner permits canonical relocation only once in
the `BOS_VALIDATED` phase. These are host-verified lifecycle properties, not
device ownership or publication proof.

`frozen_va_pool.[ch]` provides a bounded offline reservation ledger for PDS,
RASTGEOM and MMU domains, with checked alignment, overflow, overlap, external
claims and release. A mock backend connects this ledger to the scene owner and
excludes a preclaimed GTT range before constructing all ten roles. It has no
internal locking and cannot discover mappings
already installed in the SGX default page directory. A real adapter must
hold a device-wide lock and seed or share the authoritative mapping resource
tree. In particular, current gma500 maps GTT pages into the default SGX PD at
`gatt_start + offset`; its retained initialization may choose `0x40000000` as
the GATT start (`gtt.c:446–469`). The offline ledger must never be treated as
proof that its chosen MMU-domain addresses are free on the Mini 12.

The retained current `psb_gtt_pin` ignores the return from
`psb_mmu_insert_pages` (`gtt.c:235–260`), while that MMU routine can return
`-ENOMEM` after inserting a prefix of PTEs (`mmu.c:696–761`). The ordinary
pin path therefore cannot supply the frozen owner with an all-or-nothing SGX
mapping postcondition as written. A target-qualified adapter needs explicit
success accounting and exact partial rollback, in addition to qualified VA
reservations. `psb_gtt_unpin` waits for the display blitter, not a selected
SGX TA/raster fence, so its current lifetime rule is insufficient too.

The exact public antiX source package for the retained target package version
`5.10.240-antix.1-486-smp-6` was recovered from the [official source
pool](https://antixlinux.com/testing/pool/main/l/linux-5.10.240-antix.1-486-smp/).
Its original tarball and Debian
diff pass the SHA-256 entries in the package `.dsc`; the retained gma500
`gem.c`, `gtt.c`, and `mmu.c` snapshots are byte-identical to that source.
This establishes the offline source tree used here, not that the loaded Mini 12
module was built from precisely those bytes.

`kernel/sgx535_frozen/gma500_bo_owner.[ch]` is a **compile-only, unregistered
in-tree adapter draft**. It uses internal GEM shmem pages, CPU `vmap` for the
six controlled images, a single-device frozen-scene claim, an internal VA
resource tree bounded by `gtt.mmu_gatt_start`, and an explicit exclusion for
the driver's GTT range. It reserves the fixed PDS/RASTGEOM/MMU placements,
then inserts one page at a time into the default SGX page directory so a
failed insertion has an exact mapped-prefix rollback. It retains all ten
objects until an allowed release state. It does not call `psb_gtt_pin`, whose
MMU error is ignored. No ioctl or call site is installed.

The six initial user BO images are generated from the frozen Python scene as
`frozen_kernel_initial.inc`. The C builder zeroes each complete controlled
allocation before its fixed sparse stores; a host test compares every byte
against the Python pre-relocation image. The kernel adapter invokes that
builder. Live USE register ownership/programming, CPU-to-device publication,
scene/TA/XHW service, completion and recovery remain separate. The draft's
private VA tree cannot by itself prove all preexisting live mappings or
target module equivalence. Neither a successful host byte comparison nor an
offline page-table insertion path is live SGX visibility proof.

The historical USE allocator body is available in the retained public Fedora
candidate source at `psb_regman.c` (SHA-256
`5c0dffcb62a8bdc20e117769d3ac6dc66990ebf8754398b4d0cbed4dd74755d3`).
`psb_init_use_base(dev_priv, 3, 13)` creates an ordered free list for register
sequences 3–15. `psb_grab_use_base` asks `drm_regs_alloc` for a register whose
same-data-master 512 KiB window strictly covers the requested address and
size, or takes the first free register; it returns the sequence and
`address - selected_base`. The allocator writes the base/DM register when a
free register is assigned a new window. The TA scheduler fences the reserved
registers; pre-submit failure returns them to the free/LRU lists. This is a
**candidate historical contract**, with exact ELF pairing still inferred.

The fixed C contract now derives an offline plan from the canonical USE
relocations and internally resolved USE BO VA. With an otherwise idle
candidate manager, the selected first pixel request takes register 3 and the
first vertex request takes register 4; both use the aligned 512 KiB base of
the selected USE BO. Those numbers are derived only for this fixed request
order and manager start state, not asserted as existing target hardware state.
The unregistered kernel owner uses that plan to apply all 49 relocations and
reach `HOST_IMAGE_FINAL` **as a CPU-image state only**. It performs no USE
register MMIO write, no fence reservation in a live service, and no payload
publication. Current gma500 has no equivalent register owner, so live USE
ownership and service ordering remain blocked.

For a build check only, these sources were copied into a temporary extracted
antiX source tree, added to its `gma500_gfx` object list, and linked into an
ELF32 i386 `gma500_gfx.ko` with `ARCH=x86 LLVM=1 LLVM_IAS=1`. The module was
not installed or loaded. This verifies kernel API and private-symbol linkage
against that source/configuration, not target runtime conformance. The code is
kept outside the retained source tree until the missing provider, publication,
service and Gate B predicates can be reviewed.

The same core records a strictly ordered *claimed* lifecycle:
`BOS_VALIDATED → HOST_IMAGE_FINAL → CPU_PUBLISHED →
TRANSLATIONS_PUBLISHED → DEVICE_MAINTAINED → SERVICE_READY →
FIRE_POSSIBLE → SCENE_COMPLETED → RETIRED`. It rejects skipped,
repeated and post-retirement transitions. An abort before submission is
terminal; a failure after submission is held and cannot be retired by this
core without a future reviewed recovery mechanism. Each event must be supplied
by a future kernel backend after its independent postcondition is established.
Calling a transition function in an offline test does not establish that
postcondition on SGX535. Distinct owner tokens and nonoverlapping VAs also do
not by themselves prove that physical pages do not alias; the eventual memory
owner must prove that separately.

Port tasks after this offline core, in dependency order: integrate a kernel
owned exact BO manager; install the fixed request only after the owner can
resolve and retain every address; construct a qualified XHW replacement or
restored service; prove publication and bootstrap postconditions; implement
fence completion and target-specific recovery; resolve primary/auxiliary
source containment; then conduct a new exact-action Gate B review. No task
may treat a synthetic VA or an offline state transition as hardware evidence.

### Selected service boundary retained for integration

The selected historical scene path cannot be represented by one fire call.
`psb_alloc_scene` synchronously requests XHW scene-info, which returns a
cookie, hardware-BO size and clear-page metadata before scene-BO creation
(`PSB_psb_scene_c.txt:105–150`). `psb_alloc_ta_mem` synchronously requests
TA-memory-info before creating page-table and parameter BOs
(`PSB_psb_scene_c.txt:430–485`). Validation may call XHW TA-memory-load with
the validated parameter and page-table offsets and updated cookie
(`PSB_psb_scene_c.txt:230–274`). TA scheduling submits register pairs and
queues XHW scene-bind/fire; raster scheduling separately queues XHW
fire-raster (`PSB_psb_schedule_c.txt:226–330`). The ordinary fire request is
asynchronous; IRQ/fence events, rather than an XHW queue return, distinguish
TA and raster progress. The fixed CPU plan contains some selected BO sizes,
but it does not reconstruct all returned cookies, clear-page effects or the
closed Xpsb side effects. The current gma500 snapshot has no consumer for
this protocol. Implementing a nominal queue or synthetic reply would create
a false ready state; FT-SERVICE and R2 remain BLOCKED.

### USE ownership and publication after the fixed relocation plan

The retained `psb_init_use_base(dev_priv, 3, 13)` creates the 3–15 register
free list. `drm_regs_alloc` moves a compatible/free entry to the unfenced
list; `psb_use_reg_set` writes `PSB_CR_USE_CODE_BASE(reg)` with the 512 KiB
base and data master. The selected fixed relocation order gives pixel
register 3, then vertex register 4 **if the manager is idle**. After TA
queueing, `drm_regs_fence` attaches reservations to its fence; the CMDBUF
error path releases unfenced reservations. The compile-only owner now checks
the planned indices/offsets against the exact antiX `psb_reg.h`. This is an
offline plan, not live register ownership or programming. A future service
must serialize programming, retain the registers and USE BO through the
terminal fence, and hold them after timeout until recovery proves safety.

The exact antiX source provides `drm_clflush_pages`; on x86 its CLFLUSH path
orders and flushes every page. An unregistered owner method now checks the
six final CPU images and full SGX mappings for their non-local BOs, rejects
processors without CLFLUSH rather than using
the whole-machine WBINVD fallback, flushes all ten page sets, issues `dma_wmb`,
and removes the private CPU mappings to prevent another patch. This only
establishes a host cache handoff for the constructed BO pages. Service-state
initialization, SGX cache/TLB visibility, powered MMU context, USE programming, and
service submission point remain unresolved. The method deliberately does
not advance the `CPU_PUBLISHED` lifecycle predicate. The architectural R1
proof remains incomplete; the operator accepted that specific uncertainty
for the one controlled bring-up under the decision recorded above.

The owner now refuses scene construction unless the default SGX page
directory reports hardware context 0, the MMU driver has CLFLUSH, and the
SGX register mapping exists.
It now explicitly zeroes every newly acquired page across all ten private
BOs before mapping or relocation, then flushes all ten page sets after final
CPU writes. This closes an initial host-byte determinism gap for the service
objects under clean-room policy. It does not initialize historical XHW
device-side state or prove that the GPU sees those bytes.
In the exact antiX `psb_mmu_insert_pages` path, successful insertion with a
bound context flushes PTEs and invokes the BIF flush. This establishes the
source-level translation handoff condition used by the draft; the check
cannot prove that a suspended or reset target still has that live context.
The retained Poulsbo order is validate/bind → patch mapped BOs → unmap
relocation views → construct/validate scene → submit TA register pairs →
XHW bind/fire. Cached mappings use `PAGE_KERNEL`; uncached/WC mappings use
the TTM mapping protection. `psb_invalidate_caches` is a no-op. Xpsb's
three selected invalidation phases run before kicks, but the retained
status masks still lack a qualified full-payload visibility postcondition.

### rev121 historical continuation predicate (offline model)

The fixed host contract now records an ordered, **claimed** bootstrap path:
selected rev121 initialization writes return, XHW initialization replies,
TA-memory-info and 32×32 scene-info reply, TA-memory-load requests all five
selected flags (`0x1f`), Xpsb observes its retained status2 mask `7` and
`DPM_INITEND` bit `0x00400000`, TA-memory-load replies zero, and scene
validation returns success. In the candidate kernel, `psb_validate_scene_pool`
returns the scene only after the synchronous load reply. The host test rejects
missing, reordered, repeated, and unsuccessful events. This is the
**historical driver continuation predicate**, not an architectural proof of
ready state; status2 mask `7` does not independently observe HOSTD bit `8`.
No current gma500 caller supplies these events, and an offline event cannot
stand in for an actual XHW reply or SGX status observation.
Its rev121 initialization table reproduces the twelve retained `Xpsb0x3820`
CPU register writes in order for raw `0x00010201` and rejects other revision
words. The table is data only: no write is executed, and the unidentified
register effects are not classified as harmless power, clock, or reset state.
The C contract also emits the retained 32×32 scene-info CPU reply: cookie
words 0–14, size `0x1420`, and clear-page range `(0, 1)`. Cookie word 15 is
clean-room zero with explicit historical provenance **UNKNOWN**; the initial
normal bind/fire branch does not consume it. A host regression checks these
exact values. This closes one fixed reply-construction step, not the XHW
responder or its device-side effects.
The same contract assembles the selected TA cookie from the internally owned
TA page-table and parameter BO VAs, using the retained Xpsb info size
`0x620000` and selected load flags. It rejects misaligned or invalid BO
descriptors and checks the packed parameter endpoints. This is CPU-side
cookie construction only; it does not fill/load the hardware TA tables or
establish the XHW reply.
For the selected `0x1f` request, the C contract also emits a 29-action
**offline** TA-load plan: the four retained descriptor/kick groups in order,
the Xpsb status2 mask-`7` wait and clear, then the INIT kick and
`DPM_INITEND` wait and clear. It derives address words from validated
kernel-owned BO descriptors. Poll-clear steps are modeled explicitly; a
future executor must abort and hold on any failed bounded poll. No executor,
ioctl, target register access, or claim of HOSTD bit-`8` completion is added.
The candidate `psb_buffer.c` backend sets `psb_be->offset` to its manager
GPU offset plus the page allocation offset and inserts the same value into
the default SGX MMU. This qualifies the candidate kernel's scene BO offset
as an SGX VA for the selected bind/fire address calculation. A separate
31-action offline fresh-context-0 TA plan now records the normal Xpsb
scene-control writes, clear wait, three maintenance phases, and final TA
kick order. It assumes the retained initial control word `+0x630 = 0`,
setup flag `4`, and no OOM branch; it does not model reuse or raster fire.
The historical helper ignores several wait failures, so the operation list
is **not** a safe live executor. A live backend would have to fail closed on
each bounded wait and preserve scene/USE ownership after the first possible
fire. The Xpsb binary/legacy kernel pairing and target readiness remain
inferential, and no such backend is registered in current gma500.

### Raster service safety boundary

The candidate `psb_schedule.c` selected path has
`ta_complete_action = PSB_RASTER` and a non-null scene. On TA completion,
`psb_ta_done` queues raster; `psb_schedule_raster` writes
`_PSB_CS_RESET_ISP_RESET` (`1 << 5`) to `PSB_CR_SOFT_RESET` (`0x0080`),
updates its scheduler ownership/deadline state, then writes zero to that
reset register **before** submitting raster register pairs and calling XHW
scene bind/fire. The exact antiX `psb_reg.h` retains the
same register and bit names. This is an attributable historical reset
sequence scoped by the register definition to the ISP bit, not a whole-SGX
reset or an optional optimization. The historical path performs no readback
between assertion and deassertion. The compile-only fixed-service backend
contains this exact bracket; no registered caller executes it. No
target-specific proof establishes the active-display impact or bounded
recovery if the sequence is interrupted or the subsequent task hangs.
Publication risk acceptance does not cover this reset. Raster execution and
Gate B remain blocked pending a separate exact-action safety decision and
a callable, attributed completion/recovery path.

The offline C contract now copies the exact seven TA and 26 raster register
pairs into immutable owner storage after relocation and before dropping the
CONTROL CPU mapping. It also builds the selected 132-byte TA and raster
`PSB_XHW_SCENE_BIND_FIRE` CPU request images, with the scene BO SGX VA,
normal fire flag `1`, engines `0`/`1`, and selected scene flags `4`/`15`.
This constructs request bytes only: hardware context `0` is not reserved,
no XHW responder is installed, and no request is queued. The data-only
raster schedule plan records ISP bit-5 assert, deassert, all 26 validated
pairs, then `wmb`, matching `psb_schedule_raster`/`psb_reg_submit`. Invalid
register lists are rejected before a plan is emitted. The ELF32 i386 kernel
draft links against the exact-version antiX source. Its later compile-only
backend can issue only generated fixed actions, but has no registered caller.
The equivalent seven-pair TA list plus `wmb` is also emitted by the fixed C
contract. The XHW wire builder looks up `SCENE_HW` by role, independent of BO
array order; a reordered-BO regression catches an incorrect positional VA.
The one-shot session ledger now requires an attributed TA-finished event
before the sole raster stage can be claimed, and rejects raster completion
events before that claim. Duplicate raster starts, mismatched sequences,
timeouts, and faults retain the post-fire hold state. This is offline
bookkeeping only; it does not supply an IRQ handler or live fence.
The fixed C status decoder maps the retained TA-finished, pixel-backend
end-render, and DPM-3D-memory-free bits to that ledger only under a claimed
exclusive service owner; BIF requester and selected TA/DPM fault/OOM bits
hold the scene. Premature or duplicate attributed events hold it as
ambiguous. Current antiX `psb_irq.c` reads both SGX event-status registers
but enables only 2D completion and BIF requester fault in its postinstall
path. It does not enable or attribute these selected 3D completion events,
so the decoder is not a live completion service.
In the candidate kernel `psb_xhw_add` merely queues work to an external Xpsb
client; `psb_xhw_init_init` enables that queue after mapping its communication
BO. Current gma500 has neither this queue nor an equivalent Xpsb consumer.
The exact initial device-side effects, hardware-context reservation, and
attributable IRQ/fence service remain live integration prerequisites. None of
the new CPU plans makes Gate B pass or changes whitelist `[]`.

### Fixed internal service draft, still offline

`frozen_fixed_service.c` now orders the ten-BO scene through publication,
translation maintenance, the selected rev121 init writes, internal XHW
response stages, the qualified TA load predicate, USE register 3/4 claim and
programming, TA schedule/fire, attributed TA completion, ISP reset bracket,
raster schedule/fire, attributed end-render plus DPM memory-free, and USE
retirement. The selected fresh-context raster XHW branch is a 20-action plan
from retained `Xpsb_scene_switch_fire` → `FUN_00014030` →
`Xpsb_closed_kick_render`, paired with its 132-byte fixed request. The original
Xpsb helpers ignore some wait failures; the draft executor stops at the first
failed bounded poll. No program word is decoded.

`gma500_fixed_backend.c` is a compile-only internal responder in the exact
antiX source tree. It compares every supplied stage payload with the generated
fixed plan before any register action, refuses to wake a powered-off device,
uses the owner's retained power reference and holds BO/USE ownership on ambiguous execution, and
has no ioctl, module registration, or caller. The host one-shot harness injects
failure at each stage and requires HOLD after device maintenance or possible
fire. It also rejects stale completion status before fire, wrong sequences,
duplicate events, faults, and missing completion. These are software
postconditions, not target observations.

Live completion is still blocked: current `psb_irq.c` can clear SGX status on
an unrelated 2D IRQ, enables no selected 3D completion events, and has no
exclusive scene/fence hook. The draft one-shot source requires an attributed
`sample_and_ack`; none is installed. The candidate ISP reset sequence has no
qualified active-display impact or bounded interrupted-reset recovery.
Publication alone was accepted by the operator; these are independent Gate B
conditions. No target action occurred, Gate B remains BLOCKED, whitelist `[]`.

The BO owner now acquires a non-waking `gma_power_begin(false)` reference
*before* SGX MMU insertion can flush BIF and retains it until safe teardown.
The backend shares that owner lifetime. A small exact antiX `psb_irq.c` patch
captures selected SGX status under a lock before the existing IRQ handler
clears it; a fixed status source can also poll/ack while holding that lock.
Its bounded one-shot loop rejects missing, duplicate, wrong-sequence and fault
events, with no retry. `gma500_fixed_entry.c` contains a dormant one-attempt
internal caller with fixed PCI/core identity guards and persistent HOLD state.
The patch and caller compile/link against the recovered exact-version antiX
source, but are neither installed nor registered in the target driver. The
caller now prepares an after-completion 32×32 color BO row summary and hash;
its GPU-to-CPU visibility remains conditional, so a service-completed result
is not automatically a triangle observation. Live
IRQ timing, 3D event availability, target module identity, active-display
impact, and recovery remain unverified; this is not a Gate B PASS.

### Exact-action Gate B review at this offline checkpoint

The proposed action would require a separately installed and registered
exact-version gma500 module, one fixed 16-byte request, PCI `8086:8108`,
CORE_ID `0x01130000`, CORE_REVISION `0x00010201`, ten kernel-owned BOs,
49 internal relocations, fixed USE registers 3/4, the fixed rev121 init/TA
load/TA/raster register plans, at most one fire, a five-second deadline,
selected status capture, and no retry or automatic reset. The caller is not
registered and no such action has been authorized or performed.

BO construction, plan validation, bounded polls, IRQ capture ordering and
HOLD bookkeeping are **offline verified / target conditional**. The operator
accepted only the CPU-to-SGX publication uncertainty for this scene.
Target module compatibility, device-side bootstrap response, live 3D event
attribution, ISP reset impact on the active display, GPU-to-CPU readback, and
post-failure target state remain **unproved**. L12 and FT-AUX source bounds
remain **unproved and unaccepted**. Gate B is **BLOCKED**, whitelist `[]`.
No module was loaded; no Mini 12 contact occurred.

| Exact-action gate item | Classification |
| --- | --- |
| Target identity / rev121 | HW-observed previously; runtime guard compiled, not exercised |
| Ten BOs, SGX VA, 49 relocations, fixed USE 3/4 | Offline verified; target lifetime and exclusivity conditional |
| CPU-to-SGX publication | PASS for this one bring-up by operator risk acceptance only |
| Bootstrap, TA load, scene/XHW fire | Fixed code compiled; live device predicate unobserved |
| ISP bit-5 reset and active display | Historical sequence supported; target impact and interrupted sequence unproved |
| TA/raster completion, color result | Offline failure injection; live event/readback attribution unproved |
| Timeout/fault handling | One-shot HOLD and no automatic retry implemented; target recovery unproved |
| L12 and FT-AUX source containment | Unproved; no operator acceptance |
| Gate B / whitelist | BLOCKED / `[]` |

The retained `Xpsb_scene_switch_fire` normal TA branch calls its clear wait
and `Xpsb_closed_kick_ta`, then returns zero without propagating the clear
wait result. The closed kick helper likewise ignores its three maintenance
wait returns before writing the TA kick. Consequently an XHW queue success or
zero bind/fire return cannot be treated as proof that maintenance succeeded or
that the TA finished. A fixed live service must distinguish those states and
hold every scene/USE resource after an ambiguous post-fire result. The installed
gma500 path has no such XHW responder, TA/raster scheduler, or attributable
completion hook. The new internal draft is unregistered and does not establish
a live ready predicate.

Current gma500 initializes page-directory contexts and PDS/3D request bases,
but it has no installed XHW scene service or selected TA/raster fence. Retained Xpsb
analysis already recovers the 32×32 scene-info cookie words 0–14, size
`0x1420`, and clear-page range; word 15 is unwritten by that routine and is
not consumed by the candidate initial successful bind/fire branch. Those
CPU calculations do not supply the later TA-memory-load effects or a live
XHW responder. The retained B3 publication rule and B4 readiness rule remain
unproved, including the documented load-poll-mask 7 versus distinct HOSTD
bit 8 discrepancy. R2, FT-SERVICE,
completion, and recovery remain blocked or unproved. Gate B is BLOCKED;
whitelist `[]`; no target contact occurred.
