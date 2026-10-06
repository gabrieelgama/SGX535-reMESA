# From a GPU readback to the physical panel

The successful presentation path is **SGX render → preserved readback → CPU/Xorg
publication → LVDS**. It copies pixels through the existing Xorg owner; it is not
direct SGX scanout and does not render a new frame.

The first physically observed triangle was the centered enlargement at `(560,320)`
on 6 October 2026. The maintainer's historical observation is **04:11 BRT (UTC−3)**;
machine capture timestamps are recorded separately. The earlier small `(64,64)`
attempt passed software publication/restoration checks but was not seen. It must
not be described as the first visible success.

## Source and display requirements

Verify the preserved source first:

```sh
python3 tools/reproduction/verify.py all
```

The tested display session used Xorg modesetting, CRTC 34, framebuffer 77 and
LVDS 1280×800, depth 24 / 32 bpp, pitch 5120. Those are historical identities;
recheck the current owner, mode, visual and framebuffer before a new publication.
`/dev/fb0` was a separate gma500drmfb object with pitch 8192. Writing it as though
it were Xorg's visible surface would not reproduce this result.

Source pixels are little-endian ARGB8888. The Xorg publisher converts them to the
visual's RGB masks, discarding source alpha rather than blending. Zero source
pixels become black; magenta becomes RGB `0xff00ff`. Nearest-neighbor enlargement
preserves each source pixel's identity.

| Publication | Source | Affected region | Visible colored footprint |
| --- | --- | --- | --- |
| First observed triangle | 32×32 triangle, 5× | 160×160 at `(560,320)` | Enlarged triangular subset |
| Square | 32×32 square, 10× | 320×320 at `(480,240)` | 160×160 at `(560,320)`–`(719,479)` |

## Actual publication procedure

The [visible-triangle tutorial](../phase8/VISIBLE-TRIANGLE-REPRODUCTION.md) and
[manifest](../phase8/VISIBLE-TRIANGLE-REPRODUCTION.json) bind the original source,
qualified helper, successful centered card, receipts and restoration checks.
The [square execution card](../phase8/square-display-execution-card-20261006.json)
and [square result](../phase8/square-display-result-20261006.json) describe its
separate successful attempt. The historical small triangle card is retained but
is not the successful centered card.

The owner-managed helper verifies the source, display owner and visual, calculates
the rectangle, saves its original pixels, briefly pauses other X clients, uses
`XPutImage`, and reads the region back to check publication. After the 15-second
interval it restores the backup and checks exact bytes before releasing ownership.
It preserves the source, backup, observed images, streams and timing/result records.
The square restored all 409,600 region bytes; the triangle restored all 102,400.

A new attempt needs a fresh card and separate display authorization. The helper
must reject changed display ownership/mode, unsupported visual, invalid bounds,
missing source or incomplete backup. Failure to publish does not justify another
attempt. Restoration and evidence retention remain necessary after partial writes.
Do not paste the old controller command and treat its consumed permission as new.

A phone/camera observation corroborated both successful publications. The square
video was reported as recorded but was not supplied to the repository. Software
readback/restoration records and human physical observation are separate evidence.
