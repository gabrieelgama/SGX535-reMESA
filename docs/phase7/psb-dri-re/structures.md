# Incremental structure observations — checkpoint G

Only fields with an access in the shipped binary and an independently identified historical ABI are named. Unlisted offsets remain UNKNOWN; these are analysis layouts, not new C declarations for reuse.

## DRI screen object

`DRI_Legacy.createNewScreen` at ELF VA `0x00021e0e` allocates `0xd4` bytes, copies 16 consecutive pointers from the immutable driver API table at `0x002b9660` into object offsets `+0x04` through `+0x40`, writes an initial extension-list pointer at `+0x44`, and stores the fd at `+0x70`. The `DRI_Core.getExtensions` callback at `0x000213a0` returns `[screen+0x44]`. These are CONFIRMED disassembly/decompiler observations, correlated with Mesa 7.4.4 `dri_util.c:697–750,804–846` and `dri_util.h:142–230`. The driver-specific initializer at `0x0004be92` assigns the five-entry list at `0x002bb2d0` to that offset after checking the device-private record size.

| object offset | observed access | proposed role | status |
|---|---|---|---|
| `+0x04` | first copied table entry | `DriverAPI.InitScreen` | CONFIRMED by historical ABI and table pointer |
| `+0x1c` | seventh copied entry, invoked indirectly by `DRI_Core.swapBuffers` at `0x00021790` | `DriverAPI.SwapBuffers` | CONFIRMED by field index and Mesa 7.4.4 definition |
| `+0x40` | last of 16 copied table entries | `DriverAPI.InitScreen2` | CONFIRMED layout; value NULL in this binary |
| `+0x44` | set to an empty list initially, later assigned the five-entry list by `0x0004be92`, returned by core `getExtensions` | screen extensions pointer | CONFIRMED |
| `+0x70` | constructor stores fd | DRM fd | CONFIRMED binary access and source correspondence |

The legacy and DRI2 constructors share the published callback layout; the DRI2 path at `0x00021c89` checks the fixed table's `InitScreen2` slot and returns NULL because that slot is zero. This is a static property of this file. It does not prove the older display stack used DRI2.

## Direct PSB command argument

At ELF VA `0x00037b51`, the binary initializes a 144-byte stack block before the `drmCommandWrite(fd, 0, &block, 0x90)` call at `0x00037fb4`. The following field roles use the [historical PSB header](../../poulsbo-data/PSB_psb_drm_h.txt):221–263 for names; binary stores and the exact command call establish the offsets. A source/binary ABI-version match is still INFERRED.

| block offset | historical field | binary observation | classification |
|---|---|---|---|
| `+0x00` | `buffer_list` low word | address of a built validation-list chain | CONFIRMED store; pointer role from header INFERRED |
| `+0x10` | `scene_arg` low word | caller argument | CONFIRMED store; role from header INFERRED |
| `+0x18` | `fence_arg` low word | pointer into submit object | CONFIRMED store; role from header INFERRED |
| `+0x20` | `ta_flags` | constant `3` | CONFIRMED value; flag names from header INFERRED |
| `+0x28`, `+0x2c` | `ta_offset`, `ta_size` | derived offset and count/4 | CONFIRMED calculations; semantic role INFERRED |
| `+0x48`, `+0x4c`, `+0x50` | `reloc_handle`, `reloc_offset`, `num_relocs` | handle/offset from submit object, count from pointer difference | CONFIRMED data flow; names INFERRED |
| `+0x58`, `+0x5c` | `fence_flags`, `engine` | caller flags and branch-selected `3`/`2`/`0` | CONFIRMED stores; engine interpretation INFERRED |
| `+0x74` through `+0x8f` | `sVideoInfo` | within transmitted 0x90-byte block | exact field use UNKNOWN without full field-by-field trace |

The function retries the direct request on `-11`, then processes return status and a fence-related path. This is historical userspace behavior, not a safety or recovery guarantee. The relation between these fields, kernel validation and SGX hardware submission needs further cross-check against the exact paired kernel build.

## Screen-private and output-buffer fields

At `0x0004be92`, the driver-specific screen initializer checks `[screen+0x98] == 0x20` before consuming the device-private record at `[screen+0x94]`, allocates a 0x50-byte private object, stores it at `[screen+0xb4]`, and assigns the screen extension list at `[screen+0x44]`. This is the binary's expected userspace record size, not proof that the modern Dell kernel supplies such a record.

At `0x00038762`, the historical output-buffer allocator reads alignment from `outbuf+0x14`, mapped base from `+0x2c`, cursor from `+0x30`, and capacity from `+0x28`; it stores an active allocation length at `+0x34` and an output record pointer at `+0x38`. `0x000384ab` consumes the record and closes the allocation. Those offsets are CONFIRMED binary accesses; fuller type/layout ownership remains INFERRED.

At `0x0004297b`, a 0x48-byte public pool object points at a 200-byte internal object and invokes `drmBOCreate` and `drmBOMap`. It allocates `count * 0x1c` bytes for linked per-buffer records. This confirms a pool/allocation pattern, not the exact kernel-side BO representation. The complete scene, context, shader-result and command-buffer structures still require analysis of all writers and readers before writing C declarations.

## Render dispatch and scene slots

The [draw-to-submit trace](draw-to-submit.md) constrains several further fields in the historical context and clip objects. These offsets describe binary accesses, not a complete recovered C structure.

| object/offset | binary access | supported role | status |
|---|---|---|---|
| render table `+0x08` | installed by `0x00026678`, called by `0x00051860` on entry to mode 3 | start render (`0x00026dbd`) | CONFIRMED |
| render table `+0x18` | installed by `0x00026678`, called through the clip path | draw callback (`0x00026a0f`); exact input semantics UNKNOWN | CONFIRMED callback identity |
| render table `+0x1c` | installed by `0x00026678`, called by `0x000dea09` when this table is selected | indexed draw callback (`0x00026896`) | CONFIRMED |
| render table `+0x28` | installed by `0x00026678`, called by `0x00051860` on exit from mode 3 | scene finalizer (`0x0002673d`) | CONFIRMED |
| context `+0x728` (raw displacement `0x13b84`) | table stored by `0x00026678`, selected by `0x0002d124` | mode-3 render table pointer | CONFIRMED |
| context `+0x730` (raw displacement `0x13b8c`) | stored by `0x00048592`, consumed by mode-transition calls | frame tracker pointer | CONFIRMED |
| context `+0x690/+0x694` | initialized to `0xffffffff` by `0x00048592`, tested and cleared by `0x0004f3ef` | two dirty-state words | CONFIRMED |
| context `+0x698/+0x69c` | written by `0x0002b3d4` during context setup; read by `0x0004f3ef` | 17-entry state-atom pointer list and count | CONFIRMED |
| state atom `0x002b3368` | relocation at `+0xc` resolves to `0x0002d124`; second mask at `+8` is `0x74` | conditional renderer-selection callback | CONFIRMED |
| context `+0x81c` (raw displacement `0x13c78`) | stored by `0x00026dbd`, read by `0x00026896` and `0x0002673d`, cleared by `0x0002673d` | current scene pointer | CONFIRMED |
| clip `+0xf8` | selected render table stored by `0x000dd16d` | render dispatch pointer | CONFIRMED |
| clip pipe `+0x103c` | propagated by `0x000dea86`, used by `0x000dea09` | render dispatch pointer | CONFIRMED |

The selected render table is separate from Mesa's generic TNL `Render` structure. Field roles above follow pointer stores and indirect calls; they do not establish hardware semantics for the produced records.
