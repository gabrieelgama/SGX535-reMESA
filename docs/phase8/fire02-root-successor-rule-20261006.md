# FIRE #2 — normal-root successor rule, 2026-10-06

**C — SUCCESSOR_RULE_UNRESOLVED.** No normal-root EMPTY, POPULATED or
CONTINUATION predicate is established by the available permitted evidence.
Allocation and whole-slice selection are established, but not a payload-unit
successor. Decoder format remains **DECODER_FORMAT_PARTIAL** for its envelope;
common ABI remains **B — COMMON_RASTER_ABI_STRONGLY_SUPPORTED_BUT_INCOMPLETE**.
CPU flat-reference convergence remains **UNKNOWN**. No implementation is made.

The smallest missing consumer fact is:

> Under the selected normal rev121 raster controls, what interpretation of
> the bytes at the initial `S+0x1000` root position determines EMPTY versus a
> successor, and what transformation produces that successor's address?

Even the EMPTY predicate is unavailable, so this is C, not B/PARTIAL successor
semantics. It is not established that the first position is a twelve-byte
hardware record. Its width must be part of that consumer fact; no invented
three-dword parser is implied. Downstream primitive interpretation stays outside
this investigation. Publication stays separate.

## Scope and retained evidence

This continues the [normal-root envelope finding](fire02-normal-ta-root-grammar-20261006.md)
and its [structured result](fire02-normal-ta-root-grammar-20261006.json).
Applicable Xpsb evidence is **SGX535 REV121 HISTORICAL EVIDENCE**, with revision
provenance already retained in the [common-ABI report](fire02-xpsb-common-raster-abi-20261006.md).
No revision identity is used as proof of producer equivalence.

[Successor audit](/home/gama/sgx535-offline/phase8-root-successor-20261006T015923Z/)
contains current baselines, bounded searches, primary-source copies, consistency
receipts and an access ledger. Earlier analysis/export files remain unchanged.
ELF addresses below are unrebased. Pinned Xpsb SHA-256 remains
`da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f`.
The complete retained Xpsb disassembly was searched for base stores, twelve-byte
advances/multipliers,64-count candidates and the known CPU encoder constants.
Hits were inspected in their object/caller context. The normal scene-info,
bind/fire and raster-kick exports from the preceding audit were reused.

New primary-source checks recover files from trees **already pinned** in
`docs/poulsbo-data/external-trees.json`, verifying their Git blob identities:

- Historical [TTM allocator](https://github.com/gregkh/psb-kmp/blob/98b5307e5158a9ac401b29128ddd1184ae06b4d7/drm_ttm.c),
  [BO creation](https://github.com/gregkh/psb-kmp/blob/98b5307e5158a9ac401b29128ddd1184ae06b4d7/drm_bo.c)
  and `drm_objects.h` under `public-psb/` in the audit. Their notices permit
  source inspection. Source/binary exact-build pairing is still not asserted.
- EMGD `pb.c`, `sgxkick.c`, `sgx_mkif_km.h`, `sgxinfo.h`, `sgxinit.c`,
  `sgxinfokm.h` and `sgxutils.c` from commit
  `e6884ec2eaaf1afe88d5ff9dd44d70403525be5b`, under `public-emgd/` with
  URL/hash/blob manifest. These are dual MIT/GPL sources. They were followed
  only for parameter/root initialization or interpretation candidates.

Existing PSB scene/XHW/scheduler/buffer sources, SGX535 definitions, preserved
DDX/DRI references and the public SCH document were also checked narrowly.
No selected normal-root successor schema was found in those sources. Other
generation region-header event names were not promoted into rev121 fields.
The excluded current entry/reporting implementation and restricted material
were not accessed. VISTEST and Vita investigations were not reopened.

## Provenance of 64 and 12

**CONFIRMED CPU arithmetic**, Xpsb scene-info ELF `0x3a51–0x3ba9`, selected
small-scene/default rev121 branch:

```text
uX = ceil(32/16) = 2; uY = ceil(32/16) = 2
aX = align4(ceil(uX/2)) = 4; aY = align4(ceil(uY/2)) = 4
fX = 2*aX = 8; fY = 2*aY = 8
N = fX*fY = 64
rootOffset = align4096(4*nextPow2(max(fX,fY))^2) = 0x1000
followingOffset = rootOffset + 12*N = 0x1300
```

`64` is the product of scaled allocation axes, not a literal number of
regions or an observed64-iteration root walk. `12` is assembled at
`0x3ba0–0x3ba3` by `3*N`, then a scale4 address addition. It is an actual
allocation factor. There is **no explicit stride=12 hardware register setting**
or individual-unit dereference in the normal setup inspected.

**CONFIRMED whole-slice selection**, ELF `0x4519–0x4538`, then
`0x41f0–0x41fc`:

```text
selection = cookie[15] & 15          # exceptional/partial branch only
sliceOffset = selection * cookie[1] * 12
rasterAddress = S + cookie[7] + sliceOffset
```

For this scene `cookie[1]=16`, hence192-byte steps. Four steps' worth of
reservation equals768 bytes; the small-scene exceptional host loop has four
slots. The initial selected raster branch instead uses sliceOffset0.
This supports a structured reservation, **INFERRED** grouping in twelve-byte
units, but **UNKNOWN** individual hardware stride/type. A four-group/sixteen-
position organization is an arithmetic-compatible hypothesis, not proof of
regions, tiles, macrotiles or64 independently addressable headers. The derived
screen grid and the memory allocation grid must not be identified by name.

TA `0x21c` and raster `0x408` receive the same initial GPU address. Dimensions
and controls accompany it; no software mask/shift is applied to this base
handoff. This does not reveal implicitly hardwired record stride or subpointer
encoding inside ISP.

## Access inventory and rejected lookalikes

| Access / evidence | Object actually accessed | Effect on successor proof |
| --- | --- | --- |
| Xpsb`0x3a40`, multiplication at`0x3ba0` | Host scene-info cookie and returned allocation size | Reservation bounds only; no payload write/read |
| `0x40b9`, `0x4642` store MMIO`0x21c` | TA base register, value derived from S | Address selection; no root-word interpretation |
| `0x41fc` store MMIO`0x408`; exceptional offset at`0x452f` | Raster base register, S-derived address | Whole-slice selection only |
| PSB scene clear routines, lines30–71; reused-scene clear at220–228 | First S page only | Do not initialize repeated twelve-byte root headers |
| Fresh historical TTM population | New S backing pages, including root reservation | Establishes initial zero backing, not EMPTY encoding |
| Current owner `clear_highpage`, lines316–323 | All newly owned pages | Same zero-allocation fact; no unit-specific tags |
| Xpsb`0x28f3` / pool count64 in Init | Host BO descriptors / CPU batch pools | Neither is the normal S reservation |
| `0x7395`, `0x7656`, `0x78fd` twelve-byte advances | CPU background/parameter-prefix or host anchor construction | Different producer/object; no normal-root unit access |
| `0x7638`, `0x7844`, `0x7d6b` apparent`+0x21c` accesses | CPU encoder context's saved anchor field | Host field offset coincides with MMIO number; not a TA-register/root read |
| `0x9637/96d8`, `0xa0da/a184/a18f`, `0xa936` | Host BO validation/linked-list records passed toward DRM submission | Pointer links/descriptor prefixes, not TA-generated payload |
| CPU encoder`0x7d45/7d74/7df6` | CPU flat-list flags, relocation mask and terminator | Known alternative raster input; normal interpretation unproved |
| DRI input finalization`0x2a46b` | Input TA command stream's terminator | Cannot establish output ROOT EMPTY or LIST END |
| EMGD parameter-buffer management / kick | PB descriptor ownership, command/sync references | No normal S-unit encoder/parser |
| EMGD temporary region-header handle | Feature-gated separate resource handle | No payload layout or relationship to this S established |

The previously excluded communication-BO loop contributes **no** TA-output
evidence and is not reused as a parser. No inspected payload access with
`S+0x1000+12*j`, a three-dword load/store, or a64-iteration S-root traversal was
identified. Search results and object attribution are retained in the audit;
this is exhaustion of identified available permitted candidates, not a claim
about unavailable proprietary hardware specifications.

## Pre-TA bytes: new allocation fact, no EMPTY conclusion

The additional historical source closes an allocation detail. New kernel BOs
take `drm_bo_add_ttm`→`drm_ttm_init`; the page-pointer table begins zeroed.
`drm_ttm_populate` obtains every page through `drm_ttm_get_page`; absent pages
use `alloc_page(GFP_KERNEL | __GFP_ZERO | GFP_DMA32)`
(`public-psb/drm_bo.c:143–149`; `drm_ttm.c:53–63,81–91,218–233,272–289`).
PSB's populate callback retains those pages, and its bind path maps them
without encoding root words
([buffer source](../poulsbo-data/PSB_psb_buffer_c.txt#L282), lines322–367).

**CONFIRMED in that source family:** fresh kernel backing starts zero,
including the root page. Exact historical built pairing remains unproved.
**CONFIRMED current policy:** the permitted owner explicitly clears all pages
([owner](../../kernel/sgx535_frozen/gma500_bo_owner.c#L316)). Neither is a fresh
hardware observation or a GPU→CPU publication claim.

For any *hypothetical* twelve-byte grouping wholly inside the reservation:

| Position | At fresh CPU-backed allocation | Writer / purpose | Immediately before TA start |
| --- | --- | --- | --- |
| `+0` DWORD |0 | Historical zero-page allocation / current page clear; deterministic backing | UNKNOWN after intervening hardware setup |
| `+4` DWORD |0 | Same | UNKNOWN |
| `+8` DWORD |0 | Same | UNKNOWN |

There is no first/middle/final distinction in that CPU zeroing. No pointer
seed, root sentinel, valid tag or unit-specific purpose is established.
The grouping is not a proved header declaration.

The current [TA plan](../../tools/psb-dri-re/frozen_kernel_contract.c#L545)
contains DPM-control/state and other hardware setup before the final TA kick.
Its wait mask decomposes, using the retained SGX535 names, as:

```text
0x100a40 = OTPM_INV(0x100000) | TPC_CLEAR(0x800)
         | DPM_CONTROL_CLEAR(0x200) | DPM_STATE_CLEAR(0x40)
```

This is **CONFIRMED register/event-name correlation**, not proof that any
specific root dword is written to a particular value. In particular, these
events do not provide an exported post-setup root image or an EMPTY predicate.
No new reset or completion clearing is performed by this task.

On historical reuse, the explicit clear still covers only`S+0..1000`; the
following reservation is not CPU-cleared. Therefore fresh zero allocation
cannot be silently generalized into a per-scene empty-root initialization
contract. Hardware may replace/gate entries, but its byte-level rule is not
exposed by the inspected code. **TA mutation of each individual dword remains
UNKNOWN**; receiving the base and later completing TA do not prove it.

## State and successor semantics

| Required fact | Finding | Why no promotion |
| --- | --- | --- |
| ROOT EMPTY | UNKNOWN | No consumer predicate accepting zero, sentinel or flag is exposed |
| ROOT POPULATED | UNKNOWN | Cannot infer it by negating an unproved EMPTY rule |
| First-reference word/width | UNKNOWN | No normal-root word is extracted as a raster reference |
| Absolute/relative address, base, mask, shift, flags, alignment | UNKNOWN | Q/P/S address setup constrains owned objects, not the root subpointer transformation |
| Next reference | UNKNOWN | No indexed/list advance established from payload |
| BLOCK CONTINUATION | UNKNOWN | Whole-slice selection is not a root/list chaining rule |
| LIST END | UNKNOWN for normal output | CPU flat-list end`0xc0000000` belongs to another organization |
| PRIMITIVE TERMINATOR | UNKNOWN | No primitive-level normal-output decoding is reached |

EMGD `sgxinfo.h:104–106` and `sgxinit.c:216–218` name/retain
`hKernelTmpRgnHeaderMemInfo` under `SGX_FEATURE_OVERLAPPED_SPM`. Neither
constructs its words; neither ties that feature/resource to this normal scene
reservation. `sgx_mkif_km.h:336–355` exchanges sizes of render/PB structures,
not their root payload definitions. These additional primary paths do not
provide the missing predicate or address transform. A different-generation
`TE_RGNHDR_INIT_COMPLETE` event likewise cannot define rev121 root fields.

Candidate hypotheses tested against the evidence:

- **H_ZERO_EMPTY:** three zero words mean EMPTY. Compatible with fresh CPU
  allocation, but unsupported as hardware interpretation, especially across
  reuse/setup. Rejected as a qualified rule, not disproven as a possibility.
- **H_FOUR_GROUPS:** four selections contain sixteen twelve-byte entries each.
  Compatible with arithmetic and exceptional selection; individual entries,
  spatial meaning and field grammar remain unproved. No tile layout is assigned.
- **H_HEAD_TAIL_META:** three words encode head/tail/control. No normal-path
  access distinguishes this from another descriptor organization. No field
  order, mask or pointer transformation is assigned.
- **H_CPU_LIST:** treat the initial words as the CPU encoder's two-word
  reference and its following word. Shared root register is insufficient;
  root envelope/control/address construction differ and no normal parser
  establishes that conversion. Rejected as a qualified rule.

No masks or shifts are invented to fill those hypotheses.

## Minimal bounded schema and outcome

```text
RESERVATION = bytes[S+0x1000 : S+0x1300]  # software bounds CONFIRMED
GROUPING_12[j] = bytes[12*j : 12*j+12]   # hypothetical, not hardware type

consume_normal_root(initial_position, selected_controls)
  -> EMPTY | SUCCESSOR(address, type) | ...   # predicate/transform UNKNOWN
```

This is an explicit unresolved schema, **not** a parser specification.
`ROOT_UNIT := EMPTY | POPULATED | CONTINUATION` cannot be asserted as the
actual hardware grammar. Even the state partition and unit width are unproved.
There is therefore no first normal reference to compare with the CPU flat
encoding: convergence **UNKNOWN**, not NO_CONVERGENCE. Neither incompatibility
nor direct/convertible equivalence is established.

**Smallest next justified action:** obtain permitted primary rev121 evidence
of the normal ISP root consumer's first-position state predicate and successor
address transform, or a same-mode producer with an independently established
consumer contract. No identified current source supplies it. More opaque bytes
alone would not establish that interpretation. Do not proceed to primitive
decoding, publication or a live capture on these findings.

## Verification and preserved state

**38/38 PASS** CPU/source reference checks cover allocation/selection arithmetic,
event-mask decomposition, primary-source Git blobs/licenses, historical TTM
zero-page lineage and relevant current initialization/setup statements. They
are **not GPU qualification**. No decoder tests, implementation tests, build,
hardware access or historical executable execution occurred.
The final consistency receipt verifies document references, all68 sealed
FIRE #2 file hashes, pre-existing implementation/artifact bytes and empty index.

Changed repository files are only this document, its structured finding and
the latest handoff section. Driver Build ID
`9d0b5b2fdb7d9881f2828f43fb89253176c38817`, observer Build ID
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`, and image SHA-256
`ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`
remain unchanged. No new candidate exists. FIRE #2 still cannot resolve
coverage retrospectively; completion/provenance and its4096 all-zero bytes
remain unchanged findings.

GPU→CPU publication remains a separate future boundary. Another live capture
is not justified unless the independent format and publication requirements
later close. FIRE #3 remains **UNAUTHORIZED**; `sgx_execution_authorized=false`;
task SGX invocations=0; hardware interactions=0; triangle **NOT ESTABLISHED**.
