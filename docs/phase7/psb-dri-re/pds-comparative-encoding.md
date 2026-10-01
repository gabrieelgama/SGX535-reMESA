# Selected Series5 PDS word: comparative constraints

This is a CPU-side static analysis of the retained Poulsbo `psb_dri.so` and `Xpsb.so`, not a PDS instruction decode. The [42-row curated corpus](pds-instruction-corpus.csv) separates 37 verified CPU instruction-region stores/formulas from five mixed-program candidates. [The check script](../../../tools/psb-dri-re/pds_field_constraints.py) verifies literal bytes against both hashed ELFs and evaluates the recovered construction formulas. Its [output](pds-field-constraints.csv) is a regression table, not an assembler.

The DRI SHA-256 is `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`; the Xpsb SHA-256 is `da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f`. All addresses below are ELF virtual addresses.

## Corpus and direct construction rules

The selected DRI emitter `0x25970` stores `0x07000345` at `0x262f0` and, for nonzero descriptors only, constructs two following instruction classes. Xpsb `Xpsb_emit_pixel_shader` (`0x82c0`) uses the same literal and index arithmetic. These formulas are **CONFIRMED as CPU construction**:

```text
A2(ds1) = 0x070400a2 | ((ds1 >> 1) << 13) | (((ds1 & 1) + 2) * 0x800)

op64(ds0, ds1) = 0x07000064
  | ((ds0 >> 1) << 18) | ((ds1 >> 1) << 13)
  | ((ds0 & 1) << 11) | (((ds0 + 1) & 1) << 9)
  | (((ds1 & 1) + 2) * 0x80)
```

The independent literal stores in the DRI texture-replace builder `0x39c5c` are `0x070418a2` and `0x07042364`. The recovered formulas yield exactly these words for `A2(1)` and `op64(2,2)`. This cross-check ties the formula inputs to its data-store writes: DS0 index 2 is at `+0x08`, DS1 index 1 at `+0x24`, and DS1 index 2 at `+0x28`. It does **not** prove how the GPU interprets the resulting instruction words.

The primary literal `0x07000345` occurs in DRI `0x25970`, clear `0x39ac4`, texture replacement `0x39c5c`, and Xpsb's pixel-shader emitter. Secondary builders `0x287ff`, `0x29385`, and `0x39ea0` store `0x07042345`. The first two initialize DS0 `+0x08/+0x0c` and explicitly write DS1 `+0x28=0`; the third does the same before its instruction region (which starts at `+0x40`). By comparison, the `0x07000345` builders initialize `+0x00/+0x04/+0x20`, with optional later data and instructions. These are multiple **CPU emitters**, not independent GPU read observations.

The Xpsb survey also found `0x07030226` for its `param_10 == 1` tail, with one source dword written at `+0x00`, and `0x07030223` for a larger tail (`0x82c0`; stores at `0x8b39` and `0x887c/0x8ca1`). Those conditional forms strengthen the classification of the corpus as real program construction rather than an arbitrary numeric grep. They do not isolate a source field of the selected `0x45` word.

| Word pair | Exact XOR | Controlled CPU-side relationship | Interpretation |
| --- | --- | --- | --- |
| `0x07000345` → `0x07042345` | `0x00042000` | Analogous data triplets move from `+00/+04/+20` to `+08/+0c/+28` across different builder families | **INFERRED**: bits 18 and 13 may select pair indices 0 versus 2. Builder families also differ, so this is not an isolated hardware operand experiment. |
| `0x2f030343` → `0x2f070343` | `0x00040000` | Vertex builder `0x40355` optionally adds a second DS0 pair at `+08/+0c`; same in `0x4252d` | **INFERRED**: bit 18 may select a DS0 pair. These are different instruction words from the selected `0x45` class. |
| `0x07000345` → `0x070b0345` | `0x000b0000` | DRI `0x392ab` contains the latter in a mixed event-related 0x94-byte block | No controlled operand variation; fields at bits 16/17/19 remain **UNKNOWN**. |

The selected word's set bits are `{0,2,6,8,9,24,25,26}`. The `0x07042345` variant adds `{13,18}`. Its low 13 bits and top byte are unchanged. A minimal hypothesis that substitutes low byte `0x45` for `0x64` in the recovered two-index formula reproduces both words. Exhausting DS0/DS1 index pairs 0–31 under that hypothesis **cannot** produce `0x070b0345`; it is therefore **rejected as a complete decoder**. The additional bits could represent mode, destination, condition, or different operand semantics. We do not name them without evidence.

## Nine producer-unwritten selected dwords

The selected 56-byte prefix has 12 data dwords followed by two literal instruction words. The interleaved DS0/DS1 naming in this table is based on **CPU indexing formulas** in DRI `0x25970`, Xpsb `0x82c0`, and Xpsb `Xpsb_pds_get_num_constants` `0x7470`; it is not a confirmed GPU source decode.

| Data offset | CPU-layout index | Selected producer | If the pair-index hypothesis is true | Unconditional GPU-read conclusion |
| --- | --- | --- | --- | --- |
| `+0x00` | DS0[0] | relocation placeholder | candidate read | UNKNOWN |
| `+0x04` | DS0[1] | zero | candidate read | UNKNOWN |
| `+0x08` | DS0[2] | none | not selected | UNKNOWN |
| `+0x0c` | DS0[3] | none | not selected | UNKNOWN |
| `+0x10` | DS0[4] | none | not selected | UNKNOWN |
| `+0x14` | DS0[5] | none | not selected | UNKNOWN |
| `+0x18` | DS0[6] | none | not selected | UNKNOWN |
| `+0x1c` | DS0[7] | none | not selected | UNKNOWN |
| `+0x20` | DS1[0] | `0x00000020` | candidate read | UNKNOWN |
| `+0x24` | DS1[1] | none | not selected | UNKNOWN |
| `+0x28` | DS1[2] | none | not selected | UNKNOWN |
| `+0x2c` | DS1[3] | none | not selected | UNKNOWN |

The candidate read set is `{+0x00,+0x04,+0x20}`. It is plausible and explains the common initialized triplet and the two-index delta, but remains **INFERRED**. No available decoder or hardware observation proves that `0x07000345` reads exactly three dwords, that its index fields match `op64`, or that the following `0xaf000000` never reads data. Thus **none of the nine holes has been proved dead**. The retained BO may be reused without clearing these positions; assuming zero would insert an unsupported input.

The available observations are compatible with at least these distinct models:

| Model | Selected reads | Explains retained CPU emissions? | Status |
| --- | --- | --- | --- |
| Pair-index, three-source | `+00/+04/+20`; terminal has no data read | Yes; matches common triplets and index delta | Plausible **INFERRED** hypothesis, not established |
| Pair-index plus hidden/fourth source | Same three plus one or more of the nine holes | Yes; CPU emission does not expose GPU reads or assert unused slots | Still possible **UNKNOWN** |
| Terminal-dependent source | First word uses the common triplet; `0xaf000000` also consumes one or more holes | Yes; the shared terminal literal has no recovered GPU decode | Still possible **UNKNOWN** |
| Different `0x45` field layout | Bits 13/18 happen to correlate with builder data placement but select another unit/condition | Yes; no independent ISA contract forces same field meaning as the dynamic `0x64` class | Still possible **UNKNOWN** |

The smallest unresolved **semantic** fact is the exact source count and selector interpretation for `0x07000345`, together with whether `0xaf000000` reads data. In the absence of that fact, the conservative possible read set among the nine holes is still all nine; the corpus has reduced plausibility, not logically excluded any hole. A single valid Series5 instruction-field rule covering both words would eliminate the alternative models. A direct pixel-program read trace from an independently validated emulator for the same PDS ISA could also do so. Repeated CPU emitters cannot.

## Cross-checks and failed alternatives

* The literal appears at four software sites, but DRI and Xpsb may share source lineage. Repetition establishes historical emission, not an architectural read set.
* The texture-replace builder adds data at `+08/+0c/+24/+28` and the two extra instructions. That supports the assignment of those data to the extra instructions but cannot rule out another read by the unchanged first instruction.
* The secondary `0x07042345` builders write the analogous `+08/+0c/+28` triplet. Their different program purpose and preceding instructions prevent treating this as a single-input experiment.
* The selected secondary fast path at DRI `0x3faa3` commits **only** `0xaf000000` in a four-byte allocation. This strongly suggests the word is a terminator without same-program data-store operands. It does not prove architectural behavior: the program might use implicit/external state, and no target observation validates it.
* The mixed event words `0x070003e5`, `0x070b0345`, and Xpsb `0x07000363/0x07070365` enlarge the corpus but lack isolated operand changes; they cannot validate an opcode or read count. The simple two-index model fails on `0x070b0345`.
* Public searches in the [ledger](pds-readset-search-ledger.md) produced high-level Series5 descriptions, later-generation Rogue ISA documentation, and the unsuitable PSP2 assembler lead. None supplies a legitimate, SGX535-applicable source-field decode. Cross-core similarity is not used as proof.

The unresolved dependency is narrow: an applicable definition of the `0x45`-class source selectors, number of source dwords, and the terminal `0xaf000000` behavior. A legitimately available SGX535/compatible-Series5 PDS field reference, or a separately authored decoder whose compatibility is demonstrated against independent samples, would distinguish the remaining models. CPU emitters alone cannot: a model that additionally reads an unwritten slot produces identical observed CPU writes and instruction words. Hardware testing is not authorized; Gate B remains **BLOCKED**.

## Counterexample-driven static refinement

The [semantic reclassification](pds-semantic-corpus.csv), [differential matrix](pds-differential-matrix.csv), [bit-influence table](pds-bit-influence.csv), and [hypothesis audit](pds-hypotheses.md) refine the earlier rejected two-index-only formula. An arithmetic decomposition `0x07000345 | (A << 18) | (B << 13) | (C << 16)` fits the three `0x45` words with `(A,B,C)=(0,0,0),(1,1,0),(2,0,3)`. The `0x070b0345` event builder actually writes a pair at `+0x10/+0x14`, as `A=2` would suggest; its bits 16–17 remain a residual `C=3` with no established hardware role. This is **INFERRED CPU-word correlation**, not a decoder or read-count proof. The dynamic A2/64 formulas and literal holdouts confirm only construction arithmetic. Models with a fixed extra source in an unwritten slot still explain every CPU emission. The read set and all nine holes remain UNKNOWN.

Varying the *CPU formula inputs* from 0 through 7 changes A2 bits `{11,13,14}`, 64's DS0-index bits `{9,11,18,19}`, and 64's DS1-index bits `{7,13,14}`. Bit 11 is shared by two formula inputs in different classes, so these are not disjoint architectural fields. The `0x45` pair delta changes `{13,18}`; the mixed-event residual changes `{16,17}`, which no recovered A2/64 index formula in that range changes. These are algebraic influence sets for the inspected constructors, not observed hardware widths. No selected `0x45` CPU constructor varies an input at all.

The [Experiment #43 producer audit](pds-experiment43.md) confirms that
`0x070b0345` is a whole-word literal at DRI `0x39719`; no retained caller
controls its residual bits 16–17. No verified new `0x45`-class or terminal
word was found outside the modeled stores, and an older related build repeats
the same three `0x45` variants. This strengthens the negative
**identifiability** finding, not the inferred GPU read set.
