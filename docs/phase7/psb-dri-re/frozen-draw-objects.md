# Frozen triangle: objects, relocations and bootstrap

This supplements [the frozen draw](frozen-draw-closure.md). Binary addresses refer to the retained DRI ELF with SHA-256 `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`. P7G-011–P7G-013 distinguish binary facts from candidate-source behavior. None authorizes hardware access.

## Object graph

The selected input does not need a fixed GPU address. It needs handles, validated offsets and reproducible relocation algorithms. The remaining uncertainty is which complete resource contents and flags feed those algorithms.

| object | producer / owner | constrained size and content | remaining selected-path input |
|---|---|---|---|
| vertex storage | `0x0003f87a`, `0x0003f505`, `0x0003f610` | Software branch capacity `0x10000` dwords; each chosen vertex eight dwords; three consume 96 bytes. Buffer descriptor retains BO, byte offset/extent, starting vertex and generation. | Allocation success and lifetime must be supplied by compatible BO manager; the selected layout is conditional on fragment inputs `2`. |
| indices | `0x00027fa0`, `0x00027e08` | Six emitted bytes for `0,1,2`, adjusted by starting vertex; allocation pool reserves at least `0x2000` indices, not a six-byte physical BO. | Complete scene pool and exhaustion/reset state. |
| vertex USE | `0x00040355` → `0x00030ffd` | 16 derived bytes for width eight; retained output descriptor and USE relocation. | Instruction semantics/revision applicability, not literal producer bytes. |
| vertex PDS | `0x00040355` → `0x00038762` | Reserves `0xc0`, commits `0x40` bytes in one-part case; data size field is 12 dwords. | Six producer-unwritten data dwords and four literal instruction semantics; no assumption that holes are consumed or reserved. |
| fragment main/secondary programs | `0x00033490`, `0x0003503d`, `0x000370d8` | Result object `0x5e8`, link object `0x88`; sizes depend on generated program. | Selected instruction counts/bytes, registers, link metadata and constants. |
| scene pixel/background/target state | `0x00027060`, `0x000283d8`, `0x00028559`, `0x00039c5c` | Referenced before finalization even without depth or textures. | Complete color-target layout and generated scene programs. |
| TA index stream | `0x0003b890`, `0x0002a39a` | Five-word index record plus state and terminal `0xc0000000`. | Required preceding state records; the terminal is four committed bytes. |
| TA register list | `0x0002b258` | Seven register/value pairs, `0x38` committed bytes; pair `0x238` refers to TA index-stream BO. | Dynamic address relocation and complete scene backing. |
| raster register list | `0x0002ab70` | 52 committed words; depth-null branch removes five depth references. | Background/program/target references remain required. |
| validation/relocation storage | `0x000443d1`, `0x00037854`, `0x00037b51` | Underlying BO references deduplicated; 136-byte wire validation nodes, 40-byte relocations. | Full object inventory determines final list order/count and access constraints. |
| fence | `0x00037b51`, `0x000442bd`, `0x00044097`, `0x00044107` | User fence copied into a 76-byte mutex/refcount wrapper; waits/unreference delegated to libdrm. | Exact linked library/build and complete selected ownership/wait schedule. |

Do not confuse the TA **index stream** with the command's TA **register list**. The `+0x24/+0x28/+0x2c` command fields describe the latter. Its `0x238` pair provides the former's relocated reference.

Context creation `0x00048592` calls the pool constructor `0x0004297b`. On its software branch, recovered pool arguments include the following 64-bit flag halves; these are binary values, not a claim that each pool contributes an object to this triangle:

| context slot | flags low / high | chunk bytes | chunk count | alignment argument |
|---|---|---:|---:|---:|
| `C+0x800` | `0x80000001 / 0` | `0x1000` | `0x18` | `0` |
| `C+0x804` | `0x20000001 / 0` | `0x20000` | `0x1e` | `0` |
| `C+0x808` | `0x20000001 / 0x00080000` | `0x1000` | `0x10` | `8` |
| `C+0x80c` | `0x01000084 / 0` | `0x40000` | `0x10` | `0` |
| `C+0x810` | `0x10000001 / 0x00010000` | `0x40000` | `0x10` | `0` |
| `C+0x814` | `0x01000082 / 0x00040000` | `0x400` | `0x10` | `0` |

The constructor passes count × chunk size, alignment, null user pointer, flags and hint `4` to imported `drmBOCreate`. `0x00043d8c` supplies default flags `0x07000003` only when both supplied halves are zero. This is narrower than treating all selected BO placement values as UNKNOWN, but it is not a complete selected-object allocation specification.

## Relocation and validation algorithms

`0x000443d1` records underlying BO identity separately from suballocation identity, merges flags/masks and rejects conflicts. `0x00037b51` zeroes each 136-byte wire node, links 64-bit next pointers with high half zero, writes operation zero, mask at `+0x10`, flags at `+0x18`, handle at `+0x20`, and hint at `+0x24`. For presumed mode `1`, hint is `0x10` and presumed offset goes at `+0x38` with high half zero. These offsets are relative to the wire node, not its containing list node.

`0x00037854` produces offset relocations: operation, destination word index, source validation index, mask, packed shifts, BO/suballocation pre-add, background, destination validation index. Its final two words are not initialized on this operation. Candidate `psb_sgx.c:631–787` does not need them for offset operation zero. Unwritten fields must not be treated as required zero bytes without considering the operation's consumer.

`0x0003a82c` generates three entries for a USE reference: operation `5` selecting a base register with mask `0xf`, then operation `4` with left/right shifts `4/15`, mask `0xf0`, then operation `4` with shifts `8/4`, mask `0x7ff00`. The candidate kernel's `psb_sgx.c:710–787` calls `psb_grab_use_base` and selects either register number or base-relative offset. It computes `((value >> right_shift) << left_shift)` and merges the mask. **USE_OFFSET preserves the prior destination result**, using its write cache or current destination; replacing it with each entry's zero background would erase earlier fields.

Candidate `psb_regman.c:42–118` chooses/reuses a fence-protected base register covering address and size with the same data master. It returns register sequence and `address - selected_base`. It may program a USE base register; this is historical kernel behavior only. `psb_sgx.c:719–728` accepts only its vertex/pixel data-master values. These source algorithms constrain dynamic address assignment without inventing physical addresses. Their correspondence to this ELF is **INFERRED release-family compatibility**, not exact-build proof (P7G-011).

For the selected no-feedback normal-TA branch, `0x00037b51` zeroes the 144-byte command, then supplies validation pointer `+0x00`, scene pointer `+0x10`, fence pointer `+0x18`, TA flags `+0x20 = 3`, TA register-list handle/offset/word count `+0x24…+0x2c`, optional OOM list `+0x30…+0x38`, raster register-list `+0x3c…+0x44`, relocation handle/offset/count `+0x48…+0x50`, fence flags `+0x58` from caller and engine `+0x5c = 3`. Null feedback excludes the feedback fields. It retries command index zero on `-11` and checks validation handled/error replies before transferring BO reply fields. This does not establish what an actual GPU will do.

## Fence-size check

The recovered source distinguishes **40-byte userspace `drmFence`** (`libdrm/xf86mm.h:97–106`) from **48-byte ioctl `drm_fence_arg`** (`shared-core/drm.h:642–652`). These must not be compared as if they were one structure. The user record contains handle/class/type/flags/signaled/sequence and four padding words; the ioctl record additionally places error at `+0x14` and sequence at `+0x18`.

The binary submit wrapper tests returned fence error at submit-object `+0x40`, then copies the first five reply words into a local record before `0x000442bd` copies ten words into wrapper `+0x24`. The remaining five local words are not established by that construction. This does not prove a 40-versus-48 ABI mismatch. Candidate `drmFenceWait` (`xf86drm.c:2546–2578`) consumes handle/type/flags/signaled, zeroes a separate ioctl argument, submits `DRM_IOCTL_FENCE_WAIT` (number `0xca`), updates class/type/signaled, and returns the ioctl error. Candidate unreference (`:2413–2425`) uses only the handle and request `0xc7`. No selected wait requires the unestablished tail in these source paths. This **narrows** the fence uncertainty; exact linked-library equivalence and lifecycle are still not proved (P7G-012).

## First-submit dependency boundary

Candidate kernel `psb_xhw.c:42–63` rejects queueing when `xhw_submit_ok` is false; initialization at `:416–460` sets that state. Its scheduler calls `psb_xhw_scene_bind_fire` for TA (`psb_schedule.c:199`), submits the TA register list (`:313–315`) and calls `psb_xhw_fire_raster` (`:328`). Therefore XHW readiness is **REQUIRED-CONFIRMED for that candidate kernel protocol**. Cross-component application to the retained ELFs remains a family correlation (P7G-013). This replaces the earlier question of whether the candidate TA scheduler needs XHW at all; it does not close the XHW service implementation.

| prerequisite | selected-path classification | remaining limit |
|---|---|---|
| Compatible DRM fd, BO API, mappings and `PsbDRIRec` | REQUIRED-CONFIRMED within historical paths | exact built pairing still UNKNOWN |
| Candidate kernel XHW service initialized | REQUIRED-CONFIRMED within candidate source | retained Xpsb request/reply service needs full selected-path correlation |
| Each step of retained `XpsbInit` | REQUIRED-CONFIRMED for that successful initializer, REQUIRED-INFERRED for the cross-component stack | initializer order is not a hardware power/reset contract |
| BO manager, color surface, programs, scene and synchronization resources | REQUIRED-CONFIRMED | complete selected contents and lifecycle remain open |
| Uploaded 440-byte static BO as a dependency of this DRI triangle | UNKNOWN WHETHER REQUIRED | no demonstrated selected-consumer edge; not called firmware |
| SGX microkernel or additional proprietary payload | UNKNOWN WHETHER REQUIRED | no inferred payload identity or invented firmware prerequisite |
| Video/MSVDX, texturing, hardware vertex compiler, queries | NOT-REQUIRED for the frozen branch | must keep those branches excluded |
| Correct real SGX state, revision and failure/recovery behavior | BLOCKING-UNKNOWN for execution | separate hardware-safety investigation; no static path authorizes it |

No further package search was warranted by the fence-size difference: the already recovered headers explain it. The `5.0.1.0046` correlation remains **RELEASE-FAMILY MATCH**. The DDX loader-name mismatch and older bundled libdrm header remain in [version pairing](../xpsb-re/version-pairing.md). That report also records the XHW direction difference and the candidate DRM core's request-number dispatch, which resolves the narrow direction concern for the candidate source without proving an exact binary pairing.
