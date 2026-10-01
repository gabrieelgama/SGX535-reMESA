# Frozen fragment primary PDS backing-memory lifecycle

This is a static trace of the selected 56-byte primary PDS allocation. It
distinguishes the old driver's bytes, possible GPU reads, and a clean-room
driver's physical ability to write the allocated range. The retained DRI ELF
is SHA-256 `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`.
No historical binary or GPU command was executed. Function addresses below
are ELF virtual addresses. Reproduce the bounded decompilations using
[targets.txt](../../../tools/psb-dri-re/targets.txt) and the repository's
[headless instructions](../../../tools/psb-dri-re/README.md); proprietary
decompiler text remains under `/tmp/sgx535-psb-decompile/`.

## Owner, allocation and address

The selected cache miss in `0x283d8` calls `0x25970` with the owner's
fragment/PDS outbuf at `owner+0x6c`. The owner is created by `0x27a1e`,
called during context setup `0x48592`. `0x27a1e` constructs five distinct
outbufs; `owner+0x6c` uses the pool pointer at context `+0x804`, capacity
`0x20000` or `0x40000` per slot, 0x1000-byte backing alignment and flags
`0x20000001`. `0x38a9b` allocates the 0x54-byte **host** outbuf with
`_mesa_calloc`; `0x43d8c` allocates its 0x3c-byte **host** BO wrapper with
`calloc`. Neither call clears the later GPU data region.

`0x48592` creates the pool through `0x4297b`. That routine creates one
kernel BO of `30 × slot_size` bytes with `drmBOCreate`, maps it with
`drmBOMap`, and allocates a separate host array of 30 slot descriptors.
`0x42f93`, the pool allocation method used through `driBOData` `0x44c90`,
removes a slot descriptor from the free list. `0x430c3`, its map method,
returns the mapped kernel-BO base **plus the slot's fixed offset**. Pool
list links, slot offsets and fence handles live in the host descriptors,
not within the GPU data bytes. The exact backing BO placement and slot index
are dynamic.

`0x38762` reserves 400 bytes for `0x25970` at a 32-byte aligned outbuf
cursor and returns that CPU pointer. The program starts at a *suballocation*
offset `S = align32(cursor - mapped_slot_base)`, not necessarily at the start
of the slot or kernel BO. It returns a descriptor containing BO wrapper,
offset and initially zero length. On success, `0x384ab` records
`length = end_pointer - program_start = 56` in descriptor `+0x08`, asserts
it does not exceed the 400-byte reservation, and advances the cursor to
`program_start+56`. It does not copy, clear or allocate 56 new bytes.
Another outbuf reservation can then follow at its own alignment. See the
[lifecycle edges](pds-allocation-callgraph.csv).

The selected scene-creation path `0x27060` calls event builder `0x392ab`
with this same `owner+0x6c` outbuf **before** the draw's primary program.
The event builder requests 400 bytes and, on its successful PDS branch,
commits 0x94 bytes. If it began a new slot at offset zero and no intervening
rotation occurred, the selected primary cannot start before aligned offset
`0xa0`; additional clear/texture/state allocations can move it farther.
If the outbuf rotates to a new slot before the selected allocation, `S` can
again be zero. Hence no single absolute suballocation offset follows from
the frozen shader alone.

## Initialization, reuse, and synchronization

`0x3865f` resets an outbuf by calling `driBOData` `0x44c90` with a null
source pointer and then `driBOMap` `0x43ffe`. `driBOData` can retain a
sufficiently large existing slot; its content copy runs only for a non-null
source. Neither reset nor map clears the data region. On outbuf rotation,
`0x38762` unmaps via `0x385ed`, releases the old BO wrapper via `0x43c89`,
installs a new wrapper and resets its cursor. At zero wrapper references,
the batch-pool release method `0x42e37` returns the slot to the free list
immediately when its fence field is empty, or to a pending list when fenced.
`0x42c98` waits/polls the pending fence and returns the same slot to the
free list. Those methods update **host** links/fence fields; none clears or
copies the slot's mapped data bytes. Fences constrain reuse time, not byte
values. `0x27735` eventually destroys the owner and unreferences the outbufs.

Rotation also increments the outbuf's host counter at `+0x50`; it does not
stamp a generation into the GPU range. An optional reservation callback at
outbuf `+0x48` can run before rotation (`0x3838b` installs it and
`0x382ce` removes it). It is not a universal zeroing contract. The
byte-preservation findings describe these CPU allocator operations, not a
proof that arbitrary earlier GPU work could never have written the BO.

There are therefore two distinct inherited-content cases:

1. While the current mapped slot has capacity, `0x38762` advances the cursor
   without resetting or clearing it. In a separate reset path, `driBOData`
   can retain a sufficiently large existing slot and `0x3865f` resets the
   cursor to its base without clearing it. Either path can expose old bytes
   at the selected `S` until the selected builder overwrites them.
2. The pool returns a previously used fixed slot. Its mapped bytes survive
   release and fence reclamation. A new outbuf cursor starts at that slot's
   base and can later reach the same `S`, now overlaying old PDS or other
   owner-`+0x6c` data. The 30-slot pool is shared by the relevant outbuf
   users; a slot's immediately preceding writer is not fixed by the shader.

Fresh backing pages are a separate case. The deeper candidate-kernel trace
below establishes zero-filled newly allocated TTM pages **in that source
release**, conditional on the creation path. It does not authenticate the
kernel paired with the retained ELF or zero a recycled userspace slot.
A first scene also emits the event program
before the selected primary, so “first draw” is not synonymous with “BO
offset zero.” [Possible predecessor families](pds-reuse-predecessors.csv)
include event, clear, texture-replace, primary/secondary fragment and vertex
PDS producers. Their relative stores are known in several cases; the old
slot index, old program starts, old scene branches and current `S` are not
fixed, so none supplies an exact inherited value for a selected hole.

The pool is not restricted to PDS programs. `0x29fbd` passes the same
`owner+0x6c` to `0x3f94c`, which copies 128 bytes of scene vertex data,
and to `0x37409`, which copies six u16 indices (12 bytes). The indexed
draw path `0x27fa0` also reserves at least 0x2000 u16 index entries through
`0x37409` with a null initial source; subsequent index conversion fills the
used part. Thus a previous slot may contain ordinary vertex/index data and
unfilled reserved index capacity, as well as PDS data/code. This is a
concrete shared-allocation route, not proof that every possible 32-bit
value occurs at every selected hole. If an old write occupies slot-relative
range `[P,P+L)`, it supplies current hole `h` only when `S+h` lies in that
range and no intervening writer replaces it. The copied source byte is at
offset `S+h-P`. Neither `P` nor the previous source is fixed by this draw.

On a primary cache hit, `0x283d8` skips `0x25970` and copies the existing
six-word program descriptor into the scene. It does not refresh program
data or fill holes. This preserves the earlier allocation's provenance.

## Commit length and submission boundary

The 56-byte commit is a userspace **descriptor length and cursor advance**.
The owner/BO descriptor retains a suballocation offset for relocation and
validation. The selected builder also records data-prefix extent 12 dwords
at its program descriptor `+0x0c`; the two instruction literals occupy
`+0x30/+0x34`. The existing `Xpsb_pds_get_num_constants` result supports
the interleaved DS0/DS1 CPU layout, but no independent hardware count rule
proves any of the nine intervening data dwords unaddressable. The 56-byte
descriptor length is **not** established as a GPU memory-access bound.

Relocation helper `0x3a82c` emits three masked records for `+0x00`; the
selected emitter does not create a relocation targeting any hole. In the
candidate PSB kernel `psb_sgx.c:631–778`, relocation application computes a
destination from BO plus suballocation offset and writes the designated
dword with the mask/background rule. This is candidate-family evidence, not
proof of exact built pairing. It explains why the final `+0x00` word is
address-dependent and why the nine holes receive no **selected relocation**
write. Validation and a fence retain the BO for submission; no inspected
userspace or candidate relocation step normalizes those holes. The GPU PDS
program/data extents and possible reads remain architecturally UNKNOWN.

### Forward descriptor and submission trace

`0x283d8` copies the primary descriptor to scene `+0x430…+0x444`:
BO wrapper, suballocation offset, committed byte length, data-prefix dword
count, and two further state fields. `0x39ea0` places the secondary
descriptor at scene `+0x448`, leaving the primary descriptor intact.
`0x29251` passes scene `+0x414` through `0x28fff` to state builder
`0x287ff`. In the `present & 0x40` branch, the primary fields therefore
appear at state `+0x1c/+0x20/+0x24/+0x28`.

That branch relocates the primary base using state `+0x1c/+0x20`, with
address shift 4 and mask `0x00ffffff`, and uses
`(data_dwords & 0xfc) << 24` as the background. For selected count 12,
the latter expression is `0x0c000000`. **It does not use the primary
committed length at state `+0x24` in this branch.** There is no per-hole
live mask or separate DS0/DS1 initialized-slot count in this recovered
packing operation. This proves CPU field propagation, not the hardware
meaning or permissible access range of those fields.

The state builder allocates separate output in the same outbuf; these
relocations write the new state record, not the primary PDS data prefix.
`0x28fff` then calls `0x3b73b` for the TA state reference. Finalization
`0x2a39a` calls `0x27890`, which unmaps five owner outbufs including
`+0x6c`; `0x276a0` is a no-op. Outbuf unmap `0x385ed` calls
`0x45124` and clears host cursor/map fields. The pool unmap method
`0x42940` clears only slot-descriptor `+0x18`. It does not copy,
truncate, scrub or zero the underlying bytes.

Submit wrapper `0x37b51` builds the validation list and 0x90-byte command
argument. Its command-stream lengths belong to the passed command-buffer
descriptors; it does not traverse the selected primary allocation to fill
holes. After successful submission, `0x442bd` creates a fence object and
associates it with each user-list BO via `0x4423a`; the pool method
`0x42f2d` stores a referenced fence in host slot-descriptor `+0x0c`.
Map method `0x430c3` checks that fence and calls wait/unreference helper
`0x42ed3` (or returns busy for its nonblocking flag) before exposing the
same mapped bytes. Pool destruction `0x42d82` drains/releases its objects,
unreferences the backing kernel BO and frees host metadata. None of these
inspected paths supplies a hole value. The exact paired libdrm/kernel
mapping/coherency implementation is not contained in this ELF and is not
silently assumed from the candidate kernel source.

## Exact 56-byte image: what can and cannot be derived

The [per-dword provenance table](pds-buffer-byte-provenance.csv) records all
14 committed dwords individually. `+0x00` is the linked-program relocation
word (CPU placeholder or presumed-address encoding, then applicable masked
relocation); `+0x04=0`, `+0x20=0x20`, `+0x30=0x07000345`, and
`+0x34=0xaf000000` are selected CPU facts. Each of the nine remaining
offsets has **no selected producer write or selected relocation**. At GPU
consumption it can carry a prior slot value or an unestablished first-backing
value. Thus the complete historical 56-byte image cannot be reconstructed
from the frozen draw input alone. “Producer-unwritten” is not “zero.”

All 14 dwords lie within the returned 400-byte CPU-writable reservation and
the 56-byte committed range. The allocator's host metadata is separate, and
other PDS builders write some of the same relative data offsets. A clean-room
allocator can therefore make the **CPU-visible byte image deterministic at
the memory-allocation level**: initialize the 12-dword data prefix in the
returned mapped range, write the two selected instruction words, then apply
the proven selected fields and relocation. This is a possible new-driver
construction, **not** a fact about the historical driver. It need not assume
fresh-page zeroing or the slot's previous contents.

## Conditional fresh-backing reconstruction (P7H-028)

The existing public source packages give a concrete first-use model:

The repository's frozen input already specifies a new context and one
scene with no prior GL draw or clear. The retained context entry
`0x4bd92` selects `0x371f0` for device IDs 0x8108/0x8109;
`0x371f0` allocates a zeroed 0x141c8-byte host context before calling
`0x48592`. Thus this route requests a new per-context `+0x804` pool
backing BO rather than importing a previous context's pool. Scene creation
`0x27060` invokes owner-map/reset helper `0x2794b` before event emission.
The successful first-scene path is the relevant candidate for a fresh
image; scene retry/reset and recycled-slot histories must not be silently
included in that claim.

1. Retained `0x4297b` passes a null user-buffer argument to `drmBOCreate`.
   The candidate libdrm `xf86drm.c:2596–2625` transfers it into
   `req->buffer_start`; its two `memset` calls clear host argument/BO
   structures, not GPU bytes. `drmBOMap` at lines 2674–2718 uses a shared
   read/write mapping.
2. Candidate kernel `drm_bo.c:1833–1841` selects `drm_bo_type_dc` for
   zero `buffer_start`. Its TTM creation branch at lines 136–149 uses
   `drm_ttm_init`. The retained PDS-pool mask `0x20000001` selects the
   candidate header's `DRM_BO_FLAG_MEM_PRIV2`, aliased to
   `DRM_PSB_FLAG_MEM_PDS` by `psb_drm.h:49–50`.
3. `psb_buffer.c:116–124` makes PDS memory mappable, TTM-backed and
   **non-fixed**. The non-fixed move path in `drm_bo.c:196–213` adds and
   binds a TTM; this is not the stolen-VRAM fixed-memory branch.
4. `drm_ttm.c:81–92` allocates each new page with `__GFP_ZERO`.
   `drm_ttm_get_page` at lines 217–235 only allocates when a page pointer
   is absent; `drm_ttm_populate` at lines 271–289 fills the page array,
   and `drm_bind_ttm` calls it at line 408. The page-fault path also uses
   `drm_ttm_get_page` (`drm_vm.c:774`). Existing pages are returned as-is.

Consequently, **under that candidate libdrm/kernel contract**, a genuinely
new backing BO begins with zero data bytes, including all nine holes in
an untouched future suballocation. Earlier allocations at disjoint cursor
ranges do not change this; slot recycling or cursor reset onto a previously
written range does. This narrows the first-use uncertainty to the actual
backing-creation contract and allocation history, rather than a missing
page-initialization mechanism. It does not prove zeros on every draw.

For a deliberately restricted fresh-backing/no-overlap execution history,
matching that contract would give a complete conditional image: nine zero
holes plus the five selected produced/relocated dwords in the byte table.
Reproducing that image would not require deciding whether the holes are
read. **The historical-image equivalence is conditional**, because the
retained ELF's exact loaded libdrm/kernel pair is not authenticated and
fresh storage cannot be inferred from the frozen shader alone. No fresh
allocation or submission was attempted.

These are the already catalogued public packages in
[source provenance](../xpsb-re/source-provenance.md), inspected only in
`/tmp/sgx535-pairing/` and `/tmp/sgx535-libdrm-psb/`. `drm_ttm.c` carries
a permissive Tungsten Graphics notice. SHA-256 of the inspected files:

| File | SHA-256 |
| --- | --- |
| `drm_ttm.c` | `118952a4dc011dd5de5152715d0ef05356edbd6dbcfca16f349133f8fe215166` |
| `drm_bo.c` | `d194f8b961eb18fe03b1c9cda126b95097c4f9084a0cf5df3f92f752b596c8f2` |
| `psb_buffer.c` | `f2e6833fc82de488c6c41e99a5da2c5b00ec0fea36b9defeb4057e70f464c147` |
| `drm_vm.c` | `04825cfdc25ddf03a87d604bb3f105221438e46842567bcc43fa2f8a9077b873` |
| candidate libdrm `xf86drm.c` | `5db8e444c64af12dd88b24440e84c8c508bac9fdee38c76dd19d21f4a4a11587` |

## Clean-room initialization proof obligations

| Obligation | Result from this trace |
| --- | --- |
| CPU-writable mapping | CONFIRMED software contract: pool mapper returns the writable reservation used by the selected stores. Exact platform coherency remains a separate compatibility dependency. |
| Full range valid to write | CONFIRMED at allocator level: 56 bytes fit the 400-byte reservation and the slot-capacity checks. |
| No allocator metadata in the range | CONFIRMED: list links, reference/fence state and slot offset reside in separate host objects. |
| No slot must remain unmodified | No allocator preservation rule found; **UNKNOWN architecturally** for the nine selected data slots. Other programs writing the same relative offsets does not prove their selected-program semantics. |
| Address/layout unchanged | CONFIRMED arithmetic condition: filling bytes in place does not change the returned BO/offset or descriptor count. |
| Instructions/relocations preserved | CONFIRMED construction ordering at CPU level: fill only `[+0x00,+0x30)` before established selected stores and relocation processing; keep `+0x30/+0x34` intact. No implementation was added. |
| Hardware-visible contract permits chosen values | **UNKNOWN**: CPU prefix extent does not establish that a fixed hole value preserves the PDS operation, nor exclude implicit state outside this range. |

Explicit clearing and template filling solve CPU byte determinism but do
not by themselves supply a semantic constraint. Fresh candidate-kernel
backing additionally supplies a conditional historical zero-image witness;
its pairing/history requirements above remain open. No historical
full-prefix template was found in the traced userspace lifecycle.

The stronger claim required to remove FG-02's functional blocker is **NOT
YET ESTABLISHED**. The byte-write contract does not establish that choosing
zero (or any other fixed value) for a potentially live hidden source preserves
the selected PDS task/USSE handoff. Nor does the historic software define
an exact stale value to reproduce. The lifecycle work proves writable extent
and a real stale-content route; it does not prove architectural source
non-consumption or functional equivalence for a newly initialized image.
For unrestricted reuse, the remaining fact is **whether any of the nine
slots affects the selected PDS operation, and if so what value it requires**.
For a restricted fresh-backing construction, the alternative dependency is
**establishing the applicable zero-filled backing contract and excluding
prior writes to the selected range**. The candidate source provides the
mechanism but not exact historical pairing. The read-set remains
UNKNOWN, FG-02 remains OPEN, Gate B remains BLOCKED, whitelist `[]`, and
hardware functionality remains UNVERIFIED.

## Pairing and first-use refinement (P7H-029–031)

The [pairing proof](pds-stack-pairing.md) now discharges the no-overlap
condition for the reconstructed successful first scene. Event data precedes
event PDS; the internal-clear branch also emits a separately aligned secondary
PDS. The selected primary starts at slot-relative **0x160** without internal
clear or **0x1c0** with it. Neither branch rotates or overlaps prior payloads.
The [reproducible timeline](pds-first-use-timeline.csv) supersedes the earlier
coarse offset lower bound above, within this explicit first-use scope.

The candidate libdrm/kernel generic drm.h files are byte-identical, and all
115 measured i386 BO/fence wire-layout metrics agree. The selected candidate
PDS route maps and GPU-binds the same freshly zeroed TTM pages. These are
confirmed source facts. The retained DRI's runtime DRM check accepts major4
and minor>=0, so it does not identify this page-initialization implementation.
The remaining conditional dependency is **the applicable target backing
provider's fresh-zero/same-page contract**, rather than an unexplained
predecessor allocation. Exact historical package identity can be replaced
by a demonstrated equivalent contract, but wire-layout agreement alone does
not establish that equivalence. No recycled-slot or PDS read-set claim changes.

## Provider-family follow-up

[P7H-032–035](pds-backing-provider.md) extends the candidate zero/preservation
contract across the retained package's OBS kernel source lineage and recovered
Ubuntu PSB sources. Physical page reuse below TTM is covered by explicit zero
allocation; userspace pool reuse remains stale-capable. Applying the bounded
contract to the frozen target still requires a supplying-provider identity or
equivalent implementation binding. No lifecycle or ISA status is promoted.
