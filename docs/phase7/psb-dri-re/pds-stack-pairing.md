# PDS stack pairing and first-use proof (P7H-029–031)

**Decision: CONDITIONAL for the frozen target.** The candidate allocation
implementation supplies zero-filled PDS backing, and the reconstructed successful
first-scene path leaves the selected range untouched. The missing link is the
identity or an independently established equivalent allocation contract of the
libdrm/kernel actually applicable to the target. A matching version and wire ABI
do not select the page-initialization implementation.

This pass does not revisit PDS decoding. `0x07000345` and `0xaf000000` read
semantics remain UNKNOWN. Recycled slots can retain old bytes. Gate B remains
BLOCKED, whitelist `[]`, hardware functionality UNVERIFIED. No historical ELF
was executed, no device accessed, and no references were acquired or modified.

## Identity and scope

| Retained input | SHA-256 |
| --- | --- |
| `psb_dri.so` in the retained xpsb-glx archive | `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8` |
| `Xpsb.so` in the same archive | `da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f` |

The DRI needs `libdrm.so.2`; it does not pin a build digest. Both retained ELFs
contain `5.0.1.0046`. The candidate PSB 4.41.1 header has the same package label.
The retained package spec is xpsb-glx 0.18-4.1, whereas the contemporaneous RPM
Fusion index lists 0.18-5. Neither co-presence nor the unversioned DDX package
requirements authenticate a loaded stack. See [existing pairing](../xpsb-re/version-pairing.md)
and [source acquisition/hashes](../xpsb-re/source-provenance.md). The quarantined
older Intel Confidential DRI was not used.

Function addresses below are retained DRI **ELF VAs**, not Ghidra's +0x10000
addresses. Decompiler text stays outside the repository. The accumulated
[targets](../../../tools/psb-dri-re/targets.txt) reproduce the bounded exports.

A new discriminator was checked: `0x21c89` calls `drmGetVersion` and saves the
three version integers; `0x4be92` passes them through `0x1eef4` to `0x1ed61`.
The required DRM tuple at ELF VA `0x2602b8` is `(4,0,0)`. The comparison accepts
major 4 and minor at least 0; patch is not used to select an implementation.
The adjacent DRI and DDX requirement tuples are `(4,0,0)` and `(0,1,0)`.
The checked DRM comparison does not select a driver name or require 4.41.1.
Thus the candidate is accepted by this test, but the test cannot authenticate
its zero-fill behavior. The DRI private-record size check is 0x20 bytes.

## ABI fingerprint: what matches and what is not observable

Let **L** be the public `libdrm-poulsbo` 2.3.0 original source, unpacked at
`/tmp/sgx535-libdrm-psb/libdrm-2.3.0`; **K** is public `psb-kmod`
4.41.1-10.fc11.10 source, unpacked at
`/tmp/sgx535-pairing/psb-kmod-4.41.1-10.fc11.10/psb-kernel-source-4.41.1`.

**L/shared-core/drm.h and K/drm.h are byte-identical in their entirety**:
SHA-256 `e4e0c5dce5558d4aa09f33fa236832e354275ccc1612a7c4a1a906758bb51f41`.
This establishes generic source ABI equality, including BO flags, placement
bits, validation structures, fence structures, ioctl numbers and directions.
The disposable L directory was originally acquired with source RPM release11;
its original tarball is byte-identical to the already recovered release10
tarball (SHA-256 `482fa4e4904c7ec5a926627b587288f4c5c16191681e02ad44ee646af0c97c8c`).
Release10's three patches alter configure/Makefiles/header install paths,
not drm.h or xf86drm.c. Its spec supplies additional private PSB headers,
which must not be confused with the common generic header comparison.
The six catalogued K packaging patches alter credentials, interrupt types,
bus naming, AGP compatibility, module naming and I2C. Their AGP changes in
drm_vm.c concern the AGP fault route, not the selected PDS TTM fault. None
alters the checked __GFP_ZERO allocator or PDS memory-type declaration.
This patch audit strengthens the candidate source claim without identifying
which package builds the retained DRI actually loaded.
The [143-row compatibility table](pds-abi-pairing.csv) includes 115 i386
structure-size/field-offset/field-width measurements, 16 ioctl words and 12
binary/path observations. Unobserved individual binary fields are marked
UNKNOWN; header equality is not presented as recovery of the loaded library.

| Retained caller | API/path | Candidate implementation and contract |
| --- | --- | --- |
| `0x4297b` (also generic `0x436cc`) | `drmBOCreate` | L `xf86drm.c:2596–2625`; IOWR nr 0xcd, 112-byte union; K `drm_bo_create_ioctl` |
| `0x4297b`, `0x4383e` | `drmBOMap` | L lines 2674–2718; shared mmap plus IOWR nr 0xcf, 112 bytes; K map handler and TTM fault |
| `0x4297b`, `0x437fb` | `drmBOUnmap` | L lines 2723–2734; IOWR nr 0xd0, 4-byte handle; mapping retained by L |
| `0x43613` / `0x42d82` and generic release | BO reference/unreference | IOWR nr 0xd1/0xd2, 112/4 bytes; host object lifetime differs from payload lifetime |
| `0x4345a` / `0x43678` | set status / wait idle | L lines 2737–2803; nr 0xd3/0xd5, 112-byte unions; flags/mask/hint transfer |
| `0x37b51` | validation list | 136-byte `drm_bo_op_arg`; node+0x0c argument, node+0x8c handled = argument+0x80; K `psb_sgx.c` validation loop |
| `0x44097`, `0x452c7`, `0x44107` | fence wait/status/unreference | L fence functions lines 2413–2574; 48-byte wire argument; K `drm_fence.c` handlers |
| `0x442bd` | returned host fence | 40-byte host `drmFence`, ten-dword copy; distinct from 48-byte ioctl argument |
| `0x37b51` | `drmCommandWrite` | command index 0, size 0x90; L constructs ioctl from supplied index/size; K CMDBUF structure 144 bytes |
| `0x374f5` | `drmCommandWriteRead` | scene index 3, size 0x14; candidate scene structure 20 bytes |
| `0x37854` / `0x38762` | relocations | 40-byte records; K masked destination-word relocation processing |

L's i386 host `drmBO` is 112 bytes: map handle at +4, flags +12, mask +20,
size +32, offset +36, fence flags +48, virtual/mapVirtual +68/+72, mapCount
+76. Retained pool host storage is 200 bytes, with its embedded BO at +0x4c
and separate map-result pointer at +0xbc. The read at pool+0x7c is BO
**fence flags**, not its GPU offset. The observed host footprints agree;
this is compatible evidence, not a claim all host members were recovered.

K `drm_drv.c:142–159` dispatches the generic BO/fence requests to the matching
handlers. Example i386 request words are create `0xc07064cd`, map
`0xc07064cf`, unmap `0xc00464d0`, fence wait `0xc03064ca`. The retained
DRI imports wrappers rather than embedding these particular ioctl words.
L translates returned wire BO fields into the host object; its error/retry
handling is not a GPU payload copy or initialization operation.

## Exact PDS creation and zero-page route

```text
new context 0x4bd92 -> 0x371f0 -> 0x48592
  context+0x804 = new pool 0x4297b
  drmBOCreate(fd, 30*slot_size, 0, NULL, 0x0000000020000001, 4, &bo)
    L: drm_bo_create_req {mask, size, buffer_start=0, hint=4, page_alignment=0}
    ioctl BO_CREATE -> K: drm_bo_create_ioctl -> drm_buffer_object_create
    type_dc; PDS placement bit29 -> memory type5; non-fixed TTM
    drm_bo_add_ttm -> drm_ttm_init -> drm_bind_ttm -> drm_ttm_populate
    each absent page -> drm_ttm_get_page -> drm_ttm_alloc_page
      alloc_page(GFP_KERNEL | __GFP_ZERO | GFP_DMA32)
  drmBOMap(fd, &bo, 3, 0, &map) -> shared mmap -> same TTM pages
  owner+0x6c -> fixed slot -> monotonic aligned reservations/commits
```

`slot_size` is 0x20000 or 0x40000 according to the retained HWTNL option;
the backing BO size is therefore 0x3c0000 or 0x780000. Userspace's 0x1000
outbuf alignment is separate from the **zero** kernel page-alignment request.
Hint 4 is DONT_FENCE at creation, not a request for preexisting contents.
Mask `0x20000001` is READ plus MEM_PRIV2; K `psb_drm.h:49–50` aliases
MEM_PRIV2 to PDS, memory type 5. There is no libdrm mask translation. READ
here describes GPU permission; it does not forbid the CPU read/write mapping.

K `drm_bo.c:1833–1841` selects type_dc for null user backing; the creation
routine creates a new BO, not a reference to an old userspace handle.
`drm_bo.c:136–149,196–213` creates/binds the TTM on the non-fixed path.
K `psb_buffer.c:116–124` declares PDS TTM, mappable and CMA, without FIXED.
It is not the VRAM/stolen-memory branch. A zeroed page-pointer array in the
new TTM is metadata outside the payload. `drm_ttm.c:217–235,271–289,408`
populates every absent page; lines 81–92 allocate each using __GFP_ZERO.
Existing populated pages are returned unchanged. Null user backing excludes
the separate get_user_pages/user-buffer path. An allocation failure does not
produce a successful selected program; retry/reset histories are excluded.

## Same pages at CPU mapping, validation and GPU address

L `drmBOMap` uses MAP_SHARED with PROT_READ|PROT_WRITE. Its unmap method
issues the BO unmap ioctl and updates mapCount, retaining the mmap; it does
not copy payload to another BO. K `drm_vm.c:774–788` obtains the TTM page
and inserts its PFN into the CPU mapping. PDS CMA handling in
`drm_bo.c:2508–2525` excludes the direct PCI/fixed-memory mapping alternative.

K backend population `psb_buffer.c:282–289` retains the same page array.
Binding at lines 324–364 passes that array to `psb_mmu_insert_pages`;
`psb_mmu.c:843–900` obtains each page's PFN. Address derives from the memory
node and manager GPU offset. This is a page-table mapping, not a second
payload allocation. Allocation metadata and PTEs do not occupy PDS data bytes.

For non-fixed moves, K `drm_bo.c:224–228` selects `drm_bo_move_ttm`;
`drm_bo_move.c:50–84` unbinds/rebinds the same TTM. PDS/local eviction and
revalidation preserve its payload. The selected mask does not request fixed
VRAM placement. Fixed-memory move/copy paths therefore do not defeat this
specific source-level witness. The source has no bounce copy replacing the
selected PDS contents on this route.

The PDS mask omits CACHED. K `drm_ttm.c:105–129` flushes when changing
caching policy, and binding makes the relevant TTM pages uncached;
`drm_vm.c:48–78` supplies the x86 PCD or configured PAT mapping policy.
GPU binding passes uncached type zero for this flag combination. **Do not
cite `psb_invalidate_caches` as a payload flush: it is a no-op in K.**
These facts establish the candidate's same-backing/visibility software
contract, not a measured coherency result on the Dell. There is no submitted
work in this investigation. Relocations target selected +0x00, not its holes.

## Freshness and first-occupant proof

The scope is a newly calloc-allocated context, its first successful scene,
the selected zero-input fragment cache miss, no previous draw/clear/feedback,
no depth target, no allocator/scene retry, and the already traced private
render path. The later [P7G frozen closure](frozen-draw-closure.md) already
closed the historical frontend edge; that result is preserved. This pass
does not claim that all selected shader/state validation has been resolved.

Freshness is five separate events: a new context requests a new pool; that
pool calls BOCreate rather than BOReference; K makes a new BO/TTM; population
allocates zeroed pages; a slot is taken from that **new** pool. A fresh slot
in a reused BO would not prove this. Mesa share-context state does not import
context+0x804: `0x371f0` calloc and `0x48592`'s zero-pointer test lead to the
new pool call. `0x27a1e` attaches owner+0x6c to it. Host constructors do not
write GPU payload. At `0x2794b`, first-scene reset order is owner+0x64,
+0x6c, +0x68, +0x70, +0x78; these use distinct pool pointers.
The free-list constructor inserts descriptors for slots 0 through 29 at the
head; `0x42f93` removes the tail. The first pool allocation is slot 0. The
proof only needs an untouched slot, not that numeric slot identity.

[The timeline](pds-first-use-timeline.csv) enumerates **actual commits**, not
reservation upper bounds. Each row cites an emitter; addresses are relative
to the mapped slot. The model tests both slot sizes and both color-state
branches. `0x392ab` first emits 16 bytes of event data, then a 0x94-byte PDS
record aligned to 32; its code does not begin at slot zero. If color+0x24 is
1, `0x39ac4` emits a 56-byte clear primary **and a separately aligned
four-byte secondary via 0x3faa3**. Both branches then emit texture primary,
texture secondary, preterminate state data, and state PDS.

For the preterminate call `0x2a57b -> 0x287ff`, presence=0x2000 emits one
mask and one value: eight bytes. `0x381a0` sees two dwords, returns one
transfer descriptor, and `0x287ff` commits 0x3c bytes for its PDS. No PDS
instruction interpretation is involved in deriving that length.

| Slot-relative event | Color+0x24 != 1 | Color+0x24 == 1 |
| --- | --- | --- |
| end of event PDS | 0xb4 | 0xb4 |
| clear primary / secondary | absent | [0xc0,0xf8) / [0x100,0x104) |
| texture primary / secondary | [0xc0,0x100) / [0x100,0x104) | [0x120,0x160) / [0x160,0x164) |
| preterminate data / PDS | [0x104,0x10c) / [0x120,0x15c) | [0x164,0x16c) / [0x180,0x1bc) |
| **selected primary** | **[0x160,0x198)** | **[0x1c0,0x1f8)** |
| selected secondary after primary | [0x1a0,0x1a4) | [0x200,0x204) |

The largest reservation end through primary is 0x350, far below the smaller
0x20000 slot. There is no wrap or rotation. Reservation upper bounds may
cover bytes used by later allocations; they do not write those bytes.
The inspected predecessor stores/relocation destinations stay inside their
committed ranges. Commit `0x384ab` advances to the actual end, not the
reservation end. `0x38762` aligns that cursor. No selected hole falls in an
earlier payload range.

Additional first-use checks:

- The first `0x26dbd` state dispatch runs with scene NULL; primary callback
  `0x2d015` does not emit. After scene creation it dirties 0x20000 and
  dispatches primary (atom order 13) before secondary (15). Earlier atom
  masks in [state-atoms.csv](state-atoms.csv) do not intersect that bit.
- `0x26678` calloc leaves renderer+0x4c NULL. The optional earlier vertex-PDS
  callback in `0x26dbd` therefore does not run. `0x26b5c` requires a live
  scene and clean state before actual vertex allocation.
- Normal vertex payloads use the distinct context+0x810 pool, established
  by `0x3f87a`, not +0x804. This does not undo the documented later
  scene/index-copy users of +0x804; they are simply not prior occupants here.
- `0x253f1` calloc leaves feedback+8 zero; attachment `0x253cd` does not run
  its conditional allocation callback on this first-use path.
- Pretermination installs its reservation callback on owner+0x78, not +0x6c.
  USE programs and background-object output use separate owner outbufs.
- No user GL clear is assumed, but both **internal** color+0x24 branches are
  modeled. An internal clear must not be excluded just by saying “first draw.”

## Per-hole decision and adversarial checks

For each of +0x08, +0x0c, +0x10, +0x14, +0x18, +0x1c, +0x24, +0x28,
+0x2c: no predecessor store overlaps it in either timeline; the selected
builder and selected relocations leave it untouched. Under **K/L's recovered
contract**, each is ZERO_FROM_FRESH_BACKING_CONFIRMED. Applied to the actual
frozen target without an authenticated/equivalent backing provider, each
remains UNKNOWN/conditional zero. See the individually updated
[byte table](pds-buffer-byte-provenance.csv). Steady-state remains stale-capable.

The following attempted counterexamples were checked:

| Challenge | Result |
| --- | --- |
| Earlier L private psb_drm.h disagrees with K | Real mismatch retained. Generic drm.h is identical. drmCommandWrite/Read uses the DRI's supplied size/index, not L's private C structure. Does not invalidate this BO path; does prevent a blanket all-private-ABI claim. |
| Host fence 40 versus ioctl fence 48 | Distinct objects, explicitly copied/serialized; not a layout mismatch. |
| PDS mask means fixed/stolen memory | Rejected for K: type5 is non-fixed TTM/CMA. |
| Null user backing can import dirty user pages | Rejected for K's selected type_dc route; user-page import is separate. |
| First draw means first PDS allocation | Rejected: six or eight prior payload allocations; nested clear secondary matters. |
| Any reservation overlap writes a hole | Rejected: reservations do not initialize bytes; bounded stores/commits are what matter. |
| Fresh context can reuse its old +0x804 pool | Excluded by zeroed new context and new BOCreate; reset of an existing context is outside scope. |
| Validation migrates into unrelated payload | No such replacement on selected non-fixed PDS/local route; same TTM preserved. |
| Exact build follows from DRM version check | Rejected: retained comparison admits major4/minor>=0. |
| Current gma500 implements these old BO ioctls | NOT ESTABLISHED; the catalogued modern gma500 source is a different display stack, not an identified running kernel. No running-device query performed. |

No concrete second ABI-compatible kernel with contrary page initialization
was established in this pass. It is unnecessary to invent one: the zero-fill
fact is implementation behavior not encoded by the observed request/version.
The existing metadata does not identify the supplying implementation.

## Decision gate and smallest remaining fact

| Question | Classification |
| --- | --- |
| L ↔ K generic BO/fence source ABI | **ABI-EQUIVALENT**, entire common header identical |
| Retained DRI ↔ L/K used allocation interface | **INFERRED compatible** with matching observed host layouts, call arguments, validation and private sizes; loaded builds absent |
| Exact target userspace/kernel pairing | **NOT ESTABLISHED** |
| Fresh PDS zero-image under K/L implementation | **CONFIRMED source behavior; CONDITIONAL application to target** |
| First-occupant/no-overlap on reconstructed successful first-scene path | **CONFIRMED**, both internal-clear branches; not all GL histories |
| Frozen target deterministic fresh construction without PDS decode | **CONDITIONAL** |

The remaining fact is specific: **establish that the target backing provider
for BOCreate(mask=0x20000001, buffer_start=0) is K's recovered fresh-zero,
same-TTM-page implementation, or establish that contract independently for
the actual provider.** The retained library import and accepted DRM version
cannot establish this. An authenticated built libdrm/kernel pair mapped to
these sources, or a reviewed equivalent target allocator contract, would
settle it. The exact historical package release is not inherently required
if semantic equivalence of the relevant allocation/visibility path is proven.

If that condition is supplied, no further first-occupant mystery remains for
this restricted private path: reproduce the first-use zero holes and selected
five produced/relocated dwords. That removes *dependence on decoding these
holes*, without determining either instruction's read set or proving a
working triangle. The broader historical byte image, arbitrary implicit
state outside this region, and recycled operation are not thereby resolved.
FG-02 remains OPEN for the unqualified target; its existing suffix/secondary
results stand. There is no justified unconditional blocker closure in this
pass, and therefore no claim of having crossed into a proven final pixel-state
or submission specification.

## Reproduction

```sh
python3 tools/psb-dri-re/pds_first_use_model.py
python3 tools/psb-dri-re/pds_abi_layout_check.py \
  /tmp/sgx535-libdrm-psb/libdrm-2.3.0/shared-core/drm.h \
  /tmp/sgx535-pairing/psb-kmod-4.41.1-10.fc11.10/psb-kernel-source-4.41.1/drm.h
```

The [first-use model](../../../tools/psb-dri-re/pds_first_use_model.py) checks
alignment, commits, both capacities, hole non-overlap and rejection of an
undersized slot; `--write` regenerates the timeline. It models authorially
recovered arithmetic, not machine-code execution. The
[ABI check](../../../tools/psb-dri-re/pds_abi_layout_check.py) requires clang,
checks both header identities, compiles a freestanding i386 constants object,
and reads its `.rodata` without execution. It compares all 115 layout metrics
against the table. Neither program accesses hardware or decodes PDS.

## Validation of this pass

Reverified both retained ELF hashes, five previously recorded implementation
file hashes, both common-header hashes, and the two source RPM / two original
archive hashes (13 checks). All Phase7/evidence CSV widths parsed; the new
ABI/timeline schemas have unique headers and no empty cells. Evidence IDs
P7H-029–031 each have one defining matrix row. Local Markdown file targets
were checked, analysis-script syntax compiled, the first-use model passed
both branches/capacities and its insufficient-capacity rejection, and the
ABI check passed 115 metrics plus rejection of a deliberately wrong fence
size in a temporary table. Bounded Ghidra export completed312/missing0/failed0.
`git diff --check` passed; the index is empty and references/ status unchanged.
These checks validate artifacts and provenance, not GPU behavior.

## Backing-provider continuation (P7H-032–035)

The [provider audit](pds-backing-provider.md) now establishes exact OBS source
lineage and bounded backing equivalence across additional public PSB branches.
It excludes stolen/TT fallback for the selected mask. The remaining condition
is target membership in that audited class; no target provider is selected by
the unversioned package dependencies. P7H-030 no-overlap remains CONFIRMED.
