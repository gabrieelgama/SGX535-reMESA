# Independent program-base fields: scope and bootstrap gap

The original `remesa_program_registers.c` codec packs the USE base register's
raw address field and data-master selector, validates its 16 bank offsets,
and packs a raw aligned PDS execution-base register value. It performs no
register access, upload, kick, shader instruction generation or rendering.
The actual Gallium shader-program report exposes this evidence separately
from its still unsupported machine-code compilation result. The isolated
Mesa 25.2.0 build compiles the codec; OpenGL admission is unchanged.

The shader qualifier also links its NIR/callback test against the actual
Meson-built `libremesa_loader.a` and Mesa libraries, not recompiled substitute
driver objects. It checks staged source identity, requires an up-to-date
archive without rebuilding it, and rejects archive/input changes during the
test. The offline fixture exercises immutable program reports, context/shader
ownership, allocation failures and teardown. It generates no GPU instructions.
`--archive-only` runs that test without repeating the native/UBSan variants.

Numerical register facts come from the GPL-2.0-only upstream Linux v5.10
gma500 sources, pinned to commit
`2c85ebc57b3e1817b6ce1a6b703928e113a90442`. Source hashes, exact locators,
PCI attribution and limits are in
`sgx535-launch-field-evidence-20261009.json`. Implementation logic is original;
no upstream driver implementation or shader words were copied.

| Fact | Publication evidence | SGX535 rev121 qualification |
| --- | --- | --- |
| 16 USE base banks, raw address bits 24:0, master bits 26:25 | CONFIRMED | UNKNOWN |
| USE byte VA converted by `VA >> 7` | INFERRED from the header's uncertain alignment comment | UNKNOWN; conversion deliberately absent |
| PDS base is raw aligned address, not `VA >> 20` | CONFIRMED by the header and direct upstream `0x20000000` write | UNKNOWN |
| USSE/PDS instruction length, opcode and operands | UNKNOWN | UNKNOWN |
| Shader termination/export and PDS task words | UNKNOWN | UNKNOWN |

Round trips and arithmetic tests qualify this codec's handling of the public
fields. They do not qualify address reach, PFNs, mappings, instruction
alignment or shader execution. A valid register word is never an admitted
executable program. Register-field evidence flags for VA packing, rev121
verification and known instruction fields remain zero.

## New architecture evidence and preservation experiments

The public contemporaneous Imagination patent US20070101013A1 supplies
architecture semantics, not a product-specific ISA specification. Its
paragraph [0047] distinguishes a thread's Last marker from freeing output;
[0054] associates output release with consumer completion; [0069] describes
fragment work on persistent tile output followed by event-driven emission.
Those publication facts are CONFIRMED. Applicability to SGX535 rev121 remains
UNKNOWN. No Last bit, export instruction, PDS word or load/store encoding is
derived from these descriptions. See `sgx5-field-research-20261009.json`.

This distinction sharpens two future experiments:

1. **Thread end versus output retirement:** an independently authored,
   documented launch/termination program must produce a sentinel export.
   Observe thread completion separately from output consumption and retained
   backing release. A completion interrupt alone cannot prove publication.
2. **External COLOR versus persistent tile output:** seed retained COLOR,
   independently verify its GPU-visible mapping, render a known bounded
   region, and compare every untouched pixel before and after render.
   Separately test sequential operations; persistence within a tile task
   cannot establish persistence across submissions. An unseeded control and
   stale-resource rejection are required. No CPU reconstruction is permitted.

Neither experiment is executable yet. Its bootstrap must be an independently
justified USSE instruction format including valid termination/export, a PDS
launch/data format, verified program-address mapping and qualified observer
admission. There is no safe basis for opcode brute force or speculative
instruction submission. A passive qualified observer snapshot could compare
current rev121 USE/PDS base registers with the public initialization facts;
it would not resolve unknown ISA fields. No hardware was contacted here.

## Exact remaining executable compiler dependency

The existing restricted NIR-to-semantic-IR lowering remains usable, but no
admitted source supplies USSE instruction width/endianness, opcode positions,
register bank/index fields, predicates, scheduling controls, termination,
exports, or PDS instruction/task/data encoding. Consequently no independent
hardware-executable shader has been produced and OpenGL remains BLOCKED
with admission mask `0x1f`. Host semantic serialization is never uploaded.

No `psb_dri.so`, `Xpsb.so`, historical 440-byte payload, proprietary shader
binary or disputed DDK was opened or used. Other-generation compiler/corpus
search hits were not admitted as instruction evidence.
