# Frozen fragment link and pixel-state trace

**Route C continuation (P7H-036/037):** the
[clean-room primary CPU image](cleanroom-pds-image.csv) is fully defined for an
explicit resolved USE word U. Historical target-provider identity is no longer
a prerequisite for this new construction. FG-02 remains OPEN because the
[backing contract](cleanroom-backing-contract.md) has not established complete
deterministic PDS launch-state coverage or a concrete GPU provider. No linked
suffix, secondary-record, pixel-state or instruction-semantic claim is changed.

This is the selected-path FG-02 checkpoint after [FG-01](frozen-fragment-exact-output.md). It is a static trace of the retained `psb_dri.so` (SHA-256 `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`), not an executed result. ELF virtual addresses below exclude Ghidra's `0x10000` image bias.

## Selected link key and record

`0x0002c852` makes the compiler key. Its feature bits 1 and 2 are zero for the frozen no-alpha, no-depth-output, full-color-mask, no-blend state, as established in the [symbolic trace](frozen-fragment-symbolic-trace.md). The key's bit 0 compares front/back fields under `context+0x65c`. The candidate Mesa 7.4.4 stencil initialization sets the compared `Ref`, `ValueMask`, and `WriteMask` values equally (`src/mesa/main/stencil.c:558–589`; `mtypes.h:1125–1153`), so bit 0 is zero **if** that source layout and initialization match this retained build. This is a source-family correlation, not an independent binary proof of the initial context bytes. The selected `0x0002c610` no-feature path then gives a zero 16-byte link key under that condition.

The linker `0x000370d8` initializes its `0x88`-byte record to zero. Substituting FG-01 (`result+0xb4=0`, `+0xb8=2`, `+0x1cc=1`, `+0x1d0=0`, `+0x1d8=0`) and key bits 2 and 3 clear produces the following **CONFIRMED conditional binary rules**. The first sixteen key bytes need not be assumed zero for the instruction-suffix result; only bits 2 and 3 control these branches.

| Record offset | Selected value or relationship | Producer |
| --- | --- | --- |
| `+0x08…+0x17` | copy sixteen bytes of key; all zero under the stated candidate Mesa initialization | `0x0002c610`, `0x000370d8` |
| `+0x20` | CPU pointer to compiler result; allocation-dependent | `0x000370d8` |
| `+0x44/+0x48` | `0x00000000`, `0xf8000140` | `0x000370d8`, flags bit `2` branch |
| `+0x6c/+0x6d/+0x6e` | `1/0/0` | `0x000370d8`; one suffix instruction |
| `+0x6f/+0x70/+0x71/+0x72` | `1/0/1/0` | result `+0x1d8=0`; feature helpers skipped |
| `+0x74` | `0` | result `+0xb4=0` |

For a fresh fragment-result cache entry, `0x0002c610` sets the record's next pointer at `+0x00` from the previous head. The frozen new-context/no-prior-draw condition makes that pointer null. This pointer and `+0x20` are CPU addresses, not SGX device addresses.

## Linked USSE suffix and scene descriptor

`0x000283d8` calls `0x000264b7` with the link record and a separate 0x1c-byte scene-program descriptor. The latter starts zeroed (`0x00028301` allocates scene heap space, followed by the explicit seven-dword clear in `0x000283d8`). `0x000264b7` allocates `(0+0+1+even(0))*8 = 8` bytes with requested alignment `0x20`. It copies zero compiler instruction bytes, copies the suffix pair from link `+0x44`, then ORs `0x00040000` into its last word. The committed linked instruction is therefore:

```text
word 0: 0x00000000
word 1: 0xf8040140
little-endian: 00 00 00 00 40 01 04 f8
```

This is **not** the compiler output: FG-01's compiler streams remain empty. It is a link-time suffix produced by the retained binary. Its ISA meaning is not established here. The scene-program descriptor receives its allocated output reference at `+0x0c` through `0x00048762`; its other initial dwords remain zero until later writers. The `0x000283d8` disassembly at `0x28430–0x284ee` confirms that `0x00025970` receives this *descriptor*, not the link record, as its second argument. Passing the link record would falsely imply a null BO in the relocation helper. This corrects that tempting but invalid decompiler-variable reading.

## Primary and secondary scene program paths

`0x00025970` takes the descriptor and scene key and allocates a 400-byte scratch/outbuf request. With compiler constant-descriptor count `+0xc0=0`, link `+0x70=0`, and linked temp count `+0x74=0`, it takes the zero-input branch. It records three masked USE relocations through `0x0003a82c` → `0x0003773b`/`0x00037621` against the linked-program BO in descriptor `+0x0c`; these helpers initially write `0x67676767` at the patched dword. Dynamic BO offset application remains a relocation operation, not a fixed address. The selected branch writes dword `+0x04=0` from descriptor `+0x08`, dword `+0x20=0x20`, terminal words `+0x30=0x07000345` and `+0x34=0xaf000000`, and commits through `+0x38` (56 bytes). The destination allocation is 400 bytes; allocation size and committed length differ. Other dwords in the committed range require a write/consumption audit before treating them as zero or unused. The output descriptor at scene-key cache `+0x128` supplies the six words later copied to scene `+0x430…+0x444`.

`0x00039ea0` tests compiler flags `+0xb8 & 0x80`, linked byte counts `+0x70/+0x72`, and `result+0x1d8`. They are all zero in this selected case, so it calls `0x0003faa3`. That helper allocates four bytes, writes `0xaf000000`, commits them, and clears three output words at `param_2+0x0c/+0x10/+0x14`. `0x00028559` reaches it through the pixel-state cache miss. The secondary path's word is established as a historical CPU emission; its hardware effect remains unverified.

The later [primary PDS backing-memory trace](pds-buffer-lifecycle.md) shows
that the selected 56-byte object is a suballocation of a mapped batch-pool
slot. Commit advances a cursor and records length; it does not clear or copy
the nine producer-unwritten data dwords. Pool slots can be recycled without
clearing their bytes. A full-range clean-room write is physically possible,
but its equivalence to the selected historical PDS operation is unproved.

P7H-028 additionally finds a first-use zero-fill mechanism in the existing
candidate kernel's PDS TTM path. This supplies a conditional complete image
for a fresh untouched range, but the kernel pairing and fresh-range
preconditions remain open; it is not a general zero-hole rule.

The retained `Xpsb.so` export `Xpsb_pds_get_num_constants` (ELF `0x7470`) calculates the occupied dword extent for two interleaved constant stores. That is direct evidence of this binary's data-store layout arithmetic. It is **not** a PDS instruction decoder and does not prove which dwords the DRI program reads. The neighboring clear helper `0x00039ac4` also emits `0x07000345, 0xaf000000` while leaving intervening data dwords unwritten; this repetition constrains the historical software pattern but cannot establish read semantics. The exact selected-program boundary is recorded in [the PDS handoff](selected-pds-hard-boundary.md).

A later exact-byte sweep found `0x07000345` in Xpsb's named `Xpsb_emit_pixel_shader` (ELF entry `0x82c0`, store `0x8953`; P7H-017). Its zero-texture path also uses a 12-dword data prefix and the same terminal word, while its texture path appends data and instructions. This independently locates the same CPU emission in a second retained ELF, although shared historical source lineage is plausible. It does not decode SGX535 operand reads. [The comparative ledger](pds-readset-search-ledger.md) keeps the bit-difference and source searches separate from the binary facts.

The pixel-state key itself is 0x150 bytes and `0x0002cca3` conditionally fills it from render-target and fragment-input state before `0x00028559` caches it. The primary 0x128-byte key is filled by `0x0002d015`. Their dynamic target/program references and the remaining producer-unwritten PDS words are not yet specified byte-for-byte. FG-02 therefore remains **OPEN** as a complete pixel-state contract, although the linked suffix and zero-input secondary path are now exact.

This trace changes no safety gate. Gate B is **BLOCKED**, the hardware whitelist is empty, and hardware functionality is **UNVERIFIED**.

The subsequent [comparative PDS pass](pds-comparative-encoding.md) verifies 42 retained candidate/code stores or formulas and cross-checks dynamic index arithmetic against separately stored literals (P7H-019/020). It makes a three-dword selected read set `{+0x00,+0x04,+0x20}` plausible, but **INFERRED**: neither the `0x45` source count nor the terminal word's read behavior is decoded. FG-02 remains open, and no producer-unwritten slot is reclassified as dead.

## Pairing / first-use continuation (P7H-029–031)

[The stack proof](pds-stack-pairing.md) resolves predecessor overlap for the
restricted new-context first scene. Primary PDS occupies [0x160,0x198) or
[0x1c0,0x1f8) within its slot; the internal-clear branch's secondary record
is included. Candidate BO/fence headers are identical, and the candidate PDS
TTM path supplies zeroed pages preserved through CPU mapping and GPU binding.
The remaining dependency is the actual target provider's same contract:
retained major4/minor>=0 acceptance does not identify its allocator. The
restricted zero image is therefore CONDITIONAL; no PDS ISA claim changes
and no unconditional FG-02 closure is warranted.

## Backing-provider checkpoint (P7H-032–035)

The [new provider audit](pds-backing-provider.md) verifies the retained OBS
package lineage and selected backing equivalence in additional public PSB
branches. No selected dirty-backing counterexample was found; the target
provider membership remains unestablished. Therefore the restricted primary
zero image remains conditional and FG-02 is not closed. Existing link-time
suffix and secondary-PDS facts remain unchanged.

## Launch-control serialization, P7H-038/039

The [launch-state analysis](pds-launch-state-coverage.md) follows the primary
and secondary descriptors into the present-bit0x40 state record. Selected
resource fields reduce its middle word to `0x00030000`; the primary reference
carries count12 as `0x0c000000`. These are confirmed CPU formulas parameterized
by resolved BO addresses, not a complete pixel-state or hardware preload ABI.
This advances the CPU state record without closing FG-02 or Route C's L12
launch-definedness obligation. No backend or hardware operation was performed.

## Independent frozen-draw advance (P7H-042–046)

L12 is frozen OPEN. [The new triangle specification](frozen-triangle-spec.md)
connects the existing fragment launch words to a14-dword state upload and the
scoped68-byte TA stream, and inventories independent target/background programs.
The primary/pixel cache keys are CPU cache objects; skipped payload fields are
not new GPU byte gaps on the selected fast paths. This closes CPU packing and
ordering subquestions without closing FG-02 or any PDS source-domain claim.

## Final independent burn-down (P7H-047–051)

[Concrete BO/wire and surface models](frozen-triangle-burn-down.md) now bind
the fragment CPU images into the six-user-BO validation plan. FT-TARGET closes
for the selected linear ARGB8888 contract. FG-02 remains OPEN on L12, auxiliary
input coverage, device publication and revision-qualified service ready state.
