# Frozen fragment compiler output

**FG-01 CLOSED FOR THE FROZEN DRAW, as a static CPU transformation.** The retained compiler emits **zero main USSE instructions and zero secondary USSE instructions** for the selected no-texture `MOV primary color → output color; END` input. There are no encoded words and the little-endian instruction byte sequence is empty. This says nothing about the linked pixel program or hardware behavior; FG-02 remains separate.

The source is the retained `psb_dri.so`, SHA-256 `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`. Addresses below are ELF virtual addresses (the disposable Ghidra image adds `0x10000`). This result comes from selected-path control/data-flow analysis, not execution or `dlopen` of the ELF. The [symbolic trace](frozen-fragment-symbolic-trace.md) records the intermediate checks and a corrected distinction between two USC input pointers.

## Exact input and lowering

For the frozen no-texture/no-fog program, the Mesa fixed-function builder supplies primary color only. The retained converter `0x00033490` makes two linked, zero-initialized `0x84`-byte UniFlex records:

| Record | Nonzero fixed fields | Provenance |
| --- | --- | --- |
| MOV | opcode `0x48`; destination type/index `6/0`, component mask `0xf`; source type/index `1/0`, swizzle `0x0688`; link at `+0x80` to END | `0x00031381`, `0x00033490`, `0x000310e0` |
| END | opcode `0x35`; null link | same |

The pointer value in the first record is allocation-dependent and is not an input instruction word. The source has no absolute/negate/relative modifiers. The historical Mesa 7.4.4 `texenvprogram.c` path and retained `ProgramStringNotify` at `0x000398bc` establish no extra instruction or parameter for this case. The fragment key's bit 0 is unresolved but has no read in this converter/compiler setup; bits 1, 2, and 3 are zero for the frozen state. The wrapper `0x0003503d` passes flags `0x4a`, compiler mode `1`, software target `(3,111)`, zero texture count, and zero parameter count. `(3,111)` is a software build target, not a measured SGX revision.

`0x001c3ba5` and `0x00219998` lower MOV to four scalar internal opcode-`1` nodes. Before optimization, their source operands are primary-attribute class `2`, virtual indices `[42,41,40,43]`; their destination operands are temporary class `0`, virtual indices `[40,41,42,43]`. All four are unpredicated, have one destination and no side-effect flag. END creates a control-flow terminator in `0x001c22b0`, not an instruction. These numbers are **virtual**; none is a physical USSE register assignment.

The selected parser adds neither of its two potential post-END output-copy paths. It initializes state `+0x7c` to `-1`; output-type destination mapping does not change it, so the first path's `!= -1` guard fails. The other path tests a mask at wrapper configuration `+0xc0`, which remains zero after the wrapper's 0x32-dword initialization for this no-texture call. The converter stack passed as USC argument 4 is a different object from the wrapper configuration passed as argument 6.

The parser's separate `0x001c5ef0` MOV insertion is guarded by state `+0x3c` bit `0x4`. The straight-line MOV/END path does not set that bit. Its state `+0x674` output counter is reset at `0x001c5fa0` and can increment at `0x001c600c` only for a set bit in the same zero wrapper mask at configuration `+0xc0`. The counter remains zero, so it cannot create a later output-live-out root.

| Stage | Selected effect on instruction IR | Evidence |
| --- | --- | --- |
| UniFlex parsing and scalar lowering | Four virtual opcode-`1` MOVs; END remains control flow | `0x001c3ba5`, `0x00219998`, `0x001c22b0` |
| Index/texture/special-op passes | No selected relative index, texture or special opcode; no inserted instruction | `0x001d4760`, `0x0020733b`, `0x0020906a` |
| First reverse-liveness/DCE | All four MOVs removed because their temporary-class destinations are dead | `0x001deecc`, `0x001dce31`, `0x001dc31b` |
| Later SSA, packing, allocation and selection | Empty instruction list remains empty; no physical register is assigned | `0x0023ccdd`, `0x00237b63`, `0x002338b0`, `0x0022923e` |
| Finalization and group-instruction pass | No selected insertion; counts stay zero | `0x001c95f7`, `0x001f9561`, `0x001f74ab`, `0x001c72a1` |
| Emission | No instruction visits or padding | `0x001fb7af`, `0x001fa3ea` |

## Why the encoded stream is empty

Compiler mode `1` makes `0x001c6b56` seed output live-out as class `1`, register `0`, mask `0xf`. `0x001deb21` stores class-1 liveness at vector `+0x30`; `0x001dc31b` queries temporary class-0 liveness at vector `+0x20`. `0x001dbdc2` clears both before root seeding. No selected root marks temporary indices `40..43` live. The first DCE at `0x001deecc`/`0x001de047` calls `0x001dce31` with mutation enabled (i386 call at `0x001de11f`). Each scalar MOV therefore has zero live destination mask and no retained side effect, and is unlinked by `0x001ce41c`/`0x001ce28e`.

The later selected passes do not create a replacement node in an empty block. Constant packing `0x00235ca2` and the instruction/graph transforms `0x0022a4d9`, `0x001ef2ee`, `0x00227461`, `0x0022771f`, `0x00226d79`, and `0x0023adb5` require an existing instruction or dependency-graph node. The `0x00237b63` late output-copy insertion is mode-2-only; this call uses mode 1. `0x0022cc74` requires a conditional-control list, absent here. SSA has no surviving destination, and `0x002338b0` returns with resource count zero. Predicate allocation has no predicate use or output instruction to rewrite.

Finalization at `0x001c95f7` adds no instruction: its initial insertion requires state `+4` bit 0; its `0x55` insertion requires the wrapper's disabled key-bit-1 state; the remaining insertion sites traverse existing nodes. Its group-instruction helper `0x001f9561` visits the empty basic block through `0x001f74ab`; case 0 sees a null instruction list. `0x001f3ef9` clears the traversal scratch, so `0x001f3da3` has no state-transition insertion trigger. `0x001c72a1` counts the empty basic block as zero, and the straight-line END node adds no instruction. The emitter `0x001fa3ea` encounters no instruction node. Its optional terminal padding requires a two-word offset within a four-word group; the empty stream has offset zero. Consequently `0x002400eb`, `0x0023d831`, and `0x0025309a` are **not called** for this input. There is no final opcode or encoder bitfield to evaluate.

| Output | Exact result |
| --- | --- |
| Main USSE instruction count | `0` |
| Main encoded instruction words | empty sequence |
| Main encoded bytes | empty sequence, length `0` |
| Secondary USSE instruction count/bytes | `0` / empty sequence |
| Physical source, destination, temporary, predicate assignment for emitted instructions | none |
| Main/secondary instruction allocation request | no allocation: each required count is `0`; the zero-initialized result pointers remain null |
| Instruction alignment | no instruction address is allocated or aligned for this result |

This is not an unexplained copied blob: the empty sequence follows from liveness, absence of insertion, finalization count, and emitter traversal. It does **not** imply that a complete linked fragment/pixel program is empty.

There are no final output-word bitfields to tabulate: the only producing bitfield encoders (`0x0023d831`, `0x0025309a` and their callees) have no input node and are not called. The source/destination virtual components above therefore have no corresponding physical register assignment. The `0x0003503d` wrapper returns a null pointer and zero count rather than a zero-filled instruction allocation.

## Returned fields used downstream

The wrapper `0x0003503d` copies the compiler output into its zero-initialized `0x5e8`-byte result object, allocated by `0x0002c852`. Offsets are result-object byte offsets, not ELF addresses. The first fragment-link step `0x000370d8` directly consumes `+0xb4`, `+0xb8`, `+0x1cc`, `+0x1d0`, and `+0x1d8`; later program/state construction also uses the other listed counts and maps.

| Result offset | Exact selected value | Producer / derivation |
| --- | --- | --- |
| `+0xa8` main pointer | null | zero-initialized result; count 0 bypasses `0x001fb7af` allocation |
| `+0xac` main instruction count | `0` | `0x001c95f7` → `0x001fb7af` → `0x0003503d` |
| `+0xb0` first-label/phase marker | `0xffffffff` | state `+0x614` initialized to `-1`; no node changes it |
| `+0xb4` allocated temporary count | `0` | `0x002338b0` empty-SSA return → `0x001fb7af` |
| `+0xb8` compiler flags | `0x00000002` | `0x001c95f7` sets state `+0x3c` bit `0x20` for the equal zero-length phase counts; `0x001fb7af` maps it to result bit `0x2`; no selected producer sets the other mapped bits |
| `+0xbc` primary-attribute count | `0` | no surviving primary-attribute use; `0x00237b63` removes unused descriptors and writes state `+0x94=0` |
| `+0xc0` input/texture descriptor count | `0` | no texture instruction; `0x001fb7af` copies state `+0x34=0` |
| `+0x1bc` input-use mask | `0` | result initialized to zero; wrapper's per-descriptor loop is skipped at count 0 |
| `+0x1c0` secondary pointer | null | zero count and no allocation |
| `+0x1c4` secondary instruction count | `0` | `0x001c95f7` → `0x001fb7af` |
| `+0x1c8` primary-attribute limit/result count | `1` | `0x001fb7af` writes state `+0x670 + 1`; state `+0x670` stays zero |
| `+0x1cc` selected compiler mode | `1` | converter `0x00033490` → wrapper `0x0003503d` |
| `+0x1d0` mode-2-only value | `0` | wrapper explicitly writes zero when mode is not 2 |
| `+0x1d4` constant-map entry count | `0` | no surviving constant source; `0x001fb7af` mapping table count is zero |
| `+0x1d8` register-constant count | `0` | `0x00235ca2` records no constant use at state `+0x5a0`; `0x001fb7af` copies it |
| `+0x1dc` parameter bytes | `0` | converter's empty Mesa parameter list → wrapper copy |

The terminal-record predicate `0x001c7187` returns true for opcode marker `0x5a` with zero deschedule and zero operand, so finalization sets state `+0x3c` bit `0x20` when both phase counts are zero. The same predicate clears bit `0x40` in the second check. The no-predicate path of `0x001cc814` does not create a predicate instruction or set a mapped result flag. `0x000370d8` consequently takes its `compiled_flags & 2` branch; the subsequent fragment-link and pixel-state construction remain **FG-02**, outside this report.

For this exact input, the independent construction rule is:

```text
records = [MOV(primary_color, output_color, xyzw), END]
lower MOV into four scalar virtual-temp writes
seed live-out = { output-class register 0: xyzw }
remove each scalar write whose temporary-class destination is not live
assert no selected post-END conversion, conditional insertion, or late mode-2 copy
count remaining instruction nodes; result = 0
return main_bytes = empty, secondary_bytes = empty, flags = 0x00000002,
       temporary_count = 0, primary_attribute_count = 0,
       parameter_bytes = 0, selected_mode = 1
```

The rule reproduces the *compiler output* for the frozen input. It is not a general USC compiler, a linked-program specification, a hardware-safe access contract, or proof that the historical driver would render correctly on the target machine. Gate B remains **BLOCKED**; hardware functionality remains **UNVERIFIED**.
