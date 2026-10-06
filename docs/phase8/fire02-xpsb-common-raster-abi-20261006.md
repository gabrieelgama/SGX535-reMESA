# FIRE #2 — Xpsb CPU records and the common raster ABI, 2026-10-06

**Finding B: COMMON_RASTER_ABI_STRONGLY_SUPPORTED_BUT_INCOMPLETE.** The
applicable Xpsb path is **SGX535 REV121 HISTORICAL EVIDENCE**, not an SGX543
layout transfer. CPU-authored records and normal TA scenes converge on the
same raster scheduler, root-register family and Xpsb raster kick. The CPU
encoder establishes a software-authored raster-input subset. It does not yet
establish the grammar of hardware-produced references reachable from the
normal scene root. No TA-output decoder or capture implementation is justified.

The **single remaining equivalence proposition** is:

> Under the normal rev121 scene/raster controls, the TA-produced root at
> `S+0x1000` reaches primitive records through a defined region/reference
> grammar whose address and primitive interpretation agrees with the
> CPU-generated grammar below for the selected triangle's primitive class.

This is a missing consumer-format fact, not a claim that the formats differ.
The independent GPU→CPU publication/stability obligation also remains open.
Recognizing the rev121 provenance strengthens finding B; it does **not**
promote it to A. No rev121↔rev122 compatibility premise is required here.

## Provenance and evidence scope

The retained [Xpsb package specification](../../references/home:lkundrak:poulsbo/xpsb-glx/xpsb-glx.spec)
identifies Xpsb 0.18, i686, Poulsbo, from the Ubuntu-mobile package. The actual
`drivers/Xpsb.so` is SHA-256
`da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f`;
its `XpsbInit` identifies release `5.0.1.0046`. Package naming alone does not
prove the revision.

**CONFIRMED static applicability:** `XpsbInit` at ELF `0x2690` zero-allocates
the context, passes `private+0x8c` to `Xpsb_set_vopt` at `0x2c12`, then to
`Xpsb_sgx_initialize` at `0x2c1a`. `set_vopt`, ELF `0x4b02–0x4b14`, reads
SGX-relative `0x14` and computes `100 + low_byte + 10*second_byte`.
Raw `0x00010201` gives **121**. It takes the default option path, not the
107–113 workaround cases. Initialization at `0x3820–0x38ee` then reaches
the normal register branch; there is no rev122 dependency in this selected
path. The quad encoder/finish functions are not gated to a different revision.
The repository's [retained rev121 initialization correlation](kernel-submission-reconstruction.md#L419)
and [exact-revision contract](../../tools/psb-dri-re/frozen_kernel_contract.c#L431)
independently bind that selected branch to `0x00010201`.

Thus the applicable CPU path is classified **SGX535 REV121 HISTORICAL
EVIDENCE**. This means a static historical path applicable to the selected
rev121 stack, not a rev121-only binary or proof that this task ran the old
quad compositor. The Mini 12's established `CORE_ID=0x01130000` and
`CORE_REVISION=0x00010201` remain the existing target context, reaffirmed by
the maintainer; no new hardware measurement was made.

The public DDX source 0.32.0 was reacquired from the
[preserved Fedora source RPM](https://ftp.gwdg.de/pub/linux/rpmfusion/nonfree/fedora/updates/11/SRPMS/xorg-x11-drv-psb-0.32.0-1.fc11.src.rpm),
SHA-256 `62219cacc062ba978b8a93c9606ce2fdce67eb3471d57ef5f9e77f59b8ec791f`.
Only selected MIT-licensed source files were extracted. Exact DDX/kernel/binary
build pairing remains **INFERRED**; the register and record writes within the
pinned Xpsb ELF are direct evidence independent of that pairing.

[Audit directory](/home/gama/sgx535-offline/phase8-common-raster-abi-20261006T005131Z/)
retains targeted disassembly, selected Ghidra exports, source copies and
consistency receipts. Addresses in this report are **ELF VAs**, not Ghidra's
`+0x10000` rebasing. Assembly controls uncertain decompiler types: in particular
the x87 coordinate conversion uses **round toward zero**, despite Ghidra's
`ROUND` notation. No historic binary was executed.

## Complete bounded CPU producer path

1. Public `src/psb_composite.c:134–208` prepares surfaces/textures and calls
   `psb3DPrepareComposite`. Lines210–289 construct four rectangle vertices:
   top-left, top-right, bottom-left, bottom-right, with texture coordinates;
   line289 calls `psb3DCompositeQuad`, and lines293–297 finish the batch.
2. `XpsbInit` / `Xpsb3DContextInit` (`0x2690`, `0x51c0`) allocate separate
   pools/wrappers. Shader slots are `0x400` bytes; geometry slots `0x2000`;
   render-register slots `0x1000`; each batch pool has64 slots. The geometry
   backing pool is created with flags `0x80000025`, separately from shaders
   (`0x20000025`) and register backing (`0x010000a4`). Allocation flags of the
   underlying pool, not the wrapper's default fields, determine its backing.
3. `psb3DPrepareComposite` (`0x5ee0`, calls `0x60ae–0x614e`) obtains/maps
   shader wrapper `private+0x1d0` and geometry wrapper `private+0x1d4`, then
   `XpsbResetParamContext` (`0x74c0`, call `0x637a`) records the **same**
   geometry BO, CPU base/cursor and relocation context.
4. `psb3DCompositeQuad` (`0x6430`) finishes/reprepares after its block limit;
   `XpsbAddQuad` (`0x7b50`) copies four vertices per quad, tracks bounds and
   flushes when three quads are buffered. `XpsbFlushParamblock` (`0x7560`)
   emits the layouts below. Host metadata contains record anchors/counts;
   it is not itself GPU region memory.
5. `XpsbFlushParamContext` (`0x7c90`) appends a64-byte-aligned **flat
   reference stream**, stores its BO-relative offset and terminates it.
   `psb3DCompositeFinish` (`0x5c40`) additionally constructs a background
   object (`0x71e0`) in geometry storage and bounding/raster state. That
   object is separately addressed; bytes following the list terminator are
   not evidence of another list entry.
6. Raster builder `0x5830` emits register pairs into the separate register
   BO. Register `0x408` is relocated from **geometry BO + saved list offset**.
   It emits global start/end grid bounds through `0x40c/0x410`. This path
   does not construct the normal scene's per-region header array.
7. Finish unmaps shader/geometry wrappers (`0x5e60/0x5e6e`) and invokes
   `XpsbFlush3D` (`0x9f70`, call `0x5ea5`). Its raster-only branch selects
   engine2 and rejects a non-null TA command descriptor. It validates BOs,
   submits index0 with a `0x90`-byte argument at `0xa325`, and attaches fence
   feedback to the BO list before releasing its submission-list reference.
8. Batch-pool map (`0xc700`) prevents already-mapped access and waits/refuses
   fenced reuse; unmap (`0xbff0`) changes a host map flag. Release (`0xc4a0`)
   puts fenced slots on a pending list; reclamation (`0xc310`) checks/waits
   their fence before returning slots to the free list. Context teardown
   (`0x5170`) unrefs wrappers; pool teardown (`0xc410`) waits/reclaims before
   releasing the parent BO. This is software lifetime evidence, not TA-output
   CPU publication.

## CPU byte/dword layouts

**CONFIRMED emitted bytes/formulas.** Hardware interpretations of unnamed
bits are not assigned from their values. Let `q=1..3`, `a=attribute-pair
count`, and `B` be the64-byte-aligned start of a flushed group. Input vertex
stride is `8*(a+1)` bytes. All words are little-endian32-bit.

For quad `j`, let `i=4*j`. The emitted index word is:

```text
I(j) = 0x42104210 | i | (i<<16) | ((i+1)<<26)
       | ((i+3)<<21) | ((i+3)<<10) | ((i+2)<<5)
XY(x,y) = (trunc((x+1024)*16+0.5)<<16)
          | trunc((y+1024)*16+0.5)
```

The constants are ELF `.rodata` `0xd4d4=1024.0`, `0xd4d8=16.0`,
`0xd4dc=0.5`; conversion is at `0x7725–0x7777`, repeated for the second
record. Coordinates in this bounded path are nonnegative screen coordinates;
no signed/clipping/overflow decoder contract is asserted.

| Offset from B | Emission | Evidence / interpretation |
| --- | --- | --- |
| `0x00,04,08` | three zero words | CONFIRMED prefix; semantics UNKNOWN |
| `0x0c` | `0x07e80000` | CONFIRMED record anchor, not a decoded hardware primitive tag |
| `0x10` | `0x02000000` | CONFIRMED |
| `0x14+4*j` | `I(j)`, q words | CONFIRMED vertex-index relationships; two-primitive relationship INFERRED from counts/quad geometry |
| `20+4*q`, then `24+4*q` | zero, zero | CONFIRMED |
| `28+4*q+8*v`, `v=0..4q-1` | `XY(xv,yv)`, then `0x3f800000` | CONFIRMED packed screen coordinates and adjacent1.0; plane/depth hardware interpretation UNKNOWN |

Size of this first block is `28+36*q`. Its reference anchor is **B+0x0c**,
not B. If `a!=0`, a second record follows immediately, without a separate
64-byte alignment:

| Position | Emission |
| --- | --- |
| first three words | shader/state address relocations and scalar word, below |
| next `4*q*a` pairs | input attribute pairs copied as float bits, vertex-major |
| H | `(flag==0 ? 0x02000000 : 0) + 0x01c00000` |
| `H+4+4*j` | `I(j)` |
| `H+4+4*q`, next word | `2*a`, zero |
| `H+12+4*q+8*v` | `XY(xv,yv)`, `0x3f800000` |

For descriptor words `d[]`, the prefix emissions are:

```text
W0 = relocate(d[0], d[4], right=4, mask=0x00ffffff,
              background=(d[5]&0xfc)<<24)
W1 = d[3] | 0x40030000 | (((d[6]+0x7f)&0x3f80)<<11)
W2 = relocate(d[0], d[1], right=4, mask=0x00ffffff,
              background=((d[2]&0x3fffffff)>>2)<<26)
```

The second anchor is **H**, after prefix/attributes; treating a reference
as the start of that prefix would be wrong. Second-record size is
`24+36*q+32*q*a`. These are software emission relations, not proof that TA
generates these variants or the same adjacent depth/attribute representation.

Each **host-only** metadata entry is28 bytes:
`{BO wrapper, anchor offset, 4q, 2q, (1<<(4q))-1, (1<<(2q))-1, 1}`.
The two-word GPU reference derived from that entry is:

```text
R0 = 0x40200000 | primitiveMask | (flag<<22)
     | (vertexMask<<8) | ((vertexCount-1)<<25)
R1 = (((GPU(BO)+anchor_offset)>>2)&0x03ffffff)
     | ((primitiveCount-1)<<27)
```

The reference stream is `N*(R0,R1)` followed by the single word
`0xc0000000`, length `8*N+4`. With no metadata, the writer emits just the
terminator. This establishes a **software empty stream**, not the hardware
empty-region encoding of the TA scene. No hardware block-chaining opcode or
region-header grammar is established by this emitter.

`XpsbSetOffsetRelocation` at `0x91d0` writes relocation metadata and a
`0x67676767` placeholder, not final GPU pointers. The compatible historical
[kernel relocation path](../poulsbo-data/PSB_psb_sgx_c.txt#L631) computes
`val=BO.offset+pre_add`, shifts it, and writes
`(background & ~mask) | (val & mask)` at lines694–777. Thus pre-relocation
CPU bytes cannot be decoded as actual pointers. Masked geometry addressing
agrees structurally with the historical RASTGEOM aperture at `0x30000000`
([definition](../poulsbo-data/PSB_psb_drm_h.txt#L53)); implicit address-prefix
reconstruction by hardware is not fully specified here.

## Raster consumer and independent normal TA path

**CONFIRMED within the historical PSB source:** engine2 goes to
`psb_cmdbuf_raster`, with no scene; engine3 uses a scene/TA path
([dispatch](../poulsbo-data/PSB_psb_sgx_c.txt#L1305)). The raster-only task
skips a TA command and queues raster work
([task construction](../poulsbo-data/PSB_psb_schedule_c.txt#L1323)). Normal
TA completion marks the scene COMPLETE/DIRTY and queues raster
([TA done](../poulsbo-data/PSB_psb_schedule_c.txt#L421)). Both reach
`psb_schedule_raster`, submit the task's raster register pairs, and fire
the raster engine ([shared scheduler](../poulsbo-data/PSB_psb_schedule_c.txt#L339)).
For a scene it calls bind/fire; without a scene it calls direct raster fire.
The Xpsb binary's selected scene raster branch at `0x4550` calls helper
`0x4030` then **the same `Xpsb_closed_kick_render` at0x5040** used by
the direct XHW render path. Its final render writes are `0x43c=1`,
`0xa08=1`, `0x428=1`. None was executed in this task.

The actual normal scene address handoff is separate from the quad BO:

| Owned object | Allocation/domain | TA producer handoff | Raster consumer handoff | Level |
| --- | --- | --- | --- | --- |
| S, scene hardware storage | frozen8192 bytes,4-KiB alignment, MMU | `0x220=S`; `0x21c=S+cookie[7]` | helper0x4030 writes `0x408=S+cookie[7]+context_offset`; selected initial32×32 offset is0, cookie[7]=0x1000 | CONFIRMED software addresses; root grammar UNKNOWN |
| P, DPM allocator storage | frozen32MiB,4-KiB alignment, MMU | descriptor/page-state load bases `0x600/608/610/618` | scene/DPM context load/store | CONFIRMED separate object; not primitive bytes or SGX-MMU PTE directory |
| Q, TA parameter storage | frozen32MiB,1-MiB alignment, RASTGEOM | parameter page interval supplied in TA-memory cookie | presumed parameter targets of scene references | allocation/page interval CONFIRMED; root→typed Q reference UNKNOWN |
| vertex/state/index input | independently serialized CPU input objects | PDS/USE, TA stream and TA register command | TA transformation precedes raster | CONFIRMED input, not retained TA output |

The [qualified frozen object/address contract](../../tools/psb-dri-re/frozen_kernel_contract.c#L15)
and [scene cookie](../../tools/psb-dri-re/frozen_kernel_contract.c#L322) agree
with the historical Xpsb scene-info/bind path. Scene root, allocator and
parameter arena must not be merged. In other historical contexts helper0x4030
can add `context_index * cookie[1] * 12` to the root: this confirms a
context-array stride calculation, **not** a decoded12-byte region header.
No software decoder of the hardware TA emitter was found in the traced paths.

## Field-by-field convergence and limits

| CPU quad producer | Consumer relationship | Normal TA counterpart | Finding |
| --- | --- | --- | --- |
| geometry BO+flat reference offset, low28 address bits into0x408 | direct raster input root | S+0x1000 into0x408, including scene context handling | Same register CONFIRMED; root form equivalence UNKNOWN |
| `R1`, right2/mask0x03ffffff | anchor addressing plus primitive-count bits | separate RASTGEOM Q arena known | Compatible address organization INFERRED; TA-produced pointer grammar UNKNOWN |
| 28-byte host metadata→8-byte reference | flat list counts/masks/address | scene size and context calculations contain12-byte terms | Different software objects CONFIRMED; treating these sizes as a shared record is rejected |
| 64-byte group/root alignment | emitter constraints | scene4-KiB/Q1-MiB alignment | Compatible containment; TA block alignment UNKNOWN |
| indexed quads with4q vertices/2q primitives | primitive relationships and coordinate payload | selected triangle has3 vertices, z=0.5 | Actual TA emission/clip/plane/depth representation UNKNOWN |
| state prefix, attribute pairs, header0x01c00000 variant | CPU-authored parameter record | normal DRI background builder0x38d87 emits explicit reference metadata4/2/0xf/3/1, anchor+0x2c, header0x01c00000, packed coordinates and1.0 | Independently corroborated **software-authored** raster subset; not hardware TA output |
| `0xc0000000` list ending | explicit flat CPU reference termination | same literal ends TA **input** stream | IDENTICAL VALUE; different producer/stage, not TA-output termination proof |
| global0x40c/410 bounds on16-unit grid | CPU raster extent | normal scene setup computes16-unit allocation grid | COMPATIBLE STRUCTURE; physical region/list membership rules UNKNOWN |
| 0x414=0x100,0x418=1,0x4c8=0x88,0xa64=0x4fff | common raster-state values | same values in frozen raster template | IDENTICAL VALUES; no grammar proof alone |
| 0x4bc=0x300;0x41c=0 | CPU selected raster controls | normal0x4bc=0x200;0x41c=0x0da24260; additional scene/0x400 state | Differences CONFIRMED; whether they select different root/record forms UNKNOWN |
| scheduler/raster kick | same physical consumer stage | scene-bound normal raster kick | Common consumer CONFIRMED structurally; one universal input grammar not established |

The independent DRI software-background correlation was checked directly at
ELF `0x39000–0x391c0` in the same package. It strengthens a software encoder
interpretation beyond matching magic values: counts/masks, reference anchor,
address relocations, header and coordinate payload converge. It still bypasses
the missing **hardware TA producer** edge. Different index literals can encode
different vertex orders without proving different ABIs; identical consumer
kicks can consume distinct control-selected forms without proving one ABI.
Both overclaims are rejected.

The CPU writer does not construct explicit edge/plane coefficients in this
bounded path. Whether ISP derives them, TA emits them instead, or a mode selects
a different primitive form remains UNKNOWN. There is no proved signature for
the selected `(8,8),(24,8),(8,24)` triangle: its conditional packed coordinates
`40804080/41804080/40804180` are not sufficient without typed reachability,
primitive identity, depth/interpolation interpretation and region semantics.

## Publication, decoder/capture gate and next step

CPU→GPU evidence is stronger than a bare mapping: pool slots have backing
placement flags, mappings, relocation/validation ordering and fenced reuse.
Pool unmap is only a bookkeeping change; no explicit data-cache flush is shown
in that function. Compatible PSB RASTGEOM memory is TTM-mappable
([memory manager](../poulsbo-data/PSB_psb_buffer_c.txt#L125)); caching depends
on placement/cache flags and mapping implementation. No claim is made that
every pooled mapping has one universal cache mode. The historical
`psb_invalidate_caches` helper [returns0](../poulsbo-data/PSB_psb_buffer_c.txt#L196).
Fence feedback and pool reclamation establish intended lifetime ordering;
they do not document publication of GPU-produced TA payloads to the CPU.

**GPU→CPU publication/stability: NOT ESTABLISHED.** Required independently:
the particular root/list/parameter bytes used by raster must be materialized
in the owned CPU-accessible backing pages, with appropriate cache maintenance,
and remain stable through the copy before internal reclamation/reuse. TA_DONE,
host pinning/mapping, translation invalidation and later retirement alone
do not prove that postcondition. The candidate checkpoint remains accepted
TA→before raster, conditional on such a guarantee; it is not a qualified
snapshot point. CPU→GPU behavior cannot establish reverse-direction symmetry.

FIRE #2 retained no TA output: **post-TA coverage cannot be resolved
retrospectively**. All68 sealed files are unchanged. No historical CPU fixture
can fill those missing bytes or prove the TA emitter. Because neither the
selected-triangle TA decoder gate nor publication gate is met, **no
implementation change, production decoder, native/UBSan/i386 decoder
qualification, kernel capture, opaque dump or new candidate was made**.
The explicit layouts above are reference evidence for future CPU fixtures,
not SGX535 TA-output qualification. **124/124 CPU reference/document/preservation
consistency checks PASS**; all3721 preexisting repository files were checked,
and only the intended handoff changed. Git retains19 modified tracked files,
122 untracked entries (two new reports), and an empty index. Details are
identified in [structured finding](fire02-xpsb-common-raster-abi-20261006.json).

**Smallest next justified action:** resolve the stated normal-root/primitive
grammar equivalence proposition through permitted rev121 consumer/TA-format
evidence. Even if resolved, independently establish the backing-page
publication/stability contract before instrumenting a future capture. No new
live capture is yet justified. Once geometry survival is attributable, the
next discriminator must establish fragment/export progress independently of
color-store visibility; suffix-only fragment/zero-attribute and PBE concerns
remain unproven, and no rendering state was changed here.

Current driver Build ID `9d0b5b2fdb7d9881f2828f43fb89253176c38817`, observer
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`, image SHA-256
`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`
are unchanged. FIRE #3 remains separately **UNAUTHORIZED**;
`sgx_execution_authorized=false`; task SGX invocations=0; hardware interactions=0;
triangle **NOT ESTABLISHED**. No staging, build, reboot, module change or sealed
evidence change occurred. Historical Gates/provenance/completion remain intact.
