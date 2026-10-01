# Selected PDS launch-state envelope — Route C

2026-09-27; P7H-038/039. **Coverage remains OPEN.** The new result is a
parameterized CPU launch record and a precise boundary at its hardware
interpretation, not another instruction decoder. Route C remains selected.
Historical provider identity is not a prerequisite. Gate B BLOCKED, whitelist
`[]`, hardware UNVERIFIED. Nothing here authorizes device operations.

Read alongside [the clean-room contract](cleanroom-backing-contract.md),
[the state inventory](pds-launch-state.csv), [the exact CPU image](cleanroom-pds-image.csv),
and [the frozen fragment checkpoint](frozen-fragment-link-progress.md).
The existing 16 image regressions, first-use model and historical conclusions
are retained. No corpus, Experiment #43, provider survey or PDF search was rerun.

## What the CPU actually supplies

All addresses below are **ELF VAs** in the retained DRI unless marked Xpsb.
Ghidra addresses add `0x10000`. Analysis uses the already retained bounded
exports, with targeted x86 disassembly of the launch packing branch.

1. `0x25970` emits the selected primary, returns descriptor `{BO, offset,
   committed_bytes=56, data_dwords=12, result_bc=0, linked_temp_count=0}`.
   `0x283d8` copies its six words into scene `+0x430..+0x444`.
2. `0x28559` passes **scene+0x430** to `0x39ea0`. The selected branch adds
   **0x18** before calling `0x3faa3`: the secondary descriptor is therefore
   **scene+0x448**, not an overwrite of the primary. It records four bytes of
   `0xaf000000` and zeros descriptor `+0xc/+0x10/+0x14`.
3. `0x29251` passes scene+0x414 to `0x28fff` (decompiler `int * +0x105`).
   The present-bit `0x40` group covers those primary/secondary descriptors.
   On a state change, `0x28fff` calls `0x287ff` to serialize the group.
4. Relative to this state pointer, primary descriptor fields are `+1c` BO,
   `+20` offset, `+24` committed length, `+28` data count, `+2c` result_bc,
   `+30` linked temporary count. Secondary begins at `+34`; its data count
   is `+40` and following resource count `+44`.
5. In the nonnull-primary branch of `0x287ff`, the three serialized words are:

   ```text
   R(A) = (A >> 4) & 0x00ffffff
   word 0 = R(resolved secondary BO address + offset)
   word 1 = 0x00030000
   word 2 = R(resolved primary BO address + offset) | 0x0c000000
   ```

   These are **CPU relocation/packing formulas**, parameterized by valid
   resolved addresses. They are not a hardware launch simulator. The selected
   pool already aligns these references; the formula discards four low bits.
   Backend base selection, valid address windows and GPU visibility remain
   obligations, not outcomes of the mask.

The primary count is packed as `(data_dwords & 0xfc) << 24`: CPU count bits
7:2 become output bits 31:26. For count12 this six-bit field is **3**, producing
`0x0c000000`. Low count bits are discarded; the selected CPU prefix is padded
to four-dword multiples. No count for DS0 and DS1 separately is emitted by
this branch. The committed length56 is **not** used in these three words.
It is not an architectural access bound.

For word1, the selected `state+2c/+30/+44` are zero. At ELF `0x28960..0x28a19`,
the two local quantities remain 4 and32; `(4-1)<<16` contributes `0x30000`,
`(32&31)<<25` contributes zero, and the other terms are zero. Stores/masks at
`0x28a89..0x28ac9` confirm this independently of decompiler type guesses.
The two relocation call sites are `0x28a84` and `0x28b39` (`0x37854`).
This control word is outside the 56-byte image; it is deterministic CPU
control metadata for the selected descriptors, not a newly discovered PDS
source operand.

`0x287ff` subsequently builds a **different state-upload program**. Its output
is referenced by `0x3b73b` in the TA stream. That program and the zero-input
fragment secondary are not proved to initialize the primary's PDS temporaries.
Their sequencing/CPU descriptors do not establish internal-store clearing.

## Mapping the image without inventing a preload ABI

Xpsb `Xpsb_pds_get_num_constants` at ELF `0x7470` and the already recovered
DRI construction support this **CPU byte placement**:

```text
slot(bank, i) = 4 * ((i & 7) + 16*(i >> 3) + 8*bank)
raw_extent(n0,n1) = max(last populated CPU position in either bank + 1 dword)
selected: n0=2, n1=1 -> raw extent9 -> rounded extent12 dwords
```

This is an eight-dword block interleave in CPU storage. It does not determine
whether hardware uses physically separate banks, views, or a different preload
mechanism. The [14-row image table](cleanroom-pds-image.csv) and the individual
`cpu_00..cpu_34` [inventory rows](pds-launch-state.csv) give every dword:

| CPU offsets | CPU layout role | Restricted CPU contents |
| --- | --- | --- |
| +00, +04 | DS0 labels0,1 | U, zero; selected builder fields |
| +08, +0c, +10, +14, +18, +1c | DS0 labels2..7 | Explicit clean-room zeros |
| +20 | DS1 label0 | `0x20`; selected builder field |
| +24, +28, +2c | DS1 labels1..3 | Explicit clean-room zeros |
| +30, +34 | Two CPU instruction words | `0x07000345`, `0xaf000000` |

U remains an explicit **resolved USE relocation word**, not an arbitrary
placeholder admitted as a valid GPU address. All56 CPU bytes are defined for U.
Only48 of those bytes precede the code. Initializing more BO bytes does not
by itself initialize any additional on-chip store.

An adversarial boundary example: extending the **CPU placement formula** gives
DS1 label4 at +30 and DS0 label8 at +40. The former is code in this selected
record, the latter lies beyond its committed56 bytes. Neither calculation
establishes a hardware alias, an actual selected read, or permission to append
data without changing the count/code layout. A BO size is not a DS bound.

## First use, temporary state and implicit state

The strongest first-use facts are CPU-side: initialization precedes the
builder-equivalent writes and publication; the first two instruction bytes are
fixed. There is no preceding instruction **inside this primary program**.
This does not establish its architectural entry-PC calculation. In particular:

| Candidate input/domain | What constrains it | Result at hardware first use |
| --- | --- | --- |
| DS0/DS1 | CPU bank layout and aggregate count12; inferred triplet reads | Preload map, unused-entry validity and selected access domain UNKNOWN |
| Temporary store questions0/1 | No selected temporary initialization recovered | TEMP_UNKNOWN; neither number of physical banks nor read-before-write eligibility is established |
| Task/control data at +00/+04/+20 | Builder/relocation provenance | Deterministic CPU words; selected architectural consumption remains INFERRED/UNKNOWN |
| Packed launch references/resource fields | Three-word CPU record above | Deterministic when addresses supplied; complete hardware interpretation UNKNOWN |
| Linked USE address/program | U and linked suffix `00 00 00 00 40 01 04 f8` | Downstream task target; not evidence that PDS reads USE memory as an extra input |
| DOUT-related control | Public register name plus prior program-role inference | No selected opcode/source assignment; do not turn an output destination into an uninitialized input |
| Inherited/implicit PDS state | Existing unexcluded implicit-state hypothesis | No particular extra register, persistent value or selected read established |
| `0xaf000000` | CPU-final/standalone grammar | No proof of zero sources or of consuming a value defined by the first instruction |

The `temp_count` assertion in `0x25970` compares compiler/linker **USE/USSE**
quantities (`result->temp_count <= cpost->temp_count`). The linked `+0x74`
count flows into the task words and returned descriptor+0x14. Its frozen zero
value is not a PDS temporary reset or evidence of zero PDS temporary capacity.
Thus neither TEMP_NOT_READ, TEMP_WRITTEN_BEFORE_READ nor TEMP_ZERO_INITIALIZED
can be selected from that count. No new field encoding is assigned here.

Rows temp0/temp1 are explicit audit questions requested for coverage; their
names are **not** assertions that two physical temporary banks exist. The
implicit row preserves the existing countermodel without asserting an actual
external read. No separate texture/DMA/external-memory input has been established
for these two selected words; generic capabilities of other programs are not
added as selected-path facts. Conversely, an UNKNOWN rule is not proof that
such sources are absent. There is no evidence here of a required particular
nonzero value in a historical hole.

## Targeted adversarial checks at the launch boundary

The legitimately retained SGX535 register material was inspected for a preload
or reset rule, rather than expanding the PDS corpus:

- [H535 snapshot](../../archaeology-data/H535.txt), lines375–416, supplies
  `PDS_EXEC_BASE`, `PDS_INV0..3`, `PDS_INV_CSC`, `PDS_PC_BASE` addresses/masks.
  It supplies no launch count-to-DS map, temporary clear operation or selected
  source-validity rule. The acronym `PC_BASE` is **not decoded as entry PC**.
- [Permissively licensed EMGD SGX535 header](../../poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt),
  lines324–358, also exposes `DOUT_TIMEOUT_DISABLE`. A timeout/invalidation name
  does not prove store initialization, per-invocation clearing or source count.
- The existing candidate kernel's `psb_drv.c` programs `PDS_EXEC_BASE` from
  `PSB_MEM_PDS_START=0x20000000`; `psb_buffer.c` uses that aperture offset;
  `psb_sgx.c` has PDS-relative relocation arithmetic. These constrain addresses,
  not internal-store definedness. No provider research was repeated, and this
  candidate is not silently chosen as the Route C backend.
- The nearby public `SGX_FEATURE_PDS_DATA_INTERLEAVE_2DWORDS` declaration in
  `omap5-sgx-ddk-linux/.../sgxfeaturedefs.h` is inside **SGX545**'s conditional.
  It cannot replace the selected SGX535 CPU layout or prove hardware preloading.
  No later-core instruction definitions were used.

These checks exclude three false shortcuts: resource count zero as PDS reset,
cache invalidation as store clearing, and cross-core interleaving as SGX535 ABI.
No inspected evidence establishes prior-launch persistence either. Persistence
and temporary read-before-write remain **unknown alternatives**, not findings.

## One next rule: L12, selected launch initialized-state domain

The missing rule is at the **hardware interpretation of the packed primary
data-count field**, not at BO allocation:

> For a primary reference carrying count field3 (`0x0c000000`, CPU data count12),
> with code laid out at +0x30, what launch-local state becomes defined, and is
> any state outside that defined domain eligible for a pre-definition read by
> this selected two-word program?

An adequate **L12 contract** must identify the DS preload domain and treatment
of other source-eligible state (excluded, initialized from deterministic launch
inputs, or first defined before consumption). This is one launch isolation/
definedness obligation with explicit subdomains, not a claim that a single
unknown bit explains all of them. The present CPU count only identifies packed
bits31:26; it does not establish their architectural rule. No legitimate evidence
identifies a more specific selected temporary-source bit, so none is invented.

Even granting the favorable interpretation that12 CPU dwords are preloaded,
the software observations do not distinguish a launch confined to that initialized
domain from one leaving some source-eligible state undefined. This is an
identifiability limit of the inspected launch producers, not a newly observed
out-of-range access. Exact instruction reads remain unnecessary **if L12 can
be proved by a launch contract instead**. Resolving the DS mapping alone must
not silently close the temporary/implicit subdomains.

The conservative set is therefore **PARTIALLY BOUNDED**: CPU data/code and
parameterized controls are enumerated; the architectural completion of that
set is UNKNOWN. Calling it “unbounded” or “could read anything” would overstate
the evidence. First-use no-overlap (P7H-030) remains CONFIRMED within its model.
GPU contents preservation remains CONDITIONAL, independently of L12.

## Decision matrix

| Question | Current result |
| --- | --- |
| 56-byte CPU PDS image | DETERMINISTIC for supplied resolved U |
| Committed data-state coverage | CONFIRMED for CPU prefix; hardware coverage UNKNOWN |
| DS0 launch coverage | UNKNOWN |
| DS1 launch coverage | UNKNOWN |
| Temporary-state coverage | UNKNOWN |
| Implicit launch-state coverage | UNKNOWN |
| Possible selected-program input set | PARTIALLY BOUNDED |
| All bounded possible inputs deterministic | CONDITIONAL: CPU bytes/controls defined with supplied relocations; hardware provenance incomplete |
| Contents preservation to GPU | CONDITIONAL; concrete backend still required |
| `0x07000345` exact read set | UNKNOWN |
| `0xaf000000` semantics | UNKNOWN |
| PDS ISA | UNKNOWN |

**Does coverage remove the exact-read-set implementation dependency? CONDITIONAL
on L12; it has not been removed.** No evidence proves exact decoding is the only
possible solution. FG-02 remains OPEN; its CPU launch record is advanced above,
but no complete frozen draw or backend has been established. Historical provider
identity remains UNKNOWN/CONDITIONAL and historical recycled holes potentially
stale. All nine Route C holes remain **CPU ZERO_CONFIRMED**, not GPU-launch-state
ZERO_CONFIRMED.

## Reproducibility and proof limits

[pds_launch_state_check.py](../../../tools/psb-dri-re/pds_launch_state_check.py)
checks inventory completeness **against this explicit model**, all14 CPU words
against the existing image tool, provenance/order, and scoped packing/layout
arithmetic. It cannot certify that the inventory is architecturally exhaustive.
L12 is explicitly unproved; editing a CSV status cannot manufacture a proof.
Default exit0 means a faithful OPEN inventory. `--require-closed` must fail.

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tools/psb-dri-re -p test_cleanroom_pds_image.py -v
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tools/psb-dri-re -p test_pds_launch_state_check.py -v
PYTHONDONTWRITEBYTECODE=1 python3 tools/psb-dri-re/pds_launch_state_check.py
PYTHONDONTWRITEBYTECODE=1 python3 tools/psb-dri-re/pds_launch_state_check.py --require-closed
llvm-objdump -d --x86-asm-syntax=intel --start-address=0x288f5 --stop-address=0x28b3e references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/dri/psb_dri.so
```

The last Python command's expected exit status is1. Ten new tests cover omitted,
duplicate and newly uncovered input rows; unsupported status promotion; missing
provenance; wrong initialization order; omitted CPU initialization; boundary
layout arithmetic; and the selected packed-word fixture. Sixteen existing image
tests are retained. A synthetic address fixture exercises masks only.

Retained export inputs: `/tmp/sgx535-psb-decompile/{00025970,000283d8,00028559,
000287ff,00028fff,00029251,00039ea0,0003faa3,0003b73b}.txt` and Xpsb
`/tmp/sgx535-xpsb-decompile/00007470.txt`. These are **ephemeral**, not required
runtime inputs to the checker. Recreate with the existing
[bounded Ghidra export procedure](../../../tools/psb-dri-re/README.md); ELF VAs,
descriptor offsets, formulas and limitations are preserved above. New driver
code contains no copied decompiled implementation. No new material was acquired.

Header snapshot SHA-256 values: H535
`cb2a3e9119d65b421a16744137b1114b0ebd6982cb27549b4fce1d8405bd41c3`;
EMGD SGX535
`634fbb74645d373e0c60ad5e01194fa96e91bb6b26d60dbc35a71a2eda49d12c`.
These existing public snapshots support only the limited register observations
above, not the missing L12 guarantee.

## Validation recorded for this pass

All16 existing image tests and all10 new launch-model tests pass. Both CSV/image
checkers pass their integrity mode; the closure-required CLI fails with L12 as
expected. Python syntax compilation passes. All39 Phase7/evidence CSVs have
consistent row widths and unique header columns. P7H-038/039 each have one new
evidence row; referenced IDs exist. Pre-existing multi-row evidence IDs were
preserved rather than misclassified as newly introduced duplicates.

475 local Markdown links across28 changed/untracked reports resolve. The two
retained ELF SHA-256 values match the authoritative values in the handoff.
`git diff --check` and new-file whitespace checks pass after correcting CRLF
introduced in the two appended evidence rows. `git status --short -- references`
and `git diff --cached --name-only` are empty. Prior uncommitted work remains
in place; nothing was staged or committed. No hardware was accessed.

## Middle-word field clarification (P7H-040)

[The narrow count analysis](pds-launch-count.md) distinguishes two unrelated
encoded3 values: middle-word `(Q-1)<<16` is resource-derived (Q=4 selected),
whereas primary-reference `(data_dwords&0xfc)<<24` is the CPU data-size field.
Q changes with USE temporary/attribute demand independently of that size input.
Its architectural unit remains UNKNOWN; resource-group interpretation INFERRED.
Neither field establishes source eligibility. L12 remains OPEN: whether the
selected launch makes state outside its initialized48-byte-prefix domain
unavailable until defined. No outside read is asserted. P7H-036–039 remain
unchanged; all26 earlier tests are retained, and GPU preservation stays
CONDITIONAL. Do not repeat the middle-word count survey to answer L12.

## Source availability and evidence scope (P7H-041)

[The selected availability analysis](pds-selected-source-availability.md)
distinguishes source eligibility from CPU storage, control metadata and output
destinations. The existing public SGX535 description supports temporary-store
class plausibility but supplies no selected access/reset rule. No actual outside
read or cross-launch persistent source was established. The next discriminator
is the complete pre-definition source domain of the two selected forms, confined
to deterministically established launch state. Naming a particular unknown
selector bit or asserting a temporary hazard is not supported.

The extended24-row inventory retains old provenance and adds role/availability
columns. Six new checker regressions preserve the existing36; source blockers
and control obligations are now derived separately. Inventory completeness is
not architectural completeness. L12/FG-02 stay OPEN; GPU preservation remains
CONDITIONAL; Gate B BLOCKED; whitelist []; hardware UNVERIFIED.
