# A display driver is not a 3D driver

Intel Poulsbo (marketed as GMA 500) contains a PowerVR SGX535 rendering core.
The tested Mini 12 reports SGX535 rev121. The Linux gma500 path already handles
display modes and framebuffer storage; the historical SGX userspace/kernel stack
provided additional machinery for 3D. This project is reconstructing a small part
of that machinery, not replacing every display component.

The strongest results are simple and concrete: the first successful triangle
produced 120 magenta pixels in a 32×32 buffer, and the next indexed draw produced
a 16×16 square with 256 magenta pixels. Both had attributable completion and
retirement records. Their pixels were later copied through Xorg to the physical
panel. [Triangle](reproduction/TRIANGLE.md) and [square](reproduction/SQUARE.md)
guides link the actual binaries, inputs and original results.

## The experimental rendering path

```text
fixed client + prepared state, programs and indexed vertices
                         |
                    PDS / TA work
                         |
                 ISP / raster processing
                         |
                  fragment USE program
                         |
                  PBE → color buffer
                         |
                completion + readback
                         |
           CPU conversion / Xorg XPutImage
                         |
                  existing LVDS scanout
```

This diagram is a conceptual guide, not a specification of every hardware stage.
PDS supplies launch/data sequencing; TA processes geometry into the tiled scene;
ISP/raster processing admits fragments; USE programs supply programmable work;
PBE handles the color-output path. The recovered fixed state is sufficient for
the two successful scenes. It does not yet constitute a complete general encoder.

The first zero-output render completed normally. Replacing eight fragment-program
bytes with a historically supported constant-magenta sequence made the next
render produce the expected triangle. That strongly supports the fragment-color
hypothesis, but does not retrospectively prove the previous operation's geometry
survived TA. Completion reports work finishing; it is not a pixel-coverage counter.

## Memory is several different things

CPU virtual pointers, pinned backing pages, SGX virtual addresses and display GTT
addresses are different namespaces. A numeric GPU address is not a CPU pointer.
The scene/DPM/parameter allocations are distinct from the color surface. The
normal TA and raster paths share a scene-relative root address, but its complete
record grammar and a CPU-publication contract for TA-produced root bytes remain
unresolved. Successful color readback does not settle those separate questions.

The display experiment used another object: Xorg's visible framebuffer. In the
recorded 1280×800 session it had depth 24, 32 bits per pixel and pitch 5120.
The fbdev framebuffer had pitch 8192 and was not treated as the visible Xorg
surface. The publisher used Xorg's own image path, saved the affected region,
verified the published RGB values, then restored and checked every saved byte.
This establishes CPU/Xorg presentation, not shared-buffer SGX scanout.

## What the experiment machinery does

The fixed client makes one operation. Its response, observer record ("capsule")
and readback are matched to one boot, owner and operation. Startup/source guards
reject interference and an unsuitable lifecycle. Completion acceptance tracks TA,
end-render and 3D-memory-free before retirement. These controls helped make the
results interpretable; their historical controller bugs are not GPU requirements.

A future multi-frame renderer needs a demonstrated resource/event reuse contract.
Removing a one-shot restriction would not establish it. Vertex transformation,
interpolation, depth, textures, repeated frames and direct presentation are still
separate capabilities to prove in this implementation.

For the older DDK comparison see [the dated architecture investigation](architecture.md).
For the current direction see [the 3D roadmap](phase8/FIRST-REAL-3D-ROADMAP.md).
