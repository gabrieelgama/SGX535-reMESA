# Mesa 25.2.0 EGL and OpenGL admission

The real client is `tools/remesa/mesa_egl_context_probe.c`. Its default mode
opens no render workload: it validates the retained PCI/DRM descriptor, loads
GBM/EGL, and attempts the actual Mesa context interfaces. Optional
`--triangle-reference` uses real GL entry points only after context creation
and exact renderer validation; it remains unexecuted because admission fails.
Neither a renderer name nor matching pixels establishes GPU attribution.

## Actual initialization failures

| Environment | Exact failing operation | Evidence |
|---|---|---|
| i386/QEMU without SGX DRM | `eglInitialize`: `EGL_NOT_INITIALIZED` (`0x3001`) | Actual instrumented Mesa `_eglDeviceRefreshList` finds zero DRM devices; hardware `dri2_create_screen` is not reached; software retry returns NULL |
| Mini 12, incomplete private runtime | `gbm_create_device` returns NULL | Actual loader cannot find required `dri_gbm.so`; original failed receipt retained |
| Mini 12, complete private runtime | `remesa_dri_create_screen`: `ENOTSUP` (`95`) at `opengl-admission` | Actual constructor records `identity=1 transport=1 screen=1 pci=8086:8108 missing=0x1f`; GBM returns NULL before EGL display initialization |

The live run used STOCK boot `915ed71d-b7cc-4f5b-94bd-040a083da290`,
antiX `5.10.240-antix.1-486-smp`, and the unchanged `gma500_gfx` module.
Its actual i386 production seccomp selftest passed before the client. Only
`DRM_IOCTL_VERSION` was allowed; GPU, GEM allocation and modesetting ioctls
were denied. No clear, draw, memory ioctl or SGX submission occurred. Xorg
remained present and module references returned to the original count of two.
No system libraries, graphics module, display ownership or boot were changed.
A subsequent authenticated read-only inspection captured kernel logs, the
connected LVDS connector and unchanged Xorg/module state. No panic, Oops or
Call Trace was present; existing boot-time ACPI/USB/backlight warnings are
retained in the original log rather than normalized away.

Campaign:
`/home/gama/sgx535-offline/phase9-egl-unblock-20261009T064619Z/`.
Original live observations: `live-loader-result.json` and
`live-loader-complete-result.json`; offline actual EGL:
`egl-gbm-complete/result.json`. These are actual entry points, not mock contexts.

## Implemented loader connection

`prepare_egl_primary.py` applies a pinned public Mesa-source overlay. When this
Mesa build explicitly includes SGX535, it admits a primary-only PCI
`8086:8108` device with DRM major226, exact `gma500` kernel identity and a
retained read/write descriptor. Foreign, read-only, mismatched and unexpected
render-node configurations are rejected. EGLDevice advertises only the
primary-device DRM extension, never a nonexistent render node or scanout.

`build_mesa_egl_overlay.py` compiles the three changed EGL translation units
and links the complete real Mesa EGL target with the exact Gallium frontend.
The normal Mesa overlay also carries the Meson definition. Gallium, EGL and
GBM backend artifacts were linked twice with byte-identical results. The
qualified runtime now includes both `libgbm` and its `dri_gbm.so` backend.

`remesa_dri_create_screen` uses the existing Linux transport and owned-screen
cleanup. Its per-call report distinguishes fd retention, PCI validation,
DRM transport, allocation, and OpenGL admission. The live constructor allocates
then destroys the diagnostic screen before returning ENOTSUP. It does not
return an incomplete screen to Mesa or alter capabilities to force a context.

## Remaining OpenGL contracts

The actual Mesa state tracker assumes baseline compatibility functionality:
its default version calculation starts at GL1.3 and enables inherited
extensions. Current reMESA lacks general vertex/fragment execution and linkage,
texture sampling, qualified pixel-center/edge semantics, enabled blend/depth/
stencil states and native framebuffer preservation. The admission mask is
0x1f for those five groups. Restricted callbacks do not satisfy that baseline.
No OpenGL version or extension is newly advertised.

This increment adds immutable recognition of a single fragment NIR `load_ubo`
from slot0, a constant aligned vec4 offset and one COLOR/DATA0 store. Gallium
owns a retained constant resource or a copied user snapshot, synchronizes its
writer, and specializes only the already supported magenta/green/yellow
constant-color profiles. This is host profile selection, not a new GPU uniform
instruction or general shader compiler. Unsupported operations fail before
submission. Native execution remains restricted to its qualified exact profile.

General USSE/PDS generation, texture task/sampler encodings, COLOR load/store
and larger render-surface state remain UNKNOWN. Existing resolution-independent
transfer storage through 1920x1080, including1280x800, does not establish GPU
rasterization at those sizes. No software renderer or CPU compositing is used
to claim missing hardware behavior.

The Mini12 reports GLIBC2.41. The private Mesa/GBM/DRI dependency chain loaded
on its actual i386 userspace, resolving the prior GLIBC2.38 version-floor
uncertainty for this tested path. No system GLIBC was replaced.
