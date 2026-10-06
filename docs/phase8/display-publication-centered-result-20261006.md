# Visible SGX535 triangle — centered CPU publication milestone

**DISPLAY_PUBLICATION_ESTABLISHED.**
**GPU_RENDER_CPU_PUBLICATION_ESTABLISHED.**
**DIRECT_SGX_SCANOUT_ESTABLISHED: NO.**

Exactly one attempt under the separately authorized
[centered card](display-publication-centered-execution-card-20261006.json) published
the already-sealed FIRE #3 image through the existing software Xorg owner. The
maintainer confirmed: **“Triangle visible; original display restored.”**
No SGX rendering invocation was made. The known-good FIRE #3 rendering baseline,
its sealed evidence, module/image identities and original4096-byte source are unchanged.

## What was shown and verified

- Same boot `f1ab6606-0561-445f-a397-28a028516cd7`, module notes, Xorg lifetime,
  VTtty7, root/visual, CRTC34/FB77,1280×800 mode and pitch5120 passed fresh checks.
- Source: original32×32 readback,120 magenta and904zero pixels; SHA-256
  `52b2aeb7316820f955d1b9a46cb55a888efb8f74de916acd7ec7ddff6d99ebb5`.
- Presentation only: CPU nearest-neighbor5× enlargement to160×160 at `(560,320)`.
  Each original pixel becomes25 display pixels; no geometry/shader/PBE change.
- All25600 published RGB24 pixels match the expected source enlargement:
  **3000 magenta `0x00ff00ff`,22600 black**. Inclusive foreground bounds
  `(600,360)..(674,434)`; full coordinates retained in the sealed interpretation.
- Configured display hold15seconds; total SSH/controller duration
  17.947882440988906seconds; helper exit0.
- Original160×160 display region was durably saved before the first write.
  Restored readback matches RGB24 and **all102400 original backup bytes**.

The operator's physical observation and matching software readback jointly support
visible publication through this CPU/Xorg route. This is not direct/shared SGX
scanout. The enlargement does not establish3000 GPU-rendered pixels: the original
GPU result remains120 magenta pixels.

## Preservation and stopping boundary

[Machine-readable result](display-publication-centered-result-20261006.json) and
[attempt archive](artifacts/display-centered-one-publication-20261006/) retain
the exact card/authorization/tools, fresh precheck, launch/transport receipts,
original source/backup/published/restored images, operator observation and interpretation.
Original evidence was preserved and its9-file seal verified before interpretation;
the complete38-file attempt seal also verifies. The previous36-file display attempt
seal, original95-file FIRE #3 seal and135 implementation identities remain unchanged.

| Milestone | State |
| --- | --- |
| TRIANGLE_ESTABLISHED / KNOWN_GOOD_TRIANGLE_READBACK | YES — FIRE #3 |
| DISPLAY_PUBLICATION_ESTABLISHED | YES — centered attempt + operator observation |
| GPU_RENDER_CPU_PUBLICATION_ESTABLISHED | YES — saved GPU pixels, CPU/Xorg copy |
| DIRECT_SGX_SCANOUT_ESTABLISHED | NO — not attempted |

One centered attempt under this authorization; no retry. The earlier small-region
attempt was separately authorized and remains historical evidence that physical
visibility was not established for that attempt. It is not reclassified by this result.

Centered display authorization consumed: **YES**.
Task SGX invocations: **0**.
Further live display publication authorized: **NO**.
Further SGX execution authorized: **NO**.
Original display restored; STOP.
