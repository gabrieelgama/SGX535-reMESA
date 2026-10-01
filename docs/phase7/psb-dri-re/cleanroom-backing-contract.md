# Route C: restricted clean-room PDS backing contract

P7H-060 update: the [strong contract review](cleanroom-final-closure.md)
enforces full user-BO zeroing and strict publication/bootstrap abort policy in
the host model. C5/C9 device visibility and C10 architectural source eligibility
remain unproved. The historical SGX535 PDS PDF is optional future evidence,
not a prerequisite for Route C. No historical conclusion below changes.

Project decision, 2026-09-27: select Route C. Historical target-to-provider
identity is **not a prerequisite** for the new implementation. It remains
UNKNOWN/CONDITIONAL as a historical question. The historical recycled holes
remain potentially stale; neither instruction nor the PDS ISA is decoded.

This document separates a constructible CPU image, a provider's required
contents-preservation contract, and the PDS launch's required read containment.
An executable byte-image checker is not an implemented kernel provider.

## Work plan and scope

This is a static proof/checker task authorized by the Route C decision, with
no device operations, new provider survey, historical ELF execution, staging,
or commits. Gate B remains BLOCKED; whitelist `[]`; hardware UNVERIFIED.

- [x] Implement tests first in
  `tools/psb-dri-re/test_cleanroom_pds_image.py`: exact independent byte fixture,
  all-byte definedness, dirty input, omitted initialization, corrupted fields,
  relocation parameter, preservation and containment counterexamples.
- [x] Implement `tools/psb-dri-re/cleanroom_pds_image.py`: little-endian 56-byte
  CPU construction with byte provenance; strict verification; snapshot-copy
  comparison and an explicit read-containment obligation. No decoder or GPU API.
- [x] Record the 14 dwords and provider transitions in CSV; formalize C1–C10,
  adversarial cases and the exact remaining premise without promoting a model
  into hardware evidence.
- [x] Update current decision/index/handoff alongside historical conclusions;
  run tests, hashes, schemas, IDs, links and working-tree integrity checks.

The image constructor accepts the **resolved relocation word U** as an explicit
32-bit input. It does not invent a GPU address, choose a USE base register, or
treat `0x67676767` as an already resolved relocation. The reference byte image
is parameterized by U; a real backend must supply its valid resolved value.

The contract will require initialization of the **whole backing payload before
any first-scene writer**, not a 56-byte memset into a dirty pool. This controls
padding and neighboring unallocated backing bytes without claiming that BO
bounds constrain internal PDS data-store or temporary-state addressing.

## Decision: CPU construction proved; implementation closure CONDITIONAL

P7H-036 establishes the parameterized, fully initialized **CPU image** and
records the Route C decision. P7H-037 identifies the next irreducible proof
obligation: **complete deterministic PDS launch-state coverage**. Even granting
an ideal provider that perfectly preserves every backing byte, existing evidence
does not establish that every state location consumed by these instructions is
loaded from that backing or another deterministic launch input.

This is not a newly observed out-of-bounds read. It is the absence of a proven
superset. No ISA search, corpus expansion, Experiment #43 repetition or provider
survey was performed. FG-02 remains OPEN. A concrete SGX provider adapter is
also not implemented by this Python proof aid; its refinement obligations below
are explicit, rather than being silently marked complete.

## Exact invariants and their proof scope

Let `B` be an exclusively owned backing payload of `N` bytes, `S` the selected
slot-relative offset, `I(U)` the 56-byte CPU image, and `L` the complete PDS
state at first consumption. Let `R` be the still-unknown set of state locations
consumed by the selected program. These are different domains: a byte offset in
`B` is not automatically an internal DS or temporary-register index in `L`.

| Clause | Required invariant | Present result |
| --- | --- | --- |
| C1 | Before scene writers, allocate/own the whole payload; no concurrent CPU/GPU writer, imported dirty alias or unresolved prior fence. Eagerly establish every page. | Required new-backend policy. The host initializer owns a caller-provided bytearray; it is not a kernel BO allocator. |
| C2 | For every `i` in `[0,N)`, write `B[i]=0` before any scene writer, including padding and unused slots. Failure at any page aborts publication. | CONFIRMED for supplied host payload by `initialize_payload`; not a physical GPU-storage claim. |
| C3 | At `S`, write `I(U)` using little-endian dwords; U is the resolved linked-USE word; known stores are `+04=0`, `+20=0x20`, `+30=0x07000345`, `+34=0xaf000000`. | CONFIRMED parameterized CPU construction, P7H-014/036. U's address/base-register validity is a backend relocation obligation. |
| C4 | Every byte in `[S,S+56)` is defined. The nine non-builder dwords remain zero; no native struct padding enters the image. | CONFIRMED CPU bytes and explicit initialization provenance. |
| C5 | All CPU writes, including final relocations, precede device ownership; the selected mapping's cache maintenance and ordering complete before first consumption. No later CPU writer. | Backend refinement required. A Python assignment, host digest, fence-shaped object or compiler barrier does not prove this. |
| C6 | Validation/binding retains the initialized payload and valid address association. Payload metadata is out of band. Pin/retain backing through first-use completion. | Required contract; P7H-031 supplies a public source-level feasibility witness, not a new adapter implementation. |
| C7 | Exclude migration/replacement before completion in the restricted design. If later permitted, copy all N bytes and re-establish C5/C6 before publication. | Exclusion policy specified. Whole-snapshot checker detects incomplete/corrupt copies; cannot establish hardware coherence. |
| C8 | Preserve existing slot geometry, successful first-scene allocation order, reservation/commit rules, and no-retry scope. Initialization occurs before allocation #1; no new payload allocations are inserted. | Existing P7H-030 no-overlap proof reused: S is `0x160` or `0x1c0`, including nested clear-secondary. No new model needed under these constraints. |
| C9 | For each payload byte needed by launch, GPU address `A+i` observes the final `B[i]`; A includes the selected BO/slot/suballocation offset once. Retain mapping and contents until completion. | Required concrete backend mapping/ownership proof; modern GEM alone does not establish it. |
| C10 | There is an established deterministic subset `D` of launch state L with `R ⊆ D`; each member of D is supplied by B or another established initialized launch input. Exact R is unnecessary. | **UNKNOWN.** Neither 56-byte commit nor the emitted data-size field proves this inclusion. This is the next inference barrier, even assuming C1–C9. |

The [transition table](cleanroom-backing-transitions.csv) scopes each stage.
Historical provider membership is not among these clauses.

## Initialization choice and exact image

Choose explicit whole-payload initialization under exclusive CPU ownership.
Do not rely on allocation names, first allocation, Linux's security intent or
the historical `__GFP_ZERO` flag. If retaining the already modeled pool geometry,
initialize `30 × 0x20000 = 0x3c0000` or `30 × 0x40000 = 0x780000` bytes before
any first-scene writes. This is a clean-room policy; it does not claim that the
old userspace pool performed this clear. P7H-025 locates allocator metadata in
separate host descriptors. A concrete new backend must preserve that separation.

The chosen ordering avoids a dangerous shortcut: **do not zero the 400-byte
reservation after earlier scene records have been produced**. A reservation is
not a permanently owned committed extent. Clear the backing once before all
writers, then write only each producer's committed bytes/relocation targets.
No selected instruction or linked-USSE bytes are cleared afterward.

The [14-dword image table](cleanroom-pds-image.csv) and
[per-dword historical/clean-room provenance](pds-buffer-byte-provenance.csv)
give the complete parameterized image:

```text
offset    value (u32 little-endian)
00        U   (resolved linked-USE relocation word)
04        00000000
08        00000000  clean-room ZERO_INIT
0c        00000000  clean-room ZERO_INIT
10        00000000  clean-room ZERO_INIT
14        00000000  clean-room ZERO_INIT
18        00000000  clean-room ZERO_INIT
1c        00000000  clean-room ZERO_INIT
20        00000020
24        00000000  clean-room ZERO_INIT
28        00000000  clean-room ZERO_INIT
2c        00000000  clean-room ZERO_INIT
30        07000345
34        af000000
```

The established [relocation relationships](frozen-draw-objects.md) constrain U:
the first masked USE_REG merge uses mask `0x0f`, followed by USE_OFFSET merges
with shifts right15/left4, mask `0xf0`, and right4/left8, mask `0x7ff00`.
Each offset merge preserves the previous destination result. The serializer
requires the final U instead of duplicating the partially instantiated USE
base-assignment provider. Its arbitrary test U is **not a legal-address claim**.
The historical `0x67676767` placeholder is not automatically a resolved input.

The linked USSE suffix remains `00 00 00 00 40 01 04 f8`; the secondary CPU
record remains `0xaf000000`. Neither their semantics nor their complete launch
contexts are changed by constructing this primary CPU image.

## What zeroing resolves, and what it does not

For every fixed U and for any read subset contained in this deterministic
image, both M1 and the existing in-image M2 receive defined bytes. In particular,
adding any of the nine holes as an input no longer introduces stale data.
Thus **the nine-hole CPU-byte ambiguity is removed by construction**. No proof
that those holes are padding or unconsumed is needed for this restricted claim.

The [existing hypothesis table](pds-hypotheses.md) also retains M3: a possible
implicit/temp/data-store state read by the terminal instruction. M4 does not
establish a complete source-selector interpretation for the first instruction.
These were not ruled out by the provider audit. No new outside read is asserted
here; the old model set was never established as exhaustively limited to M1/M2
inside the nine holes.

The [lifecycle trace](pds-buffer-lifecycle.md) explicitly says:

- Commit56 advances the CPU cursor and descriptor length; it is not an
  established GPU memory-access bound.
- State `+0x24` contains that length, but the recovered packing branch does
  not use it. It packs `(data_dwords & 0xfc) << 24`, giving `0x0c000000` for12.
- No independently established hardware interpretation makes that count an
  exclusive source-access bound or guarantees initialization of every other
  DS/temporary location.

Consequently, increasing the zeroed region from56 to400 bytes, a whole slot,
or a whole BO does not establish C10. A BO aperture bounds backing addresses;
it does not by itself specify how PDS launch populates on-chip state. A guard
page that faults an unknown read would not prove a valid successful triangle.
Zeroing an entire guessed hardware state space would require a supported
initialization contract; it is not authorized by a CPU mapping.

### Adversarial proof, without a new ISA hypothesis

Assume the strongest favorable lower-layer premise: B is deterministic and
every byte is preserved perfectly to the GPU. Consider two possible launch
states equal on all established initialized inputs but differing on an
unconstrained implicit state value q. Existing CPU-emission evidence does not
observe whether q is read. A semantics completion depending only on B and one
depending on B plus q are indistinguishable to that evidence. No numeric index,
opcode name or real outside access is inferred from this logical countermodel.

If `R ⊆ D` were established, the second completion would be excluded and the
exact R would cease to matter for determinism. Without it, even the ideal BO
provider cannot prove the requested universal statement. This isolates C10
from cache, migration, installed provider identity and the nine in-image holes.

## Content preservation: policy versus implemented proof

The simplest restricted provider design forbids migration and reuse until the
first submission completes. It retains one page vector, binds that vector into
the SGX-visible address space, and performs the mapping-specific CPU-to-device
handoff after all writes/relocations. Allocation or residency pressure fails
the construction rather than replacing backing or falling back to stolen
memory. These are obligations of the **new** provider, not inferred historical
installation facts.

P7H-031/034 show that a publicly inspectable PSB implementation can keep the
same TTM page array across CPU faults, PDS binding and non-fixed moves. That is
a feasibility witness for the design, reused without another source survey.
The current Python code has no kernel mapping/cache operation or SGX adapter;
therefore clean-room contents preservation through actual first GPU use stays
CONDITIONAL. No self-attested `synced=True` flag is accepted as proof. The
snapshot equality checker is useful for detecting software partial-copy bugs,
but equality of two supplied host byte strings is not a device-visibility test.

This task stops at C10 as the **next irreducible inference premise**, not with
a claim that all lower-level implementation work is done. If launch coverage
is established, the concrete provider must still be implemented/reviewed
against C1–C9 before an end-to-end implementation-grade closure. That work does
not require proving which historical kernel happened to be deployed.

## Adversarial failure table

| Case | Handling and present proof |
| --- | --- |
| Recycled/dirty BO | Explicit whole-payload zero before any writer; host tests start dirty. Runtime exclusive ownership and backing completeness remain backend duties. |
| Lazy pages | Contract requires eager establishment and full successful initialization; any failure prevents publication. No untouched fault-created page may replace an initialized one. |
| Migration/replacement | Excluded in restricted provider; full-copy equality checked only as a future migration regression, not as migration permission. |
| Partial copy / changed padding | Checker compares entire supplied payload and rejects a correct56-byte prefix with a corrupt tail or different length. |
| CPU/GPU cache visibility | Requires concrete mapping-specific handoff; no-op `psb_invalidate_caches`, a language-level assignment or an unmap name is not proof. |
| Alignment / host padding | Exact little-endian stores, length56, per-byte definedness/provenance. Reject extra byte or incomplete vectors. |
| BO larger than image | Whole backing clear before all writers covers backing padding. It does not initialize unidentified on-chip state. |
| Nested allocations | Retain P7H-030 order and geometry, including clear-secondary; zeroing creates no allocation. Do not insert extra payload bookkeeping. |
| Wrong suballocation/address | S remains0x160/0x1c0; actual A plus S must be proved by the backend, with no double offset or address truncation. |
| Relocation order | U is explicit final input. Real relocation must finish before ownership transfer and touch only established destinations. |
| Unknown read beyond proven launch state | C10 remains UNKNOWN even with perfect lower-layer preservation. This is the next blocker. |
| An unknown hole required to be non-zero | None is specified by the existing selected CPU stores, selected relocation destinations or inspected source contracts. This bounded negative finding is not proof of unspecified ISA behavior. |

The historical-equivalence objection is therefore correctly narrowed: Route C
does **not** need equality to recycled stale values. The source-family fresh-zero
construction is also consistent with zero holes. The remaining issue is not a
known requirement for a particular non-zero hole; it is complete launch-state
coverage, followed by concrete provider conformance.

## Decision matrix

| Question | Status |
| --- | --- |
| Historical target provider identity | UNKNOWN / CONDITIONAL, not Route C prerequisite |
| Historical recycled holes | POTENTIALLY STALE |
| Historical first-use holes | CONDITIONAL |
| Clean-room backing initialization | CONFIRMED for explicit host payload initializer; CONDITIONAL for future GPU backing |
| Clean-room contents preservation | CONDITIONAL for GPU path; supplied host snapshot equality check implemented |
| Clean-room first-use no-overlap | CONFIRMED under preserved P7H-030 scope/order/geometry |
| Nine clean-room first-use holes | CPU image ZERO_CONFIRMED; GPU first-use CONDITIONAL |
| `0x07000345` exact read set | UNKNOWN |
| `0xaf000000` semantics | UNKNOWN |
| PDS ISA | UNKNOWN |

**Does the restricted implementation still require the exact read set?**
**CONDITIONAL — exact reads are unnecessary if a complete deterministic launch
state superset is established and the provider conforms to C1–C9. C10 is not
established by the current evidence.** The implementation dependency is not
removed and FG-02 remains OPEN. This is not a proof that an exact decoder is
necessary or that a deterministic Route C implementation is impossible.

**One next research premise:** establish that the selected launch supplies all
consumed DS/temporary/implicit state from the deterministic backing and known
launch inputs. A supported state-load extent/initialization/isolation rule is
sufficient; exact operand names and read count are not required. Do not return
to target installation archaeology, a broad provider survey, generic PDS
decoding or the unavailable PDF. No hardware experiment is authorized.

## Reproduction

From the repository root:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover \
  -s tools/psb-dri-re -p test_cleanroom_pds_image.py -v
python3 tools/psb-dri-re/cleanroom_pds_image.py --check-table
python3 tools/psb-dri-re/cleanroom_pds_image.py --relocated-use-word 0x12345678
PYTHONPYCACHEPREFIX=/tmp/sgx535-route-c-pycache python3 -m py_compile \
  tools/psb-dri-re/cleanroom_pds_image.py \
  tools/psb-dri-re/test_cleanroom_pds_image.py
```

The last image command uses a **test parameter**, not a valid GPU address.
The tests were written first and failed for the absent constructor module;
the added whole-payload test separately failed before its initializer existed.
Negative cases reject omitted initialization even with zero-valued storage,
all14 corrupted dword positions, wrong provenance, malformed lengths, a wrong
CSV field, partial copies and unknown/outside containment. The containment tests
test the proof checker, not a hardware decoder. No historical program runs.

Validation completed: 16 unit tests pass; both scripts compile; the 14-dword
CSV matches the checker; both retained ELF hashes match. All38 Phase7/evidence
CSV header/width checks and596 Phase7 local Markdown file links pass. P7H-001–037
have unique defining rows. A broader initial uniqueness assertion encountered
older multi-row evidence groups; their repeated-ID counts exactly match HEAD
and were preserved. No new ID collision exists. `git diff --check` and explicit
new/untracked-file whitespace checks pass. `references/` has no Git changes;
the index is empty. Historical provider/corpus surveys and unrelated archive
hash suites were not rerun. GPU visibility and containment were not tested or
claimed by these host-only checks.

## Launch-state coverage refinement (P7H-038/039)

See [the selected launch-state envelope](pds-launch-state-coverage.md) and
[its checked inventory](pds-launch-state.csv). The CPU launch words are now
parameterized explicitly: `R(S), 0x00030000, R(P)|0x0c000000`. This does not
make the packed count a hardware read bound. The next rule is **L12**: the
initialized architectural state domain for this count/launch, including the
eligibility of other state for pre-definition reads. DS, temporary and implicit
coverage remain UNKNOWN; no outside read is asserted. All56 CPU bytes stay
deterministic, first-use no-overlap stays CONFIRMED, GPU preservation stays
CONDITIONAL. The implementation dependency has not closed; FG-02 remains OPEN.

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
