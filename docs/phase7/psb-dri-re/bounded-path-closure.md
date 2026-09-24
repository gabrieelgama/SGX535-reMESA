# One bounded historical scene path

This is a static, conditional **scene-setup** path through retained `psb_dri.so` (SHA-256 `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`). Addresses are ELF virtual addresses. The decompiler output is disposable analysis material under `/tmp/sgx535-psb-decompile/`; the [target list](../../../tools/psb-dri-re/targets.txt) reproduces its selected functions in a read-only Ghidra project. Small constants and field formulas below describe evidence; they are not a transliteration of the proprietary implementation or an SGX535 packet specification.

## Selection and dependency

On the first indexed scene draw, `0x00027fa0` calls `0x00029251`. That routine tests scene dirty byte `scene+0x554`: bit `2` calls `0x0002a2f6`; bit `1` calls the wider state emitter `0x00028fff`. `0x0002a2f6` compares a 20-byte scene-state key at `scene+0x53c` and, if changed, calls `0x00029fbd`, then retains the key. Thus the bounded path below runs **only when bit 2 is set and the key differs**. It cannot be asserted to run on every GL draw or to be the only state needed for a valid first draw (P7F-001). The other bit invokes a much larger state path: `0x00028fff` → `0x000287ff` → `0x0003b73b`. The selected path is useful because it fixes its vertex and index counts; it does not replace that wider path.

`0x00029fbd` takes a scene and a five-word state input `P`. It allocates/copies eight records of four 32-bit words (`0x80` bytes) through `0x0003f94c`, allocates six 16-bit indices through `0x00037409`, calls vertex-program helper `0x00040355`, emits state through `0x00028fff`, and calls `0x0003b890` once or twice. The second call depends on `P[0]` (P7F-002). The record-copy and index-copy bytes are **CONFIRMED**; names such as rectangle/clear or hardware primitive remain **INFERRED/UNKNOWN**.

Let `W` and `H` be the binary's integer dimensions converted to IEEE-754 single precision and `F(P[n])` the corresponding integer-to-float conversion. The copied vertex words, in order, are:

| record | four 32-bit words | evidence |
|---|---|---|
| 0 | `0, 0, 0x3f800000, 0x3f800000` | `0x00029fbd`, `0x0003f94c`; CONFIRMED producer |
| 1 | `W, 0, 0x3f800000, 0x3f800000` | same |
| 2 | `0, H, 0x3f800000, 0x3f800000` | same |
| 3 | `W, H, 0x3f800000, 0x3f800000` | same |
| 4 | `F(P[1]), F(P[3]), 0x3f800000, 0x3f800000` | same |
| 5 | `F(P[2]), F(P[3]), 0x3f800000, 0x3f800000` | same |
| 6 | `F(P[1]), F(P[4]), 0x3f800000, 0x3f800000` | same |
| 7 | `F(P[2]), F(P[4]), 0x3f800000, 0x3f800000` | same |

**P7G-001 correction:** P7F-002 grouped the unchanged 128 bytes as four eight-word vertices. The call is `(record_dwords=4, record_count=8)`; `0x0003f94c` stores the former at descriptor `+0x10`. Program repeat count follows record width, not the number of vertices. The first six indices address four records; the conditional second call has base-vertex argument `4`.

`0x00037409` copies six little-endian 16-bit indices `0, 1, 2, 3, 2, 1` into a 12-byte output reservation. This gives a fully described **CPU-side** vertex/index copy for symbolic inputs. The meaning and valid range of `P`, chosen attachments, required flags, and SGX interpretation are not established.

## Vertex USE/USSE and PDS-related output

For this four-dword-record input, `0x00040355` calls `0x00030ffd` with `(4+15)/16+1 = 2` eight-byte instruction slots: a 16-byte buffer, initially zeroed. It emits one word pair through `0x00030e22`, one through `0x00030a09`, and `0x00030fb7` sets a bit in the final instruction before closing the buffer. `0x0002fabd` identifies the slot operation with the embedded `psb_use.c:0x129` assertion `use_next_inst`; `0x00030466`, `0x000304d9`, `0x00030549`, `0x00030624`, and `0x00030cb1` contain further `psb_use.c` bank assertions. **Correction to P7B-009:** the `0x00030ffd`/`0x00030fb7` eight-byte-word buffer is a historical USE/USSE instruction builder, not itself a PDS buffer. The caller also creates a distinct PDS-related vertex output block. Prior P7B observations of allocation, calls and upload remain valid; only the buffer-label inference changes (P7F-003).

For this **four-dword-record branch only**, the immediate arguments and bit operations can be followed through static i386 disassembly of `0x00040355`, `0x00030e22`, `0x00030a09`, `0x0002fabd`, and `0x00030fb7`. The allocator zeroes both slots. The first emitter receives code values `1` and `2` and a repeat-count field derived from four dwords per record; the second receives `(0,8,8,0,1,0,0,1)`. The finalizer sets bit `2` in the second slot's byte `+6`. The resulting **historical CPU-produced** bytes are:

| slot | bytes at offsets `+0` through `+7` | little-endian 32-bit words | classification |
|---|---|---|---|
| 0 | `00 00 00 a0 01 30 a1 28` | `0xa0000000`, `0x28a13001` | CONFIRMED for this caller branch; derived from instruction/flag writes |
| 1 | `00 00 20 a0 00 50 27 fb` | `0xa0200000`, `0xfb275000` | CONFIRMED for this caller branch after finalizer |

This closes the **literal-byte question for this conditional buffer**, not the program contract. The bank-code helpers and `use_next_inst` assertion explain how the bytes were constructed; they do not establish what each field means on every SGX535 revision, whether this branch is valid for a minimal GL draw, or whether those bytes execute successfully. They are recorded as evidence, not an opaque program to copy into a clean-room implementation.

The same caller reserves up to `0xc0` bytes and closes a `0x40`-byte vertex-state block on this one-part (`0x000381a0` result `1`) path. This is the **observed CPU-written layout**, not proof of which words hardware consumes:

| byte offset | 32-bit producer/value | status |
|---|---|---|
| `0x00` | relocation through `0x00037854` to the vertex data BO | CONFIRMED relocation call; final address UNKNOWN |
| `0x04` | `0x80600003` from the four-word-input, one-part `0x000381a0` calculation | CONFIRMED conditional formula |
| `0x08–0x0c` | no store on this branch in `0x00040355` | CONFIRMED local absence of store; resulting bytes UNKNOWN |
| `0x10` | multi-field relocation through `0x0003a82c` to the 16-byte USE/USSE BO | CONFIRMED call; final bits UNKNOWN |
| `0x14` | zero | CONFIRMED |
| `0x18–0x1c` | no store on this branch in `0x00040355` | resulting bytes UNKNOWN |
| `0x20` | `4 × 4 = 16` | CONFIRMED |
| `0x24` | zero | CONFIRMED |
| `0x28–0x2c` | no store on this branch in `0x00040355` | resulting bytes UNKNOWN |
| `0x30` | `0x67800070` | CONFIRMED literal; hardware meaning UNKNOWN |
| `0x34` | `0x2f030343` | CONFIRMED literal; hardware meaning UNKNOWN |
| `0x38` | `0x030803e5` | CONFIRMED literal; hardware meaning UNKNOWN |
| `0x3c` | `0xaf000000` | CONFIRMED literal; hardware meaning UNKNOWN |

The three apparent holes are not proven zero-filled: `0x00038762` can return mapped pool storage and does not itself clear the reservation. Nor is there proof that those words are consumed by hardware. This is an exact byte-level reproducibility gap, not permission to substitute zeros. `0x0003a82c` calls three relocation helpers for one destination word; the post-relocation value requires the candidate kernel's address-space and use-base rules, still unverified for the retained binary/kernel pair.

## Bounded TA record and finalization

The selected `0x00029fbd` call supplies primitive input `0`, first index `0`, count `6`, and a caller word `0` to `0x0003b890`. A second call, if `P[0] != 0`, supplies the same count with caller word `4`. Both use the 12-byte index BO, 0x48-byte vertex-program object, and scene TA output (P7F-004). The emitted 20-byte record is:

| offset | selected value/producer | status |
|---|---|---|
| `0x00` | `0x81400006` (`0 | 0x81400000 | 6`) | CONFIRMED literal combination; bit semantics UNKNOWN |
| `0x04` | relocation for index BO offset | CONFIRMED relocation site; final address UNKNOWN |
| `0x08` | `0` first pass; `4` conditional second pass | CONFIRMED |
| `0x0c` | relocation for program/output BO with shift `4`, mask `0x0fffffff` | CONFIRMED call arguments; final address UNKNOWN |
| `0x10` | `0x02000103`, derived from program object `+0x0c=12`, `+0x38=4`, vertex record width `4` dwords | CONFIRMED under this selected path; field semantics UNKNOWN |

The surrounding scene may emit input and general state through `0x0003b606`, `0x00028fff`/`0x0003b73b` and the wider `0x000287ff` builder. Those bytes are **not** fully bounded by this selected record. On finalization `0x0002a39a` reserves eight bytes, writes `0xc0000000` to the first word, closes the reservation, then calls `0x00037b51` with engine selector `0`. **P7G-002 correction:** `0x000384ab` closes at `start+4`, so only the four-byte word is committed. The unused second reservation word is not an emitted termination field. P7F-005 established reservation size, but its note incorrectly treated capacity as output length. The candidate kernel's `PSB_ENGINE_TA=3` and offset/value submit handling identify a plausible interface but do not decode SGX TA words.

One general-state branch is narrower than that description suggests. `0x00029f38` zeroes a 34-word input and sets presence mask `0x54c5`. If `0x00028fff` emits all present fields (rather than eliding unchanged cached state), `0x000287ff` copies ten 32-bit words: mask `0x54c5`; field at input `+0x04` (`0x07f00100`); conditional field at `+0x0c` (`0` or `0x02000000`); three zero words for bit `0x40`; two zero words for bit `0x80`; `0x04001000` for bit `0x400`; `0x00010000` for bit `0x1000`; and zero for bit `0x4000`. This is a **conditional CPU-side layout** established by `0x00029f38`, `0x00028fff`, and `0x000287ff`; its hardware fields, cached-state elision, subsequent generated USE/USSE state program and relocation remain insufficiently specified. The same builder accepts many other presence masks, so this ten-word case is not a universal scene-state format.

The minimal remaining program/record gaps are the **meaning and applicability** of the now-bounded two-instruction USE/USSE sequence, the vertex-state relocation result and unwritten-slot policy, broader state records required by scene dirty bit `1`, and generated fragment/program state. These are dependencies before a full draw can be reproduced; known literal bytes do not make the path implementation-grade.
