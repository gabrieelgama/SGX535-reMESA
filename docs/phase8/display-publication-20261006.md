# Preserved GPU triangle → physical display: bounded publication plan

**VISIBLE_TRIANGLE_DISPLAY_READY. No display mutation or new SGX invocation performed.**
The already-established [FIRE #3 rendering baseline](FIRE3-TRIANGLE-REPRODUCTION.md)
is immutable. Its exact archived readback is the sole image source; no new triangle
render is needed. [Execution card](display-publication-execution-card-20261006.json)
and [offline qualification](display-publication-qualification-20261006.json) bind the
one proposed display-only publication. This document grants no live authority.

## Current display architecture and evidence levels

| Stage/object | Established relationship | Level |
| --- | --- | --- |
| SGX color object | Frozen owner allocates a private4096-byte color backing in an excluded SGX VA pool, independently of GEM scanout objects; retired capture produced the sealed image | CONFIRMED source and FIRE3 result |
| Source now used | Preserved4096-byte ARGB8888 file, SHA-256 `52b2aeb7316820f955d1b9a46cb55a888efb8f74de916acd7ec7ddff6d99ebb5`;120 magenta/904zero | CONFIRMED sealed evidence |
| fbdev emulation | gma500 DRM fb helper creates `gma500drmfb` `/dev/fb0`;1280×800,32bpp,pitch8192,virtual1280×800,offset0 | CONFIRMED FBIOGET/sysfs |
| fbdev backing | `drm_fb_helper` → `drm_framebuffer` → `gtt_range.gem` allocated in stolen memory, CPU `vram_addr+offset`; mmap fault maps stolen physical pages noncached | CONFIRMED current driver source; exact current backingoffset UNKNOWN |
| Current visible owner | Xorg PID1939,startticks4890; modesetting driver; fd16 `/dev/dri/card0`; VTtty7 KD_GRAPHICS, IceWM desktop | CONFIRMED passive process/VT/log observations |
| Software path | glamor refused llvmpipe then initialization failed; DRISWRAST provider; ShadowFB enabledNO | CONFIRMED target Xorg log |
| Current scanout | DRM CRTC34,FB77,x=y=0,validmode1280×800; LVDS-1 connected/enabled/primary,60.03Hz | CONFIRMED DRM getters / RandR/sysfs |
| Scanout format/layout | width1280,height800,pitch5120,bpp32,depth24; default visual0x21 TrueColor with RGBmasksff0000/ff00/ff,LSBFirst; gma plane32BPP_NO_ALPHA | CONFIRMED metadata/visual/source; effective XRGB8888 STRONGLY_SUPPORTED |
| Pipe/plane | CRTC34 is second created CRTC, corresponding to pipe1/plane1; Poulsbo regmapB uses DSPB registers | STRONGLY_SUPPORTED source/order; plane DRM object ID not separately queried |
| Scanout allocation | Xorg modesetting front dumbBO → GEM `gtt_range`,64-byte pitch alignment,CPU mapping to owned pages; pin inserts GTT and SGX MMU default mapping at `gatt_start+offset` | CONFIRMED source contracts; current page list and GTT offset UNKNOWN |
| Consumer | `gma_pipe_set_base`:stride=fb.pitch,32BPP_NO_ALPHA; Poulsbo plane base=`gtt.offset+y*pitch+x*cpp`; retained pin while displayed | CONFIRMED source |
| Actual physical/GTT/GPU addresses | Exact FB77 backingphysical pages/GTT offset not exported by passive metadata; fbdev `smem_start=0` is not a usable physical address | UNKNOWN; never guessed or used |
| Render ↔ scanout | Private one-page SGX color object and FB77 full-screen GEM frontbuffer are distinct allocations/ownership lifetimes; archive is a later CPU copy | CONFIRMED source/object separation; no direct SGX scanout claim |

Detailed read-only receipts are archived under
[display evidence](artifacts/display-publication-20261006/). `GETRESOURCES`, `GETCRTC`
and unprivileged nonmaster `GETFB` read metadata. Kernel `drm_mode_getfb` sets handle0
for nonmaster/non-CAP_SYS_ADMIN clients; no GEM handle export or master acquisition
occurred. No MMIO, framebuffer mmap/write, GPU request, module change or mode change
was performed. One earlier read-only GETRESOURCES inspector had an incomplete pointer
array and returned EFAULT; all arrays were corrected offline and complete metadata
then obtained. That controller observation error is not a GPU/display fault.

## Complete software publication path

Preserved SGX pixels → Python CPU conversion → core X11 ZPixmap PutImage →
existing Xorg software `fbPutImage`/`fbPutZImage`/CPU blit → mapped modesetting frontBO →
gma500 GEM/GTT pinned backing → planeB/pipeB → LVDS panel.

Current-driver source: successful build `framebuffer.c:290–448` (fbdev),
`gem.c:100–109,127–173` (dumb/mapping), `gtt.c:235–259` (pin/mappings),
`gma_display.c:50–130` (plane stride/base), `psb_device.c:250–298` (pipe map),
`framebuffer.c:636–637` (CRTC creation). Kernel DRM `drm_gem_mmap_obj` sets write-
combined page protection. CPU-mapped GEM faults retain the object/pin and resolve
actual backingpages, not an invented CPU interpretation of GPU VA.

Target upstream Xorg version21.1.16 source is preserved with URL/hash provenance.
`driver.c:1585–1624` binds the root pixmap to `drmmode_map_front_bo` when no GBM/glamor
and no shadow; `drmmode_display.c:4190` maps the frontBO; `fb/fbimage.c:32–112`
implements ZPixmap into the CPU drawable with GC clipping and raster operation.
This is target-version upstream evidence, not a claim that its archive bytehash
identifies Debian's patched installed binary. Runtime log and successful passive
same-ABI API queries independently corroborate the selected path.

Core API semantics follow the primary [Xlib specification](https://xorg.freedesktop.org/archive/current/doc/libX11/libX11/libX11.html):
XGetImage preserves the visible region, XPutImage uses an explicit GXcopy/RGBplane
mask and IncludeInferiors, and XGrabServer serializes other client requests.
There is no GL/SGX rendering command. An active compositor or intersecting different-
depth window is rejected; the target presently passed those observations.

## Architecture comparison and selection

| Approach | Assessment |
| --- | --- |
| A: direct CPU `/dev/fb0` copy | Mapping exists, but currently owned console fb is not a defensibly identified visible Xorg framebuffer. Reject blind copying/panning. |
| A: existing-owner CPU publication | **Selected.** Xorg exposes the visible root drawable and owns the CPU mapping/scanout lifetime. Core software PutImage reaches its current frontbuffer; preserve/restore only32×32. |
| B: SGX directly into scanout-compatible BO | Driver has linear GEM/GTT/MMU components, but private SGX BO is not an exported scanout GEM object. Shared ownership, mapping/cache completion, PBE size/pitch/format and fence integration require separate qualification and changed render construction. Not implemented. |
| C: compatible buffer + page flip | `gma_crtc_page_flip`/GEM/AddFB infrastructure exists. Requires DRM-master cooperation/new buffer/full mode lifetime and restoration. Xorg already owns master. Larger scope and no advantage for this first diagnostic rectangle. Not implemented. |

Selected label: **GPU_RENDER + CPU_DISPLAY_PUBLICATION** (proposed).
This is not direct/shared SGX scanout. Reusing the established CPU readback removes
any need to resolve TA-root publication. A future B/C path needs its own color-BO
GPU→scanout ordering/visibility ownership contract; this milestone does not supply it.

## Bounded reversible operation

Source stays unchanged. Convert each source word little-endianARGB into the observed
24-bit RGB visual, dropping alpha. `0xffff00ff` becomes XRGB `0x00ff00ff` (bytes
ff00ff00); zero becomes black, not transparency. Do not blend/recolor/scale.
Place full32×32 at `(64,64)` on the existing1280×800 screen. The magenta triangle
is at `y=72..86`, `x=72..158-y`; bounding box `(72,72)..(86,86)`.

XImage pitch128 is distinct from actual scanout pitch5120 and fbdev pitch8192.
Affected FB77 byte intervals, relative to its own base, are
`[y*5120+64*4, y*5120+96*4)` for `y=64..95`:4096 addressedbytes across32rows;
the span including untouched intervening pitch padding is not copied. No physical
base address is used by the implementation; Xorg performs those address calculations.

1. Under separate exact-card display authority, deploy only helper/source/card to
   a new user-owned tool directory; verify their pinned hashes. No SGX candidate,
   renderer state, boot configuration or protected original is modified.
2. Default helper mode is `--inspect`. Recheck same boot/module notes/Xorg PID and
   starttime/VT/root/visual/mode/CRTC/FB/pitch, no compositor, compatible child depths
   and bounds. Source hash and exact pixel pattern must match.
3. Require a new explicit authorization receipt bound to the exact card SHA-256,
   same boot, maximum_publications1, display_authorizedtrue, SGXfalse. Evidence
   directory is card-pinned and exclusive: its existence prevents reuse.
4. Grab the existing X server; other X clients pause during the bounded interval.
   Recheck ownership, save original32×32 with XGetImage, durably preserve backup,
   source, card and authority before first pixel modification.
5. Write one exact RGB copy using the existing server, capture published pixels and
   compare them. Hold15seconds for operator observation, without another trial.
6. Restore saved RGB24 pixels while still owning the same server/root/scanout; read
   back and compare before ungrabbing. XImage high byte is unused at depth24 and
   is not claimed as a meaningful preserved alpha/scanout bit.
7. Hash/seal source/backup/published/restored/card/receipt evidence. A copy failure
   invokes same-owner restoration; ownership loss forbids restoring into a different
   display and retains the backup with restorationUNKNOWN. X connection loss can
   prevent restoration and must be reported; do not retry.

Signals trigger cleanup where ownership/connection remains valid. No software can
promise restoration after a dead X server/kernel. Physical panel visibility remains
an operator observation, independently of XGetImage agreement. The execution card
contains exact source hash, target, mode, format/pitch, coordinates, row intervals,
backup/restore, expected pixels and STOP conditions. It has **display authorityfalse**.

## Offline qualification and remaining limits

**48/48 synthetic tests PASS**: source identity/shape/preservation; normal/edge bounds,
clipping rejection,pitch padding/truncation/overflow; RGB565/RGB24/XRGB/endian/masks;
unavailable destination/ownership loss; mapping and preservation failure; partial copy;
interruption; readback mismatch; backup restoration and failed restoration; evidence-
loss cleanup; exact-card/scope/boot/one-publication authority. No synthetic test is
called GPU or physical-display proof.

Independent full-buffer CPU packing/address oracle: native,UBSan,statici486/QEMU
byte-identical and matches Python's complete synthetic framebuffer. Native Xlib
structure ABI matches public C headers; i386 ABI oracle and actual read-only Mini12
Xlib calls validate the32-bit path. Python is not itself a UBSan instrumented C binary.
No installed package, module/image rebuild or live publication was needed.

Remaining UNKNOWN: exact physical/GTT offsets and plane objectID (not used by the
selected owner-managed route); internal shared SGX/scanout coherency (B/C); actual
physical visibility/restoration under the one future authorized display trial.
No remaining format/pitch/owner ambiguity blocks this narrow display-only experiment.

Milestones now: **TRIANGLE_ESTABLISHED / KNOWN_GOOD_TRIANGLE_READBACK** yes;
**DISPLAY_PUBLICATION_ESTABLISHED**, **GPU_RENDER_CPU_PUBLICATION_ESTABLISHED** and
**DIRECT_SGX_SCANOUT_ESTABLISHED** not yet. Next: explicit authorization for the
one bounded display-only execution card. **No new SGX invocation is required or authorized.**
