# First visible SGX535 triangle: actual reproduction procedure

Frozen baselines: **KNOWN_GOOD_TRIANGLE_RENDER** and
**KNOWN_GOOD_TRIANGLE_DISPLAY_PUBLICATION**. Rendering, physical publication and
GPU-render/CPU-publication are established. **DIRECT_SGX_SCANOUT_ESTABLISHED = NO**.
The [full-hash manifest](VISIBLE-TRIANGLE-REPRODUCTION.json) locks the source,
successful card, tools, command vectors and immutable result archive.

## Historical result: distinguish the two placements

The original [32×32 card](display-publication-execution-card-20261006.json) used
`(64,64)`. Its software pixels and restoration matched, but the operator did not
see that attempt; its [negative visibility record](display-publication-result-20261006.md)
remains unchanged. The **first confirmed physical display** used the separately
authorized [centered 5× card](display-publication-centered-execution-card-20261006.json):
160×160 at `(560,320)`,15 seconds. The operator reported
“Triangle visible; original display restored.”

[Successful result](display-publication-centered-result-20261006.md) binds that
observation to exact pixel readback and byte-exact restoration. The later,
separately authorized [video attempt](display-publication-video-result-20261006.md)
reused the same presentation. Video recording was operator-confirmed; no video
file was supplied for independent verification. Neither attempt issued new SGX work.

### Human-observed historical time

The maintainer's subsequent milestone record gives first observed physical
appearance as **2026-10-06 04:11 BRT (UTC−3)**, equivalent to **07:11 UTC** at
minute precision. This is supplementary human-observed history, not an instrumented
wall-clock onset or a replacement for machine receipts. The centered campaign
directory is labeled `20261006T070918Z`; that controller-directory label does not
independently timestamp when pixels first appeared. The preserved machine receipt
records17.947882440988906seconds total controller duration and the configured
15-second hold,with byte-exact publication/restoration. No original timestamp,
receipt,operator record or seal is rewritten to match the later human record.

## Exact SGX-produced source

Use the preserved [FIRE #3 original readback](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/fire3-one-authorized-call/originals/color.original.bin).
Do not regenerate a triangle and label it the GPU result.

SHA-256: `52b2aeb7316820f955d1b9a46cb55a888efb8f74de916acd7ec7ddff6d99ebb5`.
4096 bytes,32×32,128-byte rows, little-endian ARGB8888. Exactly120 words
`0xffff00ff`,904 words zero; foreground `y=8..22,x=8..30-y`, inclusive bounds
`(8,8)..(22,22)`. [FIRE #3 tutorial](FIRE3-TRIANGLE-REPRODUCTION.md) and
[render identity manifest](FIRE3-TRIANGLE-REPRODUCTION.json) bind this source to
accepted TA/end-render/3D-memory-free completion, retirement and current-operation
response/capsule provenance. This display path never invokes the rendering client.

## Actual display architecture

| Property | Successful binding |
| --- | --- |
| Boot | `f1ab6606-0561-445f-a397-28a028516cd7` |
| Kernel/machine | `5.10.240-antix.1-486-smp`,i686,Dell Inspiron1210 |
| Owner | Xorg modesetting,PID1939,startticks4890,VTtty7,display`:0.0` |
| Software publication | glamor initialization failed; DRISWRAST; core XPutImage CPU drawable copy |
| Root/visual | root614,visual33,TrueColor,depth24,RGBmasks`ff0000/ff00/ff`,LSBFirst |
| Visible KMS object | CRTC34,FB77,origin0,0; LVDS-1,1280×800 approximately60.03Hz |
| Surface | width1280,height800,32bpp/depth24,XRGB8888,pitch5120 |
| Separate console framebuffer | `/dev/fb0`,gma500drmfb,pitch8192; not presumed to be Xorg's visible FB77 |
| Source/presentation |32×32 GPU readback → CPU nearest-neighbor5× →160×160 at560,320 |
| Affected range |160 intervals `[y*5120+560*4,y*5120+720*4)` fory320..479;102400bytes |
| XImage | pitch640,RGB24 in32bpp; physical/GTT base addresses not guessed or accessed |

SGX source backing and Xorg scanout backing are separate allocations. Xorg owns
its mapped modesetting frontbuffer and DRM master. The helper uses its existing
core X11 software path, not `/dev/fb0` copying, SGX submission, GL, a page flip,
mode change or master acquisition. See the [source-backed architecture map](display-publication-20261006.md).

Conversion discards source alpha: magenta becomes RGB`0x00ff00ff`; zero becomes
black, not transparency. Nearest-neighbor replication changes presentation size
only. All25600 display pixels match the enlarged source:3000magenta,22600black;
foreground bounds `(600,360)..(674,434)`. There remain120 independently GPU-produced
magenta pixels, not3000.

## HISTORICAL SUCCESSFUL PROCEDURE

Authoritative [command vector](artifacts/display-centered-one-publication-20261006/publication-argv.json),
[deployment controller](artifacts/display-centered-one-publication-20261006/deployment-source.py),
[fresh-check receipt](artifacts/display-centered-one-publication-20261006/precheck-receipt.json)
and [complete archive](artifacts/display-centered-one-publication-20261006/) preserve
what actually ran. Controller deployment was transport/integrity machinery; it
was not an SGX or display-hardware requirement.

1. Verify the original4096-byte source's SHA-256, length and all1024 words.
2. Verify all three qualified tool byte hashes and exact successful card bytes.
   The archive contains `triangle_pixels.py`, `triangle_display.py` and
   `triangle_display_centered.py`; helper import alone performs no device access.
3. Record explicit one-display-only authorization bound to card SHA-256 and boot;
   SGX authorization false. Use new exclusive tool/evidence destinations.
4. Deploy exact tools/source/card/receipt with exclusive creation,fsync and byte
   verification. Preserve transport receipts. Do not overwrite historical files.
5. Passively inspect source/boot/module notes/Xorg PID+starttime/VT/root/visual,
   compositor/window-depth compatibility, CRTC/FB/mode/dimensions/pitch. The actual
   command was `python3 -B /home/gama/sgx535-display-FIRE3-centered-20261006-tools/triangle_display_centered.py --inspect --card /home/gama/sgx535-display-FIRE3-centered-20261006-tools/card.json`.
6. Exact current target must equal the card. Reject an unsupported owner, layout,
   active compositor or incompatible intersecting window. No guard repair.
7. Launch **once**, with the actual command below; preserve the launch intent and
   stdout/stderr/status. A transport ambiguity never authorizes another launch.
8. Before pixel writes, preserve exact source/card/authority and consumed-intent
   record. XGrabServer serializes other X clients. Recheck ownership and save the
  160×160 region using XGetImage; durably preserve102400-byte backup and bindings.
9. Convert/enlarge the original source on CPU; recheck target. XPutImage uses
   ZPixmap,GXcopy,RGBplane mask`0xffffff`,IncludeInferiors, then XSync.
10. Capture published pixels and compare every RGB24 value with expected source
    enlargement. Retain original capture before interpretation. Hold15seconds.
11. Recheck ownership. Restore saved region with the same copy path; capture and
    compare before releasing the server. Depth24 leaves the unused high byte
    undefined in general; in this result **all102400 bytes** also matched.
12. Preserve original evidence and hash/seal it. Fetch originals read-only and
    verify the remote9-file seal before deriving the result. The complete38-file
    attempt seal binds controller inputs, receipts, operator statement and interpretation.

Actual successful launch command, copied from the preserved argv:

```text
python3 -B /home/gama/sgx535-display-FIRE3-centered-20261006-tools/triangle_display_centered.py --publish-once --card /home/gama/sgx535-display-FIRE3-centered-20261006-tools/card.json --authorization /home/gama/sgx535-display-FIRE3-centered-20261006-tools/authorization.json --source /home/gama/sgx535-display-FIRE3-centered-20261006-tools/source.bin --evidence /home/gama/sgx535-display-FIRE3-centered-20261006-evidence
```

This is a **consumed historical command**, not permission to execute it. Its
evidence directory already exists and deliberately prevents reuse. The successful
helper exit was0; total controller duration17.947882440988906seconds, including
the configured15-second display interval. Camera readiness was explicitly obtained
for the later video attempt; no desktop recorder was assumed to run through the
XGrabServer interval.

## CURRENT RECOMMENDED REPRODUCTION PROCEDURE

Preserve this immutable known-good source/tools/card first. Under new explicit
display-only authority, create a new exact card and exclusive destinations. Bind
fresh current boot/Xorg/scanout observations rather than replacing historical
fields silently. Hash/verify the source and tool identities again. Run the same
qualified transaction once, only after all new card bindings pass. Have a physical
observer/camera ready; other X clients pause during the15-second interval.

Do not remove freshness/exclusive-destination checks or rewrite old evidence to
make the historical command usable again. An X server/connection or ownership loss
can prevent restoration; retain backup, report UNKNOWN and STOP rather than writing
into a different display. Source/card mismatch, missing privilege/access, topology
change, preservation failure, pixel mismatch or ambiguous outcome also STOP.
This tutorial grants no new display or SGX authorization.

CPU-only checks:61 synthetic tests, native/UBSan/statici386-QEMU packing oracle,
native/i386 Xlib ABI checks. [Result verifier](../../tools/display/verify_display_publication_result.py)
checks source, original seals, copy, restoration and archived physical-observation
statement without contacting hardware. These checks do not themselves observe a panel.

## Future 3D presentation

Early demos may use **SGX RENDER + CPU/XORG PRESENTATION** once each new frame has
attributable completion/readback. This exact helper is a bounded reversible
diagnostic publication, not a frame scheduler:15-second server grabs and historical
one-shot receipts should not become animation architecture. Direct/shared-buffer
presentation needs separate ownership, synchronization, color-buffer and scanout
qualification. The first visible triangle remains established independently.

## Continued development

The [capability matrix and3D roadmap](FIRST-REAL-3D-ROADMAP.md) branches from
these frozen baselines. The display milestone and its source are never replaced
by a synthetic reference image or a later experimental result. Read-only tutorial
agreement check: `python3 -B tools/display/verify_visible_triangle_reproduction.py`
from the repository root.
