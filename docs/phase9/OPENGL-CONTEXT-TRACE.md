# Mesa 25.2.0 context execution

The real i386 EGL client is `tools/remesa/mesa_egl_context_probe.c`. It loads
the isolated Mesa EGL library, calls the actual platform-display, initialization,
API/config/context and pbuffer interfaces, and accepts only the exact hardware
renderer name. It issues no clear or draw. The offline runner denies host
ioctls before QEMU starts; it does not mock EGL or create a simulated context.

| Layer | Observed result | Meaning |
|---|---|---|
| Load real Mesa libEGL | PASS | Actual library and entry points loaded |
| `eglGetPlatformDisplay` | PASS | Display handle obtained |
| `eglInitialize` | `EGL_NOT_INITIALIZED`, `0x3001` | No qualified SGX DRM device in the offline sandbox |
| EGL software retry | Attempted, unsuccessful | No software context accepted |
| `eglTerminate` after failed initialization | PASS, exactly once | Loader cleanup executed |
| `eglCreateContext`, `glClear`, `glDrawArrays` | Not reached | No OpenGL rendering claim |

The client also implements the primary-node GBM route required when surfaceless
render-node discovery cannot serve the display device. It independently owns
the descriptor, checks DRM character-device and PCI `8086:8108` identity before
GBM initialization, and unwinds EGL, GBM and fd ownership. Invalid descriptors
never reach GBM. The genuine-device route remains physically untested.

Separately, `remesa_dri.c:create_screen` still rejects state-tracker admission
with `ENOTSUP` after diagnostic screen validation. This is established by source
and interposed descriptor tests, not by the offline EGL initialization failure.
Mesa's `st_api_create_context` creates the pipe context and then initializes
the state tracker. Its compatibility-version calculation assumes baseline GL
functionality; a tiny shader recognizer and zero texture sampling cannot
truthfully satisfy that baseline. No version override or admission bypass was
introduced.

This increment adds actual sampler creation/binding/deletion and empty-view
unbind contracts, with persistent rejection of every nonempty texture binding.
Legacy position NIR now recognizes `VERT_ATTRIB_POS` as well as `GENERIC0`, with
matching metadata required. FLOAT2/3 inputs expand missing components to z=0,
w=1 before checked host vertex preparation and unchanged SGX packet generation.
These are integrated driver contracts, not general GPU shader execution.

The remaining hardware-dependent contracts include independently justified
texture-task/instruction encodings, general shader/uniform/varying execution,
standard raster center/edge behavior and framebuffer load/store. CPU clear and
transfer implementations do not prove these GPU behaviors. The existing shader,
texture and framebuffer admission checks remain active; no GL version is exposed.
