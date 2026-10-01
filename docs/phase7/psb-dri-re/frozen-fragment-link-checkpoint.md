# Fragment link record for the frozen draw

**Historical checkpoint:** the [selected compiler result](frozen-fragment-exact-output.md) now supplies the values that this checkpoint previously marked UNKNOWN (`+0xb4=0`, `+0xb8=0x2`, `+0x1d8=0`, `+0x1cc=1`, `+0x1d0=0`). The table below preserves the earlier conditional link formulas. The later [FG-02 trace](frozen-fragment-link-progress.md) substitutes those fields, derives the link-time suffix and secondary fast path, and records the pixel-state work that remains.

The linker at retained DRI ELF `0x000370d8` initializes an `0x88`-byte object to zero, then fills selected fields from the fragment key and compiler result. These are **CONFIRMED binary construction rules**, not completed values for the frozen triangle. Its caller and downstream scene atoms are in the [frozen draw](frozen-draw-closure.md); source identity is the DRI SHA-256 in the [compiler checkpoint](frozen-fragment-compiler-checkpoint.md).

| link-object offset | producer rule | selected-path limit |
|---|---|---|
| `+0x08…+0x17` | copy first 16 bytes of fragment key | key bytes must be fixed by the selected no-feature state |
| `+0x20` | pointer to compiler result | dynamic CPU pointer, not GPU address |
| `+0x74` | copy compiler result `+0xb4` | actual register/resource value UNKNOWN |
| `+0x6f` | low byte of compiler result `+0x1d8`, plus one | actual count/value UNKNOWN |
| `+0x71` | `+0x6f` plus low byte at link `+0x70` | `+0x70` is initialized to zero before optional feature helpers; any selected helper effect must be checked |
| `+0x44/+0x48` | if result `+0x1cc == 2` and feature-key bit `8` clear: `(result+0x1d0 << 7) | 0xa0000000`, then `0x28811001` | branch outcome and result `+0x1d0` UNKNOWN |
| `+0x44/+0x48` | a later conditional can instead write `0`, `0xf8000140` when a suffix flag remains clear and either feature-key bit `4` or result `+0xb8` bit `2` is set | condition requires exact compiler flags and key |

Feature-key bit `4` invokes `0x00036f32`; bit `8` invokes `0x000364ed`. The no-texture/no-fog/no-specular choice narrows these branches, but the link object cannot be instantiated until the compiler result and exact key bytes are established. `0x000283d8` then caches a `0x128`-byte fragment-state key and six derived words; `0x00028559` caches a `0x150`-byte pixel-state key and calls further writers. Those caches are additional consumers, not proof that the link record alone is the complete pixel state.
