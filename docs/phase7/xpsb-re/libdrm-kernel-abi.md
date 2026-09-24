# Candidate historical libdrm → PSB kernel ABI

Both retained ELFs import old `drmBO*` and `drmFence*` symbols, and `Xpsb.so` additionally imports `drmBOVersion` and `drmCommandNone` (P7B-006/P7C-002). The public `libdrm-poulsbo` 2.3.0 source archive supplies **source correlations**, not an authenticated copy of the `libdrm.so.2` used to build or run either retained ELF. See [provenance](source-provenance.md). The source's `libdrm/xf86drm.c` signatures and `shared-core/drm.h` request definitions fit the imports and 32-bit argument shapes (P7C-008).

| observed imported helper | candidate source request number | candidate source argument | source locations |
|---|---:|---|---|
| `drmGetVersion` | `0x00` | `drm_version` | `xf86drm.c:655–686`; `drm.h:1036` |
| `drmGetLock` / `drmUnlock` | `0x2a` / `0x2b` | `drm_lock` | `xf86drm.c:1249–1285`; `drm.h:1072–1074` |
| `drmWaitVBlank` | `0x3a` | `drm_wait_vblank` | `xf86drm.c:1855–1864`; `drm.h:1088` |
| `drmFenceUnreference` | `0xc7` | `drm_fence_arg` | `xf86drm.c:2413–2424`; `drm.h:642–652,1097–1104` |
| `drmFenceSignaled` | cached or `0xc9` through `drmFenceFlush` | `drm_fence_arg` if flushed | `xf86drm.c:2425–2468`; not every call issues an ioctl |
| `drmFenceWait` | cached or `0xca` | `drm_fence_arg` if waited | `xf86drm.c:2546–2574` |
| `drmBOCreate` | `0xcd` | `drm_bo_create_arg` | `xf86drm.c:2596–2624`; `drm.h:776–820,1106` |
| `drmBOMap` | `0xcf` plus conditional fd `mmap` | `drm_bo_map_wait_idle_arg` | `xf86drm.c:2674–2719`; `drm.h:764–838,1107` |
| `drmBOUnmap` | `0xd0` | `drm_bo_handle_arg` | `xf86drm.c:2723–2734`; `drm.h:822–824,1108` |
| `drmBOReference` / `drmBOUnreference` | `0xd1` / `0xd2` | `drm_bo_reference_info_arg` / `drm_bo_handle_arg` | `xf86drm.c:2627–2664`; `drm.h:825–831,1109–1110` |
| `drmBOSetStatus` | `0xd3` | `drm_bo_map_wait_idle_arg` | `xf86drm.c:2737–2762`; `drm.h:1111` |
| `drmBOWaitIdle` | cached or `0xd5` | `drm_bo_map_wait_idle_arg` if queried | `xf86drm.c:2784–2803`; `drm.h:1113` |
| `drmBOVersion` | `0xd6` | `drm_bo_version_arg` | `xf86drm.c:2882–2900`; `drm.h:876–880,1114` |

These are `_IOC_NR` values in the **candidate source**. The source uses `DRM_COMMAND_BASE = 0x40` (`drm.h:1141`): direct PSB indices 0, 1, 2 and 3 thus address DRM command numbers `0x40`, `0x41`, `0x42` and `0x43` through libdrm wrappers. Full `_IOC` words also encode direction and structure size. The archived PSB kernel dispatch macros do not always have the same direction annotation as the userspace helper name; matching by index alone is not proof of runtime compatibility.

For Linux i386's `_IOC` encoding, the **candidate source implementation** computes `drmCommandWrite(index=0,size=0x90)` as `0x40906440`, `drmCommandWrite(index=1,size=0x14)` as `0x40146441`, `drmCommandNone(index=2)` as `0x00006442`, and P7B's `drmCommandWriteRead(index=3,size=0x14)` as `0xc0146443` (`xf86drm.c:2243–2252,2300–2339`). These are calculated source correlations; the retained ELFs pass index and size to an external library and do not themselves expose the final ioctl word. The archived kernel's `XHW_INIT` descriptor is declared with `DRM_IOR` (`psb_drv.c:88–108`), unlike the candidate `drmCommandWrite` caller's direction bits. The bundled core dispatches driver-private requests by `_IOC_NR` and explicitly disables full-word matching; it copies according to the caller's input/output bits (`drm_drv.c:607–659`). The candidate XHW-init handler consumes the supplied input and does not write a reply (`psb_xhw.c:416–470,516–528`). This resolves the *direction* concern for that request **within the candidate source tree**, but does not authenticate the linked libdrm/kernel binary pair or show runtime success.

The candidate `drm_bo_op_arg` (`drm.h:840–859`) chains BO validation requests, while the archived PSB kernel's `psb_validate_buffer_list` reads those elements and requires `drm_bo_validate` (`PSB_psb_sgx_c.txt:410–450`). The same archived handler looks up command, relocation, TA and OOM BOs; applies relocations; selects 2D/video/TA branches; manages scenes; and writes fence feedback (`PSB_psb_sgx_c.txt:1237–1430`). This is an **INFERRED source correlation** to the retained binaries' command/validation paths, because the exact paired kernel build and generic libdrm binary are absent. The current gma500 driver is a separate display driver and is not asserted to implement this legacy ABI.

For the exact binary-side direct calls, see [the Xpsb request map](ioctl-map.csv) and [P7B's direct request map](../psb-dri-re/ioctl-map.csv). No ioctl was sent during this investigation.
