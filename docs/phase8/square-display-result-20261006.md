# Preserved square physically published — 2026-10-06

**SQUARE_DISPLAY_PUBLICATION_ESTABLISHED.** Exactly one display-only attempt
published the preserved SGX square through CPU coreXPutImage and the current
Xorg-owned LVDS scanout. No SGX invocation occurred in this display task.
The maintainer reported: “i see it, insane, i recorded”. A video file has not been
provided/archived. Physical restoration was not explicitly stated by the operator;
machine readback independently proves every saved byte was restored.

- Source4096bytes,SHA-256`6e9af8e8b6576b979aa78816d44bd3b0ffd31b70e169a982e83736d63ab91729`;
  original32×32:256magenta,768zero. The sealed SGX source remains unchanged.
- Exact nearest-neighbor10× image320×320 at`(480,240)`; visible magenta square
 160×160 at`(560,320)..(719,479)`. Alpha is dropped for XRGB24; zero is black.
- Xorg PID1797,startticks4688,VTtty7,root614,visual33;CRTC34,FB77;
 LVDS1280×800,depth24/bpp32,pitch5120. `/dev/fb0` was not used.
- Publication interval15seconds; total controller duration18.94050410797354seconds.
- RGB readback matches25,600magenta/76,800black display pixels exactly.
- Original affected320×320region saved before the write; all409,600bytes restored
 exactly. No padding/other screen rows were included.
- Source/tool/card/authorization/owner bindings and bounded directory are exact;
 other X client requests are briefly serialized by the qualified Xorg path.

[Execution card](square-display-execution-card-20261006.json),
[full result](square-display-result-20261006.json),
[originals seal](artifacts/SQUARE-DISPLAY-20261006/originals/originals.seal.json),
[attempt seal](artifacts/SQUARE-DISPLAY-20261006/attempt-seal.json),
[derived lossless RGB readback](artifacts/SQUARE-DISPLAY-20261006/published-derived.png).

The adapter retains the prior triangle publisher/transaction implementation;
only exact square-source validation and bounded10×coordinates change.
27CPUchecks and independent native/UBSan/i386409600-byte layout models pass.
These establish construction/cleanup, not visibility; actual publication/readback,
restoration and operator observation supply the live evidence.

The archive contains the actual preparation/binding/publication/preservation
controllers, exact tools, source/card/authorization, original screen backup,
published/restored bytes, receipts, hashes and operator report. Reproduction
requires a new authorization and fresh owner/boot/mode bindings. Never reuse this
consumed card or automatically retry. Display and SGX authorizations are separate.

**GPU_RENDER_CPU_PUBLICATION_ESTABLISHED**, **DISPLAY_PUBLICATION_ESTABLISHED**.
**DIRECT_SGX_SCANOUT_ESTABLISHED = NO**. No page flip, shared-buffer direct
scanout, cube, two-color render or repeated-frame run occurred. Both this display
permission and the preceding single square render permission are consumed.
Further display or SGX execution is not authorized. Live work stopped.
