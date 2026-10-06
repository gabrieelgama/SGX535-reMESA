# FIRE #2 — normal rev121 TA-root grammar, 2026-10-06

**Endpoint B; DECODER_FORMAT_PARTIAL. Common ABI remains B —
COMMON_RASTER_ABI_STRONGLY_SUPPORTED_BUT_INCOMPLETE.** This investigation
establishes the normal scene's allocation envelope and address handoff more
precisely. It does not establish the payload traversal grammar. No decoder,
capture instrumentation, rendering change, build or new candidate is justified.

The **first irreducible missing format fact** is:

> For the normal rev121 raster controls, what is the typed interpretation of
> a unit beginning at `S+0x1000`: how does ISP distinguish empty/populated/
> continuation state and derive the next raster-reference address from it?

This is one missing root-to-next-object transition rule. The twelve-byte
allocation unit is established; its field meanings are not. Resolving that
rule would permit testing the downstream CPU/TA primitive equivalence; it
does not pre-confirm that equivalence. Publication/stability remains a
separate unresolved obligation and was not investigated here.

## Evidence and scope

The applicable pinned Xpsb implementation remains **SGX535 REV121 HISTORICAL
EVIDENCE**, for the revision provenance already established in the
[common-ABI report](fire02-xpsb-common-raster-abi-20261006.md). Revision identity
supports applicability, not producer equivalence. Xpsb SHA-256 remains
`da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f`.
ELF virtual addresses below are unrebased; Ghidra exports use `+0x10000`.
Assembly controls inferred decompiler types, notably the register-argument
helper at `0x4030`.

[Current audit](/home/gama/sgx535-offline/phase8-normal-root-grammar-20261006T013334Z/)
retains targeted assembly, Ghidra exports and arithmetic/source checks. The
copied analysis project leaves the preceding audit intact. Normal-path targets
are `0x30f0`, `0x3a40`, `0x3d70`, `0x3df0`, `0x4030`, `0x4550`, `0x5040`,
with a bounded check of `0x3cf0` for the root-selection input.

Primary source snapshots inspected: `PSB_psb_scene_c.txt`,
`PSB_psb_xhw_c.txt`, `PSB_psb_schedule_c.txt`, `PSB_psb_drm_h.txt`,
`PSB_psb_reg_h.txt`, and permitted SGX535 definitions. Missing host declarations
were retrieved from the already-preserved public primary source tree:
[psb_scene.h](https://github.com/gregkh/psb-kmp/blob/98b5307e5158a9ac401b29128ddd1184ae06b4d7/psb_scene.h)
and [psb_schedule.h](https://github.com/gregkh/psb-kmp/blob/98b5307e5158a9ac401b29128ddd1184ae06b4d7/psb_schedule.h).
Their Git blob identities match the repository's retained tree exactly;
copies and SHA-256s are in `public-headers/manifest.json` in the audit. These
declare host objects, not the TA-produced root payload.

The permitted frozen owner/backend/contract were read only for object roles,
initialization and matching register formulas. The excluded current
ioctl-entry/reporting implementation was not read. No VISTEST or Vita branch
was reopened. No historical executable was run.

## What S and S+0x1000 actually establish

**CONFIRMED software identity:** `S` is the GPU address/offset of
`scene->hw_data`, not the host `struct psb_scene`, its cookie, the DPM allocator
BO, or the parameter BO. Historical scene allocation requests the byte count
returned by XHW scene-info, in the MMU domain with READ/WRITE/CACHED flags
([scene source](../poulsbo-data/PSB_psb_scene_c.txt#L106), allocation at lines136–145).
The scheduler passes `scene->hw_data->offset` independently of
`scene->hw_cookie` ([scheduler](../poulsbo-data/PSB_psb_schedule_c.txt#L199)).
XHW copies the cookie into the request; it does not copy it into S
([XHW source](../poulsbo-data/PSB_psb_xhw_c.txt#L148)).

The frozen contract allocates S as8192 bytes,4096-byte aligned in its private
GPU MMU address space; P is a separate32-MiB MMU DPM allocator object and Q a
separate32-MiB,1-MiB-aligned RASTGEOM parameter object
([requirements](../../tools/psb-dri-re/frozen_kernel_contract.c#L9)). CPU page
objects and GPU VAs are linked by the permitted owner mapping. A GPU address
is not a CPU pointer. Actual FIRE #2 root/parameter contents were not retained.

**CONFIRMED calculation**, `Xpsb_scene_info`, ELF `0x3a40–0x3bfd`, selected
32×32/default rev121 option branch:

```text
ux = ceil(width/16) = 2; uy = ceil(height/16) = 2
ax = align4(ceil(ux/2)) = 4; ay = align4(ceil(uy/2)) = 4
fx = 2*ax = 8; fy = 2*ay = 8
p = next_power_of_two(max(fx,fy)) = 8
cookie[7] = align4096(4*p*p) = 0x1000
cookie[9] = cookie[7] + 12*fx*fy = 0x1300
cookie[10] = cookie[9]+0x50 = 0x1350
cookie[11] = cookie[10]+0x90 = 0x13e0
requested_size = cookie[11]+0x40 = 0x1420
clear_start_page=0; clear_page_count=1
```

The `+0x1000` is therefore a **calculated allocation boundary**, not a literal
page-relative primitive pointer or a parameter-memory base. Cookie words2/3
are `0x01004004`, word1/4 are16, word5 is`0x1001`, word6 is`0x1f01f`.
These agree with the permitted frozen contract's
[scene-info calculation](../../tools/psb-dri-re/frozen_kernel_contract.c#L320).
The historical function writes words0–14; it does not initialize word15.
The current contract's zero word15 is a separate clean-room policy. The
initial selected normal branch does not require the historical OOM selector.

**CONFIRMED address programming:** helper `0x4030`, TA arm `0x4073–0x40bb`,
writes MMIO `0x21c = S+cookie[7]`; normal raster arm `0x41d5–0x4202` writes
`0x408 = S+cookie[7]`, with `0x63c=3`, `0x658=0`. There is no pointer shift or
mask at this normal root addition/store. Its software representation is a
full32-bit GPU address. Hardware-internal fetch/address transformations are
not established by the store. The selected frozen
[TA plan](../../tools/psb-dri-re/frozen_kernel_contract.c#L556) and
[raster plan](../../tools/psb-dri-re/frozen_kernel_contract.c#L618) match it.

**INFERRED hardware role:** a region-associated raster input reservation,
because TA and raster point at it and allocation depends on scene dimensions.
**UNKNOWN exact type:** region headers versus descriptors/pointer records,
their fields, and the first dereference. Naming it a root does not decide that.

## Allocation envelope and writer/consumer map

Ranges are half-open offsets from S. These are software reservation boundaries,
not invented payload field layouts.

| Range | Size | CPU initialization | Hardware relationship | Confidence |
| --- | ---: | --- | --- | --- |
| `0000–1000` |4096 | Historical explicit clear range; selected current owner clears all pages | TA`0x220=S`; address selected independently from root | Clear/address CONFIRMED; payload meaning UNKNOWN |
| `1000–1300` |768=`64*12` | Historical explicit clear does **not** cover it; no header/sentinel initialization found in normal setup. Current owner zeros backing here | TA`0x21c`, raster`0x408` select its start | Reservation/selection CONFIRMED; TA-produced region records INFERRED; field grammar UNKNOWN |
| `1300–1350` |80 | No typed CPU initialization found | TA`0x234=S+1300` | Boundary/address CONFIRMED; stored fields UNKNOWN |
| `1350–13e0` |144 | No typed CPU initialization found | DPM-side`0x62c=S+1350` | Boundary/address CONFIRMED; fields UNKNOWN |
| `13e0–1420` |64 | No typed CPU initialization found | DPM-side`0x634=S+13e0` | Boundary/address CONFIRMED; fields UNKNOWN |
| `1420–2000` |3040 | Current allocation's page-rounded zero backing | No additional selected payload role established | Current padding CONFIRMED; no record grammar |

Historical `psb_clear_scene[_atomic]` only zeroes the returned page interval
([scene source](../poulsbo-data/PSB_psb_scene_c.txt#L30)). New historical scenes
are marked CLEARED after allocation; this source does not explicitly clear
their entire BO. Reuse waits for the BO then clears that interval. Host
`drm_calloc(struct psb_scene)` is not a write of scene BO headers.
Current [owner initialization](../../kernel/sgx535_frozen/gma500_bo_owner.c#L316)
explicitly zeroes every owned BO page and labels that as clean-room policy.
Neither policy establishes that an all-zero normal root is a valid empty list.

Scene-info is CPU computation of **host cookie metadata**, not a writer of
root words. Normal bind/setup configures addresses and DPM/MTE controls, then
starts TA; raster later consumes the configured root. TA mutation of the
root reservation and reference links into Q is the likely producer/consumer
relationship, but no inspected software path reads those payload words back.
`Xpsb_ta_mem_load` (`0x3df0`) configures separate allocator/parameter state:
Q is masked to28 bits and transformed to page indices, while P is programmed
to DPM bases`0x618/600/610/608`. Its load/init waits establish no payload parser.
The host `hw_cookie` and P's DPM page indices are not SGX MMU PTEs or scene
region pointers.

## Region and reference grammar: established envelope, opaque transition

The software screen-grid calculation uses16-coordinate units, and the
selected target yields2×2 before padding/doubling. **CONFIRMED arithmetic;
UNKNOWN physical tile dimensions and tile-to-record indexing.** A4×4 padded
grid and64 twelve-byte allocation units do not prove64 visible regions.
No row-major/Morton order, per-region screen bounds, valid/empty tag, list
length, end marker or chaining pointer was identified.

There is additional **CONFIRMED selection arithmetic**, not a decoder:
`0x4030`'s exceptional/partial-render arm selects
`S+cookie[7]+(cookie[15]&15)*cookie[1]*12` when bit4 of word15 is clear.
For this size that advances by192 bytes; four such slices cover the768-byte
reservation. The corresponding small-scene host loop handles four slots
(`0x4873–0x4880`). `Xpsb_oom_abort` (`0x3d03–0x3d09`) copies MMIO`0x720`
into cookie15. Those facts support a structured, selectable reservation.
They do not define a slice as a region or reveal its three dwords.

The apparent eight-byte table loop in `Xpsb_scene_switch_fire` is **not**
normal-root traversal. `param1[0x4e]` means `private+0x1c4`, holding a CPU
mapping of a separate`0x88`-byte communication BO (`XpsbInit` ELF
`0x2cf5–0x2d6f`). The loop preserves upper16 bits across DPM save/load
operations. Its pointer is neither S nor Q. Likewise, `0x4030`'s other
eight-byte loop emits supplied offset/value pairs to MMIO. Neither supplies
a TA-generated reference-entry stride.

Bounded constant checks find Xpsb `0x40200000`, `0x03ffffff` and
`0xc0000000` together in the **CPU** flat-list encoder (`0x7c90–0x7e16`),
not a normal-root reader. DRI's `0x2a46b` terminator belongs to the **input**
TA command stream. Other matching high-bit constants are scalar field
encoders/host lock operations, not typed TA-output transitions. The permitted
SGX535 header's OTPM load/flush event masks describe events, not record words.
The additional pinned host headers provide no region/ref payload schema.

## Producer/consumer convergence comparison

| Feature | CPU quad path | Normal TA path | Relation |
| --- | --- | --- | --- |
| Root register | `0x408` with relocated geometry BO list address masked28 bits | `0x408=S+cookie7+selectionOffset`, no software mask | IDENTICAL register; DIFFERENT software address construction |
| Root ownership | CPU geometry BO, appended flat list | Separate scene S; dimension-derived reservation | DIFFERENT objects/layout envelope; not proof of incompatible primitive ABI |
| Root units |8-byte references plus4-byte terminator | Allocation in12-byte units; selected slices stride`12*cookie1` | UNKNOWN structural convergence;12-byte entry interpretation unproved |
| Empty/end/chain | CPU writes`0xc0000000` as flat-list end | No normal payload tag or next-pointer extraction established | UNKNOWN |
| Primitive pointer | CPU reference word1: shifted/masked relocation, count bits | No decoded normal reference yet | UNKNOWN |
| Parameter storage | CPU-authored records/anchors in geometry backing | Separate Q selected through DPM/RASTGEOM address programming | STRUCTURALLY_COMPATIBLE separate raster geometry concept; byte/address grammar UNKNOWN |
| Coordinate/index records | Explicit CPU writer formulas, quad indices, packedXY, adjacent1.0 | No hardware-produced primitive word reader/schema | UNKNOWN |
| Consumption | Raster scheduler/root family/`Xpsb_closed_kick_render` | Same raster scheduler/root family/kick after TA | IDENTICAL shared software consumer path; hardware payload equivalence incomplete |
| Raster controls | CPU path includes`0x4bc=0x30041c0` | Normal path includes`0x4bc=0x20041c0` | DIFFERENT values; their root-format implications UNKNOWN |

The CPU encoder can remain a specification for its demonstrated software
subset. It cannot yet specify the normal TA-produced root or its triangle
record. Different root organizations can converge later, but no evidence
establishes that later convergence here. Classification B is unchanged;
neither A nor D follows from these comparisons.

## Minimal schema and selected-triangle gate

```text
HOST_SCENE_COOKIE[16]: CPU metadata separate from S
  word7: root reservation offset (selected 0x1000)
  word9: end of reservation (selected 0x1300)
  word1: selection stride factor (selected 16)
  words10/11: following state-area offsets

SCENE_S[0x2000]: selected owned GPU backing
  [0x0000,0x1000): explicitly cleared prefix
  [0x1000,0x1300): opaque reservation, 64 allocation units * 12 bytes
    unit[j]+{0,4,8}: opaque DWORD positions; hardware meaning UNKNOWN
  [0x1300,0x1420): three separately addressed state reservations

NORMAL_ROOT = GPU(S)+cookie7       # initial selected raster branch
NORMAL_ROOT -> NEXT_TYPED_OBJECT  # UNKNOWN: tag + pointer/advance rule
NEXT_TYPED_OBJECT -> PRIMITIVE    # not yet established
PRIMITIVE -> SELECTED_TRIANGLE    # CPU format only supplies conditional leads
```

The opaque dword positions are an envelope, not a validated three-word
hardware header declaration. Entry width, pointer mask/shift/address space,
chaining, termination and primitive class are not assigned.

Known input remains `(8,8),(24,8),(8,24)`, area128 and edges
`x=8,y=8,x+y=32`. Conditional CPU packing yields
`40804080,41804080,40804180`. No established normal typed traversal connects
these values to raster-consumable output. A match could be stale data, copied
input or an unrelated record. Region membership and absence cannot be checked.
Thus **DECODER_FORMAT_PARTIAL** describes the allocation/root envelope;
selected-triangle recognition is not sufficient for a decoder. All hypothetical
snapshots still classify UNKNOWN under currently supported semantics. No
CPU-only decoder or its native/UBSan/i386 qualification is justified.

## Checks, separate publication boundary and next action

**33/33 PASS** CPU source/layout consistency checks verify cookie0–14 against
the permitted frozen contract,32×32 arithmetic, reservation/nonoverlap/selection
bounds, root stores, binary identity and new header Git blobs. These are
reference-consistency checks, **not hardware or TA-decoder qualification**.
The audit's final consistency receipt also checks document references, all68
FIRE #2 file hashes, unchanged implementation/artifact bytes and empty index.

No GPU→CPU publication mechanism was investigated or claimed. Historical
scene-reuse waits and DPM load waits were encountered only to place allocation
and setup in their lifecycle. They do not define root payload visibility or a
safe CPU snapshot. FIRE #2 contains no TA-output payload and cannot be decoded
retrospectively. Its completion/provenance findings and all-zero readback stand.

**Smallest next action:** obtain permitted rev121 consumer evidence for the
normal root-unit's typed successor rule under the selected raster controls;
then test downstream compatibility with the demonstrated CPU records. Only
after format sufficiency may the independent publication gate be addressed.
No live capture is justified at the present format boundary.

Only this report, its structured finding and the latest handoff section change
in the repository. No implementation or candidate identities change:
driver`9d0b5b2fdb7d9881f2828f43fb89253176c38817`,
observer`f11d3abb072caa4e1d32836ef92ce201e9c9d126`,
imageSHA-256`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`.
FIRE #3 remains **UNAUTHORIZED**; `sgx_execution_authorized=false`;
task SGX invocations=0; task hardware interactions=0; triangle **NOT ESTABLISHED**.
