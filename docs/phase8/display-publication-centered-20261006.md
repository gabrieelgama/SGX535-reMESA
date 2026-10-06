# Prepared centered display-only scope

The [previous attempt](display-publication-result-20261006.md) matched software
readback and restored the original pixels, but the operator did not see its small
triangle. This does not establish a display-path defect or disprove publication.

The maintainer requested authorization of a new display attempt with its scope
specified. The concrete proposed scope is now the
[centered execution card](display-publication-centered-execution-card-20261006.json).
Exact scope approval and fresh destination bindings remain prerequisites. No new
attempt, deployment, Mini 12 contact or SGX work occurred during this preparation.

## Single proposed attempt

- Use the same sealed FIRE #3 source, SHA-256
  `52b2aeb7316820f955d1b9a46cb55a888efb8f74de916acd7ec7ddff6d99ebb5`.
- CPU-only nearest-neighbor enlargement by 5; the original 4096 source bytes remain
  untouched. No new GPU render, geometry, shader, PBE or module change.
- Copy the enlarged 160×160 image at `(560,320)`, centered on the 1280×800 screen.
  Expected 3000 magenta display pixels and 22600 black pixels. Magenta inclusive
  bounds `(600,360)..(674,434)`; these are replicas of the original 120 GPU pixels,
  not 3000 independently GPU-rendered pixels.
- Existing software Xorg owner; same depth24/XRGB32 format and scanout pitch5120.
  XImage pitch640. Touch exactly160 row intervals /102400 bytes; no pitch padding.
- Preserve the original160×160 region before publication. Serialize other X
  clients, show once for15 seconds, restore and compare before releasing ownership.
- Use new exclusive tool/evidence destinations. The previous attempt is immutable.
  No retry. Preserve originals before interpreting. Physical visibility requires
  operator observation; drawable readback alone does not establish that milestone.

The new `triangle_display_centered.py` retains the unchanged original helper's
transaction, authority, source validation, evidence sealing and failure handling.
It adapts rectangle checks, XImage dimensions and CPU presentation only. All three
tool hashes are validated before connecting to X. It rejects missing/wrong card,
unexpected topology, other visual depths, compositor, layout or owner changes.
Default inspection reports the actual640-byte image pitch and160×160 extent.

## Offline qualification

[Qualification](display-publication-centered-qualification-20261006.json):
**61/61 PASS** (48 unchanged baseline tests +13 centered-extension tests).
Exact replication, source preservation, full bounds, wrong pitch/dimensions,
partial copy, evidence failure, ownership loss and restoration covered.
Independent full synthetic framebuffer C oracle is byte-identical under native,
UBSan and statici386/QEMU, and matches the Python result. The initial i386 tool
invocation used the wrong host shared-library directory; correcting the environment
completed qualification without changing source. This was an offline tool error.

These are CPU construction/policy checks, not physical display qualification.
Original renderer, driver, observer, client, UAPI and image identities are unchanged.
Original helpers and the prior sealed attempt remain unchanged. No new SGX candidate.

New card SHA-256:
`0cac9c3b4bc602c880ff21163fa4ddf9f4dbc7d01b4aeca2412ca03595973735`.
No display mutation follows until this exact centered scope is approved and its
fresh bindings pass. No SGX invocation is required or authorized.
