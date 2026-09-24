# Direct PSB/libdrm kernel boundary — checkpoint D

The binary imports `drmCommandWriteRead` and `drmCommandWrite`. Independent `llvm-objdump` disassembly locates one direct call to each: `0x00037529` and `0x00037fb4` (ELF VAs; executable segment file offsets are equal). These calls were **not executed**. See [machine-readable map](ioctl-map.csv).

| binary fact (CONFIRMED) | historical source correlation (INFERRED) |
|---|---|
| At `0x00037529`, arguments are fd, index `3`, caller-supplied pointer, size `0x14` to `drmCommandWriteRead`; caller tests the pointed first word and skips the call if zero. | [PSB `psb_drm.h`](../../poulsbo-data/PSB_psb_drm_h.txt):206–214,338–345 names index 3 `DRM_PSB_SCENE_UNREF`; its `drm_psb_scene` has five 32-bit fields (20 bytes). The integer/size/guard align, but a byte-identical binary-to-kernel ABI version has not been proved. |
| At `0x00037fb4`, arguments are fd, index `0`, pointer to a stack block at `[ebp-0xc4]`, size `0x90` to `drmCommandWrite`. After the call, the code retries on `-11` and handles a nonzero result. | The same header, lines 221–263 and 338–345, names index 0 `DRM_PSB_CMDBUF`. Its `drm_psb_cmdbuf_arg_t` has 144 bytes under the header's `PSB_DETEAR` definition. The binary's stack block, request number and size agree; exact field semantics require data-flow review. |

The historical command struct includes four 64-bit pointers (`buffer_list`, `clip_rects`, `scene_arg`, `fence_arg`), TA/oom/command/relocation buffer handles and sizes, engine/fence/feedback fields, and the `PSB_DETEAR` video tail. The historical relocation structure and operations are in [the same header](../../poulsbo-data/PSB_psb_drm_h.txt):120–153. These source declarations are an ABI comparison, not proof that this binary initializes every field as the header suggests. [Structures](structures.md) tracks offsets only after matching binary accesses.

The binary also imports old libdrm BO/fence routines including `drmBOCreate`, `drmBOMap`, `drmBOSetStatus`, `drmBOWaitIdle`, and `drmFenceWait`. Those calls may issue ioctls inside `libdrm.so.2`; their kernel request numbers and exact library version cannot be inferred from this ELF's import names alone. The complete PSB request map remains open until all call sites and dependencies are checked.

The [import-caller inventory](analysis/import-callers.csv) locates direct BO creation at `0x0004297b` and `0x000436cc`, mapping at `0x0004297b` and `0x0004383e`, status update at `0x0004345a`, idle wait at `0x00043678`, and fence wait at `0x00044097`. It also records `drmWaitVBlank`, `drmGetLock`/`drmUnlock`, `drmFenceSignaled`/`drmFenceUnreference`, BO reference/unreference, `drmGetVersion`/`drmFreeVersion`, `drmUnmap`, and `drmCloseOnce`. These are CONFIRMED imported call sites, not a list of kernel ioctl numbers. The sole `mmap` import caller identified in this ELF is named `_mesa_exec_malloc` and is not, by itself, a PSB BAR mapping.

`0x00037854` builds a relocation record before command submission. Its embedded `psbSetOffsetRelocation` assertions and stores establish a mask, shifts, pre-add, destination word and referenced buffer relationship. The function advances the relocation cursor by `0x28` bytes and visibly writes fields through word index seven; the remaining bytes need a field-by-field review. Do not substitute the historical header's field labels without an exact paired ABI check.

The current gma500 display driver is not assumed to implement this historical 3D ABI. Nothing in this analysis authorizes submitting the recovered command structure to the Dell.
