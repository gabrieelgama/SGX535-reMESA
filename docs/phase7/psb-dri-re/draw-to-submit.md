# Historical GL draw to TA submission: static control flow

This extends [the earlier path map](command-submission.md). Addresses are `psb_dri.so` ELF virtual addresses (SHA-256 `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`); the disposable Ghidra view adds `0x10000`. The links below describe **historical binary behavior**, not a tested SGX535 execution path. Selected decompilations were produced with [DecompileSelected.java](../../../tools/psb-dri-re/DecompileSelected.java); the original bytes can also be checked with a static i386 disassembler.

## The previously missing indirect edge

`0x000371f0`, reached from the DRI `CreateContext` callback `0x0004bd92`, stores the 0x68-byte renderer table returned by `0x00026678` in context-derived field `+0x728` (raw i386 displacement `0x13b84`). Its installed entries include `+0x08 → 0x00026dbd` (assertion identifies `pbr_start_render`), `+0x18 → 0x00026a0f`, `+0x1c → 0x00026896` (assertion identifies `pbr_draw_indexed_prim`), and `+0x28 → 0x0002673d`. `0x00048592` creates a separate frame-tracker object through `0x00051830` and stores it at context-derived `+0x730` (raw displacement `0x13b8c`). These assignments and offsets are **CONFIRMED** binary facts (P7E-001). This is a driver-private table; the upstream Mesa 7.4.4 `tnl_device_driver.Render` layout in `src/mesa/tnl/t_context.h:410–480` is different and is not used to name these slots.

`0x00051860` has an embedded `intel_frame_tracker.c` assertion identifying its state-transition role. When the old mode is `3`, it loads the renderer pointer from context `+0x728` and calls its `+0x28` entry before entering the new mode. The table store above resolves that indirect call to `0x0002673d`. In mode `3`, entry calls the same table's `+0x08` entry, resolving to `0x00026dbd`. Mode `4` uses a distinct pointer at `+0x724`; its full path is outside this GL/TA chain. The conditional calls, argument `new_mode == 1`, and mode values are **CONFIRMED** binary facts (P7E-002). The exact hardware meaning of modes `0–5` remains UNKNOWN.

```text
DRI CreateContext 0x4bd92 → context setup 0x371f0
  → renderer table 0x26678 → context +0x728
  → frame tracker 0x51830 → context +0x730

frame-tracker transition 0x51860, enter mode 3
  → renderer +0x08 = 0x26dbd, pbr_start_render
  → create scene 0x27060 → store context +0x81c (raw 0x13c78)

renderer +0x18 = 0x26a0f or +0x1c = 0x26896
  → selected indexed path: scene at context +0x81c → 0x27fa0 scene draw-elements
  → 0x3b890 TA index-list output and relocations

frame-tracker transition 0x51860, leave mode 3
  → renderer +0x28 = 0x2673d
  → scene at context +0x81c → 0x2a39a finalization
  → 0x37b51 → drmCommandWrite(fd, index 0, 0x90 bytes)
  → clear context +0x81c
```

`0x00026dbd` asserts that the scene slot is empty, obtains color/depth state, creates a scene through `0x00027060`, and stores its pointer at `+0x81c`. `0x00026896` sends indexed primitives to `0x00027fa0` using that slot. `0x0002673d` passes the same slot to `0x0002a39a`, then clears it. `0x0002a39a` finalizes TA state, emits an eight-byte output record beginning with `0xc0000000`, invokes `0x00037b51` with selector `0`, and frees the scene. The wrapper maps selector `0` to value `3` in its 0x90-byte argument; `PSB_ENGINE_TA` is `3` in the *candidate* 4.41.1 `psb_drm.h:161–165` (P7E-003). This is a **strong static end-to-end control-flow path** from the private draw callback through historical userspace submission, conditional on the same context entering and later leaving mode `3`. It does not establish every GL dispatch or GPU command semantic.

The clipping frontend supplies another intermediate edge (P7E-007). `0x0002d124` selects the renderer at raw context `+0x13b84` or an alternate at `+0x13b80` and calls `clip_set_render` (`0x000dd16d`); the selected pointer is retained at clip-context `+0xf8`. `clip_pipe_emit` later copies that pointer to pipe field `+0x103c` (`0x000dea86`) and calls its `+0x1c` slot at `0x000dea09`. In the selected mode-3 case, that slot resolves to the private indexed-draw callback `0x00026896`. `clip_vb` contains further `+0x18/+0x1c` calls through a rendering interface. Selection and pointer propagation are **CONFIRMED** binary behavior; the exact Mesa GL entry → `clip_vb`/`clip_pipe_emit` dispatch for a particular draw is still UNKNOWN.

P7F-010 constrains one earlier indirect edge: `0x0002d124` is the callback at `+0xc` in the 16-byte state atom at ELF VA `0x002b3368`. Its second dirty mask is `0x74`; a static pointer array contains its address at `0x002bb208`. `0x0004f3ef` invokes atom callbacks when context dirty words `+0x690/+0x694` intersect their masks. `0x0002b3d4`, called by context creation `0x000371f0`, copies the 17-pointer static array containing this atom into context `+0x698` and sets count `+0x69c` to `0x11` (P7F-011). The conditional atom-dispatch edge is therefore **CONFIRMED** within the retained binary. Which GL call sets the relevant dirty bits and reaches this clipping path remains UNKNOWN. The exact frontend draw edge remains UNKNOWN.

## Submission triggers visible in the binary

| path | condition and effect | confidence |
|---|---|---|
| `0x000477b3` swap callback | `_mesa_notifySwapBuffers`; transition to mode `1` before the separate copy path `0x0003aa2a`. An old mode `3` finalizes first. | CONFIRMED; DRI role from the [driver API table](dri-extension-map.md) |
| `0x000483a3` flush helper | requests mode `0`; `0x000483d4` DRI `UnbindContext` reaches it. Other direct callers exist. An old mode `3` finalizes. | CONFIRMED call chain; exact GL `Flush` slot for every caller not yet mapped |
| `0x000473bd` framebuffer attachment change | if tracked color or depth pointer changes, requests mode `0` before updating that pointer and dirty bits `0x80`/`0x100`. | CONFIRMED condition and writes; bits are binary values, not adopted hardware names |
| `0x00026c9f` vertex allocation failure | retries after `0x00051a1f`, which switches to mode `0` and restores the prior mode. An old mode `3` finalizes and new mode `3` starts a new scene. | CONFIRMED branch and state calls |
| `0x00026896` scene-draw failure | the first failed `0x00027fa0` attempt uses the same `0x00051a1f` transition and retries. The exact failure reason can include capacity/state and is not proven for every case. | CONFIRMED retry; cause per path UNKNOWN |
| `0x000483ef` context destruction | requests mode `0` before tearing down Mesa/private context state. | CONFIRMED |
| `0x000493d1` | calls `0x0004926d`, the mode-0 flush helper, an outbuf/fence-related routine, and another helper. A `Finish` interpretation is plausible but its exact Mesa callback slot is not proven here. | CONFIRMED calls; role INFERRED |
| `0x00048e50` / `0x0004f2d3` | select mode `3` or `4` based on renderer selection, or request mode `4`; leaving an old mode `3` can finalize. | CONFIRMED calls; higher-level trigger partly UNKNOWN |

There is **no unconditional submit on every indexed draw** in `0x00026896`: successful `0x00027fa0` returns without `0x00051a1f`. A particular draw can still trigger submission through the failure/retry path. The mode transition, not a direct draw→submit call, is the missing edge resolved in this pass. The clipping frontend narrows callback invocation, and [P7G](frozen-draw-closure.md) later traces the historical DrawElements entry into that pipeline. Required dirty-state outputs remain incomplete. Upstream Mesa 7.4.4 `t_vb_render.c:262–325` documents a generic TNL render sequence; it does not prove this private 0x68-byte table is a TNL callback table.

## Synchronization and lifetime boundary

`0x0002a39a` can receive a fence-related object, consumes the submit result, updates a retained fence slot, closes output buffers, and frees the scene. The candidate 4.41.1 kernel handler at `psb_sgx.c:1237–1430` validates BOs, applies relocations, acquires a scene pool, dispatches TA work, and returns fence feedback. Matching request index, size and `5.0.1.0046` label make this a strong **release-family correlation**, not an authenticated exact binary/kernel pair. Fence ordering, error retry beyond `-EAGAIN`, and complete scene ownership rules remain UNKNOWN. Current gma500 offers no established compatibility with this historical request.

