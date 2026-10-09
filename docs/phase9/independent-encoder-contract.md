# Independent shader backend contract

The implemented path is Mesa NIR → validated reMESA semantic IR → scalar virtual
USSE plan → immutable shader-state ownership. PDS preparation additionally binds
a symbolic code/data storage request to the exact canonical plan and fresh
operation, generation, owner and mapping metadata. It acquires no GEM references
and cannot authenticate PFNs or GPU completion on its own.

`remesa_usse_serialize_host` and `remesa_usse_deserialize_host` describe an original
little-endian **RUSI host format**, not SGX535 machine instructions. Its 16-byte
header, 24-byte records, virtual register bounds and enum values are original
software limits, not hardware limits. The checksum is noncryptographic and is
never used alone to compare shader plans or admit hardware work.

Supported host semantics are identity float32 position, endpoint constant
fragment color admitted by the existing NIR recognizer, and validated copies.
No arithmetic, texture instructions, predicates, branches, varying linkage or
hardware register allocation is claimed. Uniform color snapshot specialization
remains CPU-side preparation and cannot be queried as a compiled GPU program.

Both hardware emitters return `ENOTSUP` and preserve output bytes and lengths.
The old IR encoder now reaches the same USSE validation/emission boundary;
there is no fallback to historical instruction images. Callback-created shader
handles have context ownership, stale-handle tombstones and immutable snapshots.
Queries and emission attempts do not change binding or submit work.

The evidence inventory records independent primary-source architecture facts
and 23 unknown USSE fields plus 16 unknown PDS fields. It admits zero instruction
constants. No proprietary library or historical 440-byte image was accessed,
decoded, extracted or used. Public Rogue/SGX530 information is not treated as a
SGX535 rev121 encoding specification.

Required before executable shader programs: independently admitted SGX535
instruction widths/alignment, opcode locations/values, operand/register banks,
predicate/control/end/export rules, and PDS task/address/loading semantics.
Instruction register packing, binary relocation and instruction decoding remain
unsupported until those fields have independent evidence. Encode/decode host
round trips establish serialization correctness only.

The separate `remesa_program_registers` module now packs public upstream Linux
USE/PDS **MMIO base fields**, not shader instructions. Its evidence is exposed
by the actual shader report without admitting a program. Raw USE address-field
packing is supported; conversion from a byte VA is deliberately absent because
the upstream alignment interpretation is only inferred. PDS base packing
preserves the raw aligned address corroborated by upstream initialization.
No register access or launch is implemented by this codec. See
`independent-program-registers.md` for sources, limits and experiments.

OpenGL admission remains `0x1f`. This increment neither enables a physical
endpoint nor changes any hardware admission, provenance or retirement guard.
