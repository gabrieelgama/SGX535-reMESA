# Native Gallium adapter

> **Historical adapter notes.** Later restricted original GLXGears hardware results are documented in the [October 27 physical status](../../docs/phase9/PHYSICAL-GLX-STATUS-20261027.md). Statements below about unsupported execution describe this earlier path/checkpoint; they are not a current project-wide hardware status or general OpenGL conformance claim.

See the [October 14 development update](../../docs/phase9/DEVELOPMENT-UPDATE.md)
for the current original-fragment integration and its explicit laboratory gate.
The older diagnostic interface described below remains a separate path.

The Mesa 25.2.0 adapter now provides real `pipe_screen`/`pipe_context` callbacks
for buffer/render-target resources, mapped transfers, vertex/index state,
framebuffer state, indexed triangle translation, submission and fences.
The submit seam consumes a clean, immutable projected-position draw packet.
No standalone client is launched and no CPU-rendered image is substituted.

The Linux winsys now issues the new native DRM draw request directly and copies
only the kernel's captured SGX color output into the Gallium render resource.
It does not invoke the historical client. The initial kernel provider supports
one asset-matched triangle, using CPU upload/readback staging in userspace and
owned SGX buffers in the kernel. This hardware path is implemented and compiled,
and the historical diagnostic Gallium draw has a retained hardware receipt.
That result does not qualify execution of the current NIR-generated shader pair.

The recording-backend tests remain separate: they exercise provider transitions
while leaving pixels zero. Neither their success nor compiling the real hardware
application establishes hardware rasterization or OpenGL support.

The initial diagnostic pipeline uses already-projected XY/Z/W positions and an
explicit constant ARGB value. It is a temporary driver-level test interface,
not a shader API. NIR compilation, general framebuffer formats, textures,
depth, clipping, blending and OpenGL capabilities are unavailable. Only a
32×32 `B8G8R8A8_UNORM` render target is accepted. The backend must establish
retirement and CPU publication separately and quarantine uncertain submissions.

[mesa-api-pin.json](mesa-api-pin.json) records official interface identities.
No external SGX implementation is used as a reference. With those headers and
the pinned generated format header available:

```sh
python3 mesa/sgx535/tests/qualify.py \
  --mesa /path/to/mesa-25.2.0 \
  --generated /path/to/generated \
  --output /tmp/remesa-gallium-tests
```

This runs focused native and UBSan tests; an optional compiler/sysroot/QEMU
triplet adds i386 execution. Integration tests enter through Gallium callbacks
and reach the clean resource registry/provider using synthetic completion
receipts. No GPU work occurs.

The remaining first-draw bridge is actual clean SGX allocation/MMU, independently
supported command encoding, device dispatch and visibility-qualified completion.
See [the driver architecture](../../docs/architecture.md).

The actual driver-level test is `tests/gallium_first_draw.c`. It enters through
Gallium resource/state/draw callbacks, performs at most one native submission,
preserves the entire input/output ioctl response even on failure, and preserves
the 4096-byte readback before comparing the exact magenta triangle. Its
`--execute-once` mode requires a separately reviewed execution card and explicit
live authorization. `tests/qualify_linux.py` builds this application without
executing it, and runs interposed ioctl transport tests offline.
