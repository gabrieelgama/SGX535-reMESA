# XHW setup required by the candidate first TA submit

This is a static correlation, not an initialization recipe for current hardware. The retained `Xpsb.so` is SHA-256 `da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f`; ELF addresses below are unrebased. The candidate 4.41.1 kernel source is a strong `5.0.1.0046` release-family match, not an authenticated built pair.

**CONFIRMED in the Xpsb ELF:** `XpsbInit` at `0x2690` creates and maps a `0x84`-byte communication BO before its pool and SGX initialization calls. After those calls succeed, disassembly at `0x2c27–0x2cdc` sets the first word of a 20-byte stack request to zero, sets the second word to the communication BO's handle, and calls `drmCommandWrite(fd, 1, request, 0x14)`. No store to the three remaining stack words appears in the function before this call. On success it creates and maps another `0x88`-byte BO and starts `XpsbThread` at `0x30f0`. The historical request does **not** establish deterministic values for its unused 12 bytes.

**CONFIRMED in the candidate kernel source:** `psb_drm.h:265–273` declares `drm_psb_xhw_init_arg` as five 32-bit fields when `PSB_DETEAR` is enabled: operation, buffer handle, temporary BO handle, framebuffer pointer, framebuffer size. `psb_xhw.c:516–528` dispatches operation zero to `psb_xhw_init_init`; that handler at `:416–470` reads the buffer handle, maps the communication BO, rejects device-memory placement, then sets `xhw_on` and `xhw_submit_ok`. It does not read the optional three fields on the operation-zero path. Thus the uninitialized trailing request bytes do not affect **this candidate handler**. This is not proof that a different paired kernel ignores them.

The candidate `drm_psb_xhw_arg` at `psb_drm.h:293–344` occupies `0x84` bytes on i386: 16-byte header, 64-byte cookie and a 52-byte largest union member. That exactly matches the retained `XpsbInit` communication BO allocation. `XpsbThread` reads its operation at BO `+0x00`, result at `+0x04`, cookie at `+0x10`, and union at `+0x50`. Its operation 1 calls `Xpsb_scene_info` (`0x3a40`), and operation 2 calls `Xpsb_scene_switch_fire` (`0x4550`). The candidate constants are `PSB_XHW_SCENE_INFO=1` and `PSB_XHW_SCENE_BIND_FIRE=2` (`psb_drm.h:350–363`); `psb_scene.c:128–132` requests scene info, and `psb_schedule.c:199–207` requests bind/fire. Field, size and operation agreement strengthen the compatible-family correlation. They do not authenticate exact pairing or successful hardware execution.

`Xpsb_scene_info` (`0x3a40`) branches on retained Xpsb state at context `+0x64`. `Xpsb_set_vopt` (`0x4af0`) reads the 32-bit word at its SGX mapping `+0x14`, computes `100 + low_byte + 10 * second_byte`, and writes option `+0x64 = 1` on its `0x6b`, `0x6c` or `0x6d` cases. `XpsbInit` passes the **same** `private+0x8c` pointer to `Xpsb_set_vopt` (`0x2c0f`) and the later XHW thread (`0x30f0`), which passes it to `Xpsb_scene_info`. This is a historical register-derived software input, not a measurement of the Dell's SGX revision or evidence that reading the register is safe.

For the frozen **32×32** target, both `+0x64` branches happen to converge. The top-level small-target branch is taken regardless of context `+0x5c`; rounded tile counts are `2,2`. Both option branches produce `local_10=4`, `uVar3=4`, and the same 16-word cookie *except for word 15, which this routine never writes*:

| Cookie word | Derived value | Cookie word | Derived value |
| --- | --- | --- | --- |
| 0 | `0x00000000` | 8 | `0x00000000` |
| 1 | `0x00000010` | 9 | `0x00001300` |
| 2 | `0x01004004` | 10 | `0x00001350` |
| 3 | `0x01004004` | 11 | `0x000013e0` |
| 4 | `0x00000010` | 12 | `0x00000000` |
| 5 | `0x00001001` | 13 | `0x00000000` |
| 6 | `0x0001f01f` | 14 | `0x00000000` |
| 7 | `0x00001000` | 15 | **UNKNOWN: unwritten here** |

The returned size is `0x1420` (XHW reply `+0x58`), clear-page start is `0` (`+0x5c`), and clear-page count is `1` (`+0x60`). These are **CONFIRMED as deterministic CPU calculations in the retained Xpsb binary for the stated dimensions**, not confirmed hardware-valid encodings. The `+0x64` value therefore does **not** block this particular scene-info calculation. It remains unknown on the Dell and can matter for other dimensions. No SGX register read was performed in this investigation. The per-word [format map](../psb-dri-re/frozen-draw-formats.csv) records the same result.

In the candidate kernel, `psb_scene.c:111–130` allocates a zeroed scene, but passes a separate stack `psb_xhw_buf` to `psb_xhw_scene_info`. That function copies all 16 returned cookie words into the scene (`psb_xhw.c:80–112`), while retained `Xpsb_scene_info` explicitly writes only words 0–14. The candidate `psb_xhw_add` (`psb_xhw.c:42–63`) does not initialize the buffer's cookie. Therefore word 15 cannot be assigned a deterministic historical value from these sources. The retained `Xpsb_scene_switch_fire` (`0x4550`) reads word 15 in its `fire_flags & 0x01000000` branch; its normal branch without that flag uses other cookie words. The candidate normal TA task starts with `PSB_FIRE_FLAG_RASTER_DEALLOC` (bit 0), whereas `PSB_FIRE_FLAG_XHW_OOM` is bit 24 (`psb_schedule.c:1252–1256`, `psb_drm.h:377–384`). Thus word 15 is **not required by the initial successful bind/fire branch in this candidate source/binary correlation**. Whether an exact paired stack initialized it elsewhere, or a later OOM path needs it, remains UNKNOWN.

The selected TA path still lacks a proved complete XHW service lifetime and request/reply ordering against DRI submission. Physical SGX power/clock/reset requirements remain separate UNKNOWNs. `Xpsb_scene_switch_fire` can issue MMIO writes and kicks in the historical implementation; no part of this report authorizes repeating them.
