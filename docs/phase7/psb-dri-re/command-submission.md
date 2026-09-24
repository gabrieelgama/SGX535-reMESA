# Static command path — checkpoint E

This file reconstructs one path through the shipped `psb_dri.so`, not a working driver or a hardware-safe test. Addresses are **ELF virtual addresses**; the disposable Ghidra project's displayed addresses add `0x10000`. The target bytes have SHA-256 `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`.

**Later static closure:** [draw-to-submit.md](draw-to-submit.md) resolves an indirect frame-tracker edge omitted by the direct call graph: mode-3 exit calls the installed renderer's `+0x28` finalizer, then this file's scene submit path. The paragraphs below retain the earlier checkpoint's direct-call observations; their “unresolved trigger” wording describes that earlier checkpoint, not the current result.

## A traced swap path

The binary and the historical Mesa 7.4.4 ABI support this chain:

```text
DRI_Core.swapBuffers descriptor, 0x002b9620 + 0x24
  -> 0x00021790 DRI glue
  -> screen DriverAPI.SwapBuffers, table 0x002b9660 + 0x18
  -> 0x000477b3 driver callback
  -> 0x0003aa2a, assertion source psb_swapbuffers.c (copy path)
  -> 0x000464e0, assertion source intel_blit.c (2D copy builder)
  -> 0x00037b51, PSB submit wrapper
  -> drmCommandWrite(fd, 0, stack argument, 0x90) at 0x00037fb4
```

The descriptor/table pointers are CONFIRMED binary data; the callback roles follow the matching [Mesa 7.4.4 ABI](dri-extension-map.md). The direct edges after `0x000477b3` are in the [call graph](analysis/callgraph.csv), and the selected function bodies were decompiled in a disposable project. At `0x0003aa2a`, the path can wait on a fence, handle drawable lock state and copy clip rectangles. One branch calls `0x000464e0`; another calls `0x00046924`, which remains less analyzed. The `intel_blit.c` branch reserves a 0x28-byte output block, writes command words, records relocations and invokes the submit wrapper. That is a **2D copy path** in the historical binary, not proof of SGX 3D rendering.

At `0x00037b51`, the wrapper constructs a 0x90-byte command argument, links validation entries, supplies buffer handles/offsets and a relocation count, selects one of several engine values, retries request index 0 on `-11`, and processes a fence-related result. The bytes passed to libdrm are CONFIRMED; [the historical PSB header](../../poulsbo-data/PSB_psb_drm_h.txt) lines 221–263 and 338–345 provides an INFERRED `DRM_PSB_CMDBUF`/`drm_psb_cmdbuf_arg_t` correspondence. The exact kernel version paired with this binary is UNKNOWN. The current display-only gma500 is not presumed to accept this ioctl.

## A separate scene/TA submit path

Static call edges also establish a scene-finalization path:

```text
0x0002673d (caller with render/context state)
  -> 0x0002a39a (scene finalization)
  -> 0x0003b73b (TA/PDS-related state emission)
  -> output-buffer reservation, final state word, buffer release
  -> 0x00037b51 (submit wrapper, engine-selection argument 0)
  -> drmCommandWrite(fd, 0, stack argument, 0x90)
```

`0x0002a39a` conditionally sets a scene flag, flushes pending state, emits a TA state record, reserves an eight-byte output block, writes `0xc0000000`, then passes its scene/outbuf objects and an optional fence to `0x00037b51`. The exact hardware meaning of `0xc0000000` is UNKNOWN. The emitted words and call arguments are CONFIRMED binary behavior; the semantics of the kernel command and its SGX execution remain only partly reconstructed.

A draw-side branch is independently visible: `0x00026896` (embedded `psb_render.c` assertion `pbr_draw_indexed_prim`) calls `0x00027fa0` (embedded `psb_scene_draw.c` assertion `psb_scene_draw_elements`), which calls `0x0003b890` (`psb_ta_index_list`) to reserve a `0x14`-byte index-list record and attach relocations. Scene setup at `0x00029fbd` can invoke vertex/PDS upload at `0x00040355` and the same index-list builder. These are high-value pieces of a historical TA path. The exact control/data-flow connection from a specific GL entry point to scene finalization has not been completed; sharing a scene object is not enough to claim a full draw-to-submit chain.

The later static pass confirms the shared **scene-pointer slot** but not a call edge: `0x00026896` reads the context-derived field at `DWORD_ARRAY_0001345c + context_index + 0x81c` and passes it to `0x00027fa0`; `0x0002673d` reads that same expression, passes the result to `0x0002a39a`, then clears the slot. The indexed-primitive caller `0x00026a0f` reaches `0x00026896`. No direct call-graph path from `0x00026896` to `0x0002673d` or `0x00037b51` exists in the exported graph. **INFERRED:** draw and finalization operate on a common retained scene under compatible context state. The trigger and ordering that make a particular GL draw reach that finalization remain **UNKNOWN** (P7D-008).

A read-only Ghidra XREF pass found that `0x00026678` installs `0x00026a0f`, `0x00026896`, and `0x0002673d` at offsets `+0x18`, `+0x1c`, and `+0x28` in one 0x68-byte callback table. `0x000371f0` calls that initializer and stores the table at the context-derived `+0x728` slot. The driver API table at `0x002b9660 + 0x08` points to `0x0004bd92`, which calls `0x000371f0`; Mesa 7.4.4 `dri_util.h:153–157` names this driver API slot `CreateContext`. These pointers, stores and calls are **CONFIRMED** historical binary/ABI facts (P7D-010). They establish that draw and finalization callbacks are installed together during context creation. They still do not establish which Mesa dispatch invokes each callback, the required state transitions, or the ordering from one draw to the final index-0 request.

## Buffer and synchronization evidence

`0x0004297b` allocates an object and invokes `drmBOCreate`, `drmBOMap`, `drmBOUnmap`, and on failure `drmBOUnreference`. It allocates a pool of fixed-size per-buffer records and installs function pointers in a 0x48-byte vtable-like object. This is CONFIRMED userspace buffer-manager behavior; the kernel request numbers inside this particular `libdrm.so.2` are not in `psb_dri.so`. Other imports include `drmBOSetStatus`, `drmBOWaitIdle`, and `drmFenceWait`; [import callers](analysis/import-callers.csv) give entry points. The swap path uses `drmGetLock`/`drmUnlock` and fence operations, but their exact synchronization contract with the historical kernel needs the paired libdrm and kernel sources.

`0x00038762` (`psb_outbuf.c` assertion references) reserves aligned output space, grows/rotates its backing buffer when needed, and returns a mapped CPU pointer plus a buffer handle and offset. This supports a userspace command-buffer construction model. A reconstructed buffer format must still distinguish the 2D copy commands, TA/3D state commands, relocations and the kernel command argument. No instruction or GPU packet format is adopted here from numeric constants alone.

`0x00037854` (`psbSetOffsetRelocation` in an embedded assertion) reserves a relocation record, checks alignment/mask inputs, writes either a computed offset word or a placeholder, and fills fields in the relocation record. `0x000384ab` closes an outbuf allocation and checks the associated relocation capacity. These are CONFIRMED binary actions; their field names and validity rules still need cross-checking with the exact paired kernel header.

## Limits

These paths establish that the historical library **contains** both a 2D copy submit path and a scene/TA submit path to the PSB/libdrm boundary. They do not establish that the 2009 binary matches the Dell's installed 5.10 gma500 ABI, that either command can safely run, or how SGX power/reset/firmware prerequisites were met. The complete 3D path from a GL draw through all scene-state transitions and SGX execution remains only partially traced. No command was submitted in this investigation.
