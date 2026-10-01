# Selected fragment PDS: data-consumption boundary

**Current Route C scope (P7H-036/037):** the new
[clean-room contract](cleanroom-backing-contract.md) constructs all 56 CPU bytes
deterministically, including the nine zero holes, without requiring equality to
historical recycled values or historical provider identity. Implementation
closure remains CONDITIONAL: complete deterministic PDS launch-state coverage
is unproved even with an ideal BO provider; concrete GPU-provider conformance
also remains to be implemented. The CPU commit/data-prefix extent is not an
established hardware read bound. Earlier historical-reproduction objections
below must not require stale-value equivalence under Route C.

This handoff concerns only the frozen first triangle after the [fragment-link trace](frozen-fragment-link-progress.md). It does not decode general PDS, authorize GPU execution, or change Gate B.

The retained `psb_dri.so` (SHA-256 `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`) reaches `0x00025970` through `0x0002d015` → `0x000283d8`. The selected compiler has zero input descriptors, zero linked temporary count and no secondary constants. The scene heap descriptor passed to `0x00025970` is distinct from the linked-key object: i386 call setup at `0x28430–0x284ee` supplies the descriptor allocated at `0x000283d8`, whose `+0x0c` receives the eight-byte linked-program BO reference. This resolves an apparent null-relocation contradiction caused by following the wrong pointer.

## Exact CPU stores and remaining bytes

`0x00025970` requests 400 bytes from the outbuf and commits 56 bytes for this zero-input branch. A clean-room implementation would need to determine the value or non-consumption of *every* committed dword. The allocation itself is not a zeroing operation (`0x00038762`). The outbuf reset at `0x0003865f` calls `driBOData` (`0x00044c90`) with a null source pointer. That routine can retain a sufficiently large existing BO, and copies source bytes only when a non-null source is supplied; `driBOMap` (`0x00043ffe`) only maps it. This establishes a **possible reused-BO path with no clearing by these functions**, not the actual initial contents of the selected BO. Zero-filled holes cannot be inferred from allocator behavior.

| Offset | Selected CPU behavior | Status |
| --- | --- | --- |
| `+0x00` | `0x0003a82c` emits three masked relocation records to the linked-program BO; each helper writes placeholder `0x67676767` before the candidate kernel applies relocations | CONFIRMED producer/relocation relationship; final address is dynamic |
| `+0x04` | zero under the frozen first-entry key and zero temp count | CONFIRMED conditional CPU store |
| `+0x08…+0x1f` | no store by this selected emitter | UNKNOWN whether PDS reads these six dwords |
| `+0x20` | `0x00000020` | CONFIRMED CPU store |
| `+0x24…+0x2f` | no store by this selected emitter | UNKNOWN whether PDS reads these three dwords |
| `+0x30` | `0x07000345` | CONFIRMED literal CPU store; instruction semantics UNKNOWN |
| `+0x34` | `0xaf000000` | CONFIRMED literal CPU store; instruction semantics UNKNOWN |

The table separates the six dwords at `+0x08/+0x0c/+0x18/+0x1c/+0x28/+0x2c` mentioned in the earlier vertex-PDS report from this **primary fragment** record. This primary record has **nine** producer-unwritten dwords: six at `+0x08…+0x1f` and three at `+0x24…+0x2f`. Neither set may be silently filled with zero for an implementation-grade historical reproduction. The terminal `0xaf000000` is also emitted by the selected secondary fast path (`0x0003faa3`), but shared literals alone do not establish identical PDS execution context.

The nine holes, individually, are `+0x08`, `+0x0c`, `+0x10`, `+0x14`, `+0x18`, `+0x1c`, `+0x24`, `+0x28`, and `+0x2c`. The first six fall in the initial eight-dword half of the data prefix; the last three fall in the next half. `Xpsb_pds_get_num_constants` supports that two-store layout, but does not prove a bank/source selector for `0x07000345`. None of the selected emitter's three relocation records targets a hole: each is tied to the linked-program address at the start of the prefix. A hole could still be read by the PDS, since CPU writes and relocations are not a hardware read-set trace.

The [comparative search ledger](pds-readset-search-ledger.md) records the next static pass (P7H-016). The same `0x07000345` literal appears in the clear emitter at ELF `0x39bd2` and the texture-replace emitter at `0x39e0c`, as well as here at `0x262f0`. Clear and frozen primary both initialize `+0x00`, `+0x04`, and `+0x20` while leaving the same nine slots without a producer store. Texture replacement adds four data words and two later instructions without changing the first word. This supports—but does not prove—the hypothesis that the first word reads only common initialized data. A bit comparison with the related `0x07042345` word changes bits `0x00042000`; it is insufficient to assign source fields or prove the selected read set.

The retained `Xpsb.so` supplies a second exact software path (P7H-017). Its named `Xpsb_emit_pixel_shader` at ELF `0x82c0` stores `0x07000345` at `0x8953`; with zero texture inputs it also lays out a 12-dword data prefix, initializes `+0x00/+0x04/+0x20`, and follows the literal with `0xaf000000`. This is strong corroboration of the historical **emission pattern**, but the binaries may share source lineage and neither interprets the selected PDS instruction on the CPU. It is not independent evidence of SGX535 hardware reads.

I also inspected Xpsb's named `Xpsb_emit_pixel_event_program` at ELF `0x7eb0` in a disposable Ghidra project. It emits different PDS-shaped words (`0x07000363` and `0x07070365`) in a different 0x80-byte program. It supplies no controlled single-field variation of the selected word, so it cannot settle the read set. The [ledger](pds-readset-search-ledger.md) records this and the independent public-source routes checked.

`Xpsb_pds_get_num_constants` in the retained `Xpsb.so` (SHA-256 `da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f`, ELF `0x7470`) computes a two-store interleaved dword extent. The function does not parse either instruction word or identify its constant operands. The DRI clear path `0x00039ac4` emits the same two terminal words and leaves intervening data dwords unwritten. That is a software-pattern corroboration, **INFERRED** as to likely non-consumption, never CONFIRMED hardware semantics. The [public Imagination forum thread](https://forums.imgtec.com/t/pds-programming/333) records a user-supplied SGX535 input-parameter document title and a staff referral to developer support; it is a locator, not an authenticated copy or a decoding rule.

A focused check of the public [PVR_PSP2 repository](https://github.com/GrapheneCt/PVR_PSP2/tree/ae4df4b723f3f1aa629b1f3b276b9cca6d663891) found a `host/pdsasm` directory. Its README identifies PSP2 as the target, and the examined PDS header bears an Imagination all-rights-reserved notice that forbids redistribution without permission. I did not use that code or its encodings as project evidence: its availability in a public Git repository establishes neither reuse rights nor SGX535/Poulsbo applicability. This lead therefore does not close the read-set question.

## Why this blocks a clean-room first triangle

The first indispensable unknown is the selected PDS instruction's source-operand mapping: does `0x07000345` read any of the producer-unwritten dwords, and if so which ones? The two program words, relocation producer and BO commit are known; the missing fact is the SGX535/Poulsbo PDS consumption contract. The following `0xaf000000` word is also a literal rather than a sourced decode, so its behavior would need separate confirmation after the first word's read set is settled. A clean-room implementation could initialize unused dwords deterministically *after* proving they are unused. It cannot claim an exact historical program or hardware safety by assuming so.

The comparative binary samples do not identify that contract by themselves. Two decoders could agree with every observed CPU write: one reads only `+0x00/+0x04/+0x20`; another also reads an unwritten slot. The retained code emits the same instruction word under both models, and no CPU-side decoder has been identified in the relevant retained paths. The texture-replace comparison and zero-index bit pattern favor the first model, but do not falsify the second. This is why the read-set classification stays **UNKNOWN** despite a plausible hypothesis. Filling holes with zero is not a neutral historical reproduction: it supplies values for inputs whose consumption and effect remain unknown.

The expanded [42-row instruction corpus](pds-instruction-corpus.csv) and [differential analysis](pds-comparative-encoding.md) narrow the hypothesis without closing it. The dynamic `0x64`/`0xa2` CPU formulas in DRI `0x25970` reproduce `0x07042364`/`0x070418a2` literals in DRI `0x39c5c` for known DS indices. The two `0x45` literals differ by `0x00042000`, precisely bits 18 and 13 used by those formulas for index 0→2. Secondary builders that emit `0x07042345` initialize the analogous `+0x08/+0x0c/+0x28` triplet. This supports **INFERRED** consumption of selected `+0x00/+0x04/+0x20`, but the candidate formula fails on the mixed-program `0x070b0345` and no source fixes the `0x45` instruction's read count or bank/selector semantics. All nine holes remain **UNKNOWN** as GPU reads. The [constraint script](../../../tools/psb-dri-re/pds_field_constraints.py) verifies the ELF literals and rejected complete-template hypothesis; it is deliberately not a PDS decoder.

For this narrow question, I checked A: the retained DRI producers `0x00025970`, `0x00039ac4`, `0x00039c5c`, `0x00040355`, the outbuf allocator `0x00038762`, and masked relocations `0x0003a82c/0x0003773b/0x00037621`; B: the retained Xpsb constant-layout helper `0x7470`; H: the already catalogued SGX535 register/DDK material, which lacks an applicable PDS instruction decode; and the project's Phase 5–6 restricted-document trail. A repository-wide exact-word/symbol search found no other decoder or producer for `0x07000345`/`0xaf000000`. C–F define submission and relocations, not PDS source-operand semantics. No available static artifact in this trace establishes the read set of the two selected PDS words. This is **not** proof that no public source exists; a legitimately obtainable SGX535 PDS instruction reference or independently attributable matching assembler/decoder is the smallest artifact that could settle it.

This is a hard **specification** boundary for the selected clean-room program, separate from the harder **execution** boundary: [Phase 7.0](../7.0-gate.md) still has FIRST-OBSERVATION-DESIGN BLOCKED, Gate B BLOCKED, and whitelist `[]` because register access attributes, SGX power/clock/reset state, concurrency, failure and recovery are unproved. No first-triangle hardware experiment is permitted. Hardware functionality remains UNVERIFIED.

The later [external Series5 PDS evidence pass](series5-pds-external-evidence.md) searched public code indexes, archived repository indexes, package/source indexes, a relevant independent SGX540 reversing project, public architecture documents and patent trails. It found no provenance-qualified SGX535-applicable rule for the `0x45` source selectors/read count or `0xaf` read behavior. This strengthens the *search record*, not the inferred read-set hypothesis: all nine producer-unwritten dwords remain possible reads, and FG-02 remains OPEN.

The latest [CPU semantic-corpus pass](pds-hypotheses.md) fits the previously contradictory `0x070b0345` with candidate pair field `A=2` (matching event data at `+0x10/+0x14`) plus unexplained bits 16–17. The selected/secondary/event triplet is now an explicit, reproducible *word* model, but no CPU-side construction varies the fixed `0x45` source count or proves that the terminal `0xaf000000` has no data reads. A three-source model and one with an additional fixed read of `+0x08` make identical predictions for every retained CPU store; the first irreducible distinction is therefore the fixed opcode/read-count interpretation, not the arithmetic pair delta. Deterministically filling holes does not establish their semantic safety. The selected read set is still **UNKNOWN**, FG-02 OPEN, Gate B BLOCKED.

The subsequent [Experiment #43 search](pds-experiment43.md) establishes that
the `0x070b0345` residual bits 16–17 are literal/invariant at their only
retained producer, and finds no new `0x45` or `0xaf` direct-immediate
variation in either retained ELF or a quarantined older related DRI package.
It does **not** determine whether an additional fixed/implicit source exists.
M1 and M2 remain observationally indistinguishable in the available
CPU-emission evidence; `0xaf000000` remains architecturally undecoded.

The subsequent [backing-memory lifecycle trace](pds-buffer-lifecycle.md)
follows the selected 56 bytes through a 30-slot batch pool, mapped
suballocation, commit, release and fenced slot recycling. Recycled slots
retain their byte contents; neither the selected builder nor relocation
normalizes the nine holes. They can contain stale bytes whose exact values
are not determined by the frozen draw. The entire committed range is
CPU-writable without allocator metadata, so a new driver can create a
deterministic *byte image* by initializing it. Whether any chosen fill value
preserves the selected PDS task behavior is still UNKNOWN. Historical bytes,
PDS reads and clean-room writeability are separate claims; FG-02 remains OPEN.

The candidate-kernel continuation (P7H-028) does establish a conditional
fresh-backing alternative: its non-fixed PDS TTM pages are allocated with
`__GFP_ZERO`. Under that contract and with no prior writes to the selected
range, all nine holes would be zero. This can reproduce a specific first-use
image without decoding reads, but exact historical pairing and the required
fresh/no-overlap history are not established for the frozen path. It does
not establish zeros after pool reuse.

## Restricted first-use boundary (P7H-029–031)

The current immediate dependency is [stack pairing](pds-stack-pairing.md),
not additional PDS corpus inference. First-use allocation history now closes
for the selected successful path: the primary range is untouched at slot
offset0x160 or0x1c0, accounting for both internal-clear branches. Candidate
libdrm/kernel share an identical generic DRM header, and candidate PDS backing
is freshly zeroed and mapped to the same CPU/GPU pages. The actual target
backing provider is not authenticated by the retained DRI's major4 version
check. Thus the zero-hole construction remains CONDITIONAL only on that
provider's applicable allocation/visibility contract (within the stated
first-scene scope). No hole is classified as architecturally dead; read-set
UNKNOWN and FG-02 OPEN remain. An equivalent provider contract suffices;
obtaining a PDS manual is not the next prerequisite for this route.

## Target-provider boundary (P7H-032–035)

The [adversarial provider audit](pds-backing-provider.md) establishes a bounded
historical fresh-zero/preserved-contents family and excludes stolen fallback
for mask0x20000001. The exact retained spec matches OBS revision3, but neither
its dependencies nor the target record selects a kernel implementation. The
remaining fact is the target-to-provider binding, not another corpus or
first-use experiment. The modern gma500 source associated with the target
report is not a direct legacy PDS provider. Restricted zero holes remain
CONDITIONAL; read semantics UNKNOWN; FG-02 OPEN.

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
