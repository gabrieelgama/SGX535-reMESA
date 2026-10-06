# From the first triangle to a reusable 3D renderer

Current baselines are **KNOWN_GOOD_TRIANGLE_RENDER** and
**KNOWN_GOOD_TRIANGLE_DISPLAY_PUBLICATION**. The [FIRE #3 tutorial](FIRE3-TRIANGLE-REPRODUCTION.md)
and [visible-display tutorial](VISIBLE-TRIANGLE-REPRODUCTION.md) preserve the actual
successful procedures and immutable sources. Later failures do not invalidate them.
No hardware was contacted and no SGX invocation occurred in this offline extension.

## Reconstructed-path capability matrix

These classifications concern this implementation, not features advertised for SGX535.
FIRE #3 is evidence for one successful current operation. Historical encoders can
support construction without establishing execution of a new feature.

| Capability | Classification | Evidence and limit |
| --- | --- | --- |
| TA operation lifecycle | ESTABLISHED | FIRE #3 accepted TA completion, end-render, memory-free and retirement, ledger7 |
| Vertex ingestion | ESTABLISHED | Measured footprint agrees with three supplied post-viewport vertices; arbitrary vertex attributes remain untested |
| Index handling | ESTABLISHED for selected mesh | Six indices0,1,2,1,3,2 and four vertices produced exact256pixel square |
| Multiple foreground primitives | ESTABLISHED for two indexed triangles | Exact square, same-operation ledger7 and attributable readback; see MULTI-TRIANGLE-REPRODUCTION.md |
| Primitive sequencing | PARTIALLY_ESTABLISHED | Bounds/state/foreground/termination sequence works; general mesh sequencing unqualified |
| Vertex transformation | UNKNOWN | Input is already in screen space; no reconstructed general matrix transform demonstrated |
| Perspective | UNKNOWN | Constant reciprocal/position state; no varying-w scene demonstrated |
| Clipping | UNKNOWN | First triangle lies wholly inside32×32; no outside/near-plane case |
| Interpolation | UNKNOWN | Constant fragment result bypasses varying color interpolation |
| Varying transport | UNKNOWN | Historical fields exist; zero compiled attributes and no varying-output shader qualified |
| Depth/Z ingestion | PARTIALLY_ESTABLISHED | Fixed z=.5 travels in known vertex input; no independently measured depth result |
| Depth testing | UNKNOWN | No competing-depth scene or depth readback oracle |
| Fragment execution | ESTABLISHED | Exact constant-program substitution produced the expected magenta footprint |
| Constant fragment output | ESTABLISHED |120 attributable words0xffff00ff |
| Nonconstant fragment output | UNKNOWN | No arithmetic/varying export sequence demonstrated |
| Texture sampling | UNKNOWN | No qualified texture experiment |
| Multiple render operations | UNKNOWN | Historical separate FIRSTLOAD operations are not repeatable-frame evidence |
| Repeated frame submission | UNKNOWN | Current kernel entry permanently consumes one attempt |
| Synchronization between frames | UNKNOWN | No authoritative frame-N+1 rearm provider |
| Buffer reuse | UNKNOWN | Completion plus closed capsule does not prove safe reuse/publication |
| PBE color publication | ESTABLISHED | Fixed32×32 linear target, stride128, full attributed readback; arbitrary formats/sizes untested |
| CPU readback | ESTABLISHED |4096 bytes agree with response/capsule for current operation; TA root publication remains separate |
| Display publication | ESTABLISHED | Preserved FIRE #3 bytes shown through CPU/Xorg; exact restoration and operator confirmation |
| Direct scanout | UNKNOWN | Private SGX target and Xorg scanout are distinct; no shared backing contract |
| Presentation scheduling | PARTIALLY_ESTABLISHED | One bounded15-second server-grab display transaction; no animation scheduling |
| Static 3D object | UNKNOWN | No multi-face perspective/depth scene measured |
| Lighting | UNKNOWN | No controlled varying intensity verified |

Primary evidence: [sealed FIRE #3 manifest](FIRE3-TRIANGLE-REPRODUCTION.json),
[sealed display manifest](VISIBLE-TRIANGLE-REPRODUCTION.json),
[historical indexed record fields](../phase7/psb-dri-re/format-fields.csv),
[draw layout](../phase7/psb-dri-re/frozen-draw-formats.csv), and
[current frame architecture](REUSABLE-RENDER-FRAMES.md). No exhausted TA-root
grammar/publication investigation was repeated or needed for this experiment.

## Preserved completed experiment: two foreground triangles

**TWO_TRIANGLE_FOREGROUND_QUAD** is the smallest discriminating extension.
Keep the observed constant fragment program and every state/program except the
geometry/count and mandatory address adjustments. Draw two equally oriented
triangles forming a square. This tests a fourth vertex, additional indices and
two foreground primitive contributions without introducing a varying shader,
projection, texture, depth buffer or different color-store path.

Historical encoder0x3b890 constructs `0x81400000|count`; independent historical
bounds construction already emits `0x81400006`. The [serializer](../../tools/psb-dri-re/frozen_triangle_image.py)
uses that relationship. This supports the new count construction, not a claim
that arbitrary multi-triangle scenes have already executed.

| Intentional delta | FIRE #3 | New experiment |
| --- | --- | --- |
| Foreground vertices | (8,8),(24,8),(8,24) | Add (24,24),same z=.5,w=1,RGBAwhite |
| Indices,uint16 |0,1,2 |0,1,2,1,3,2 |
| Indexed packet count |0x81400003 |0x81400006 |
| Vertex bytes,stride32 |96 |128 |
| TA stream BO offset |96 |128,still68bytes |
| Relocations |49 |49;seven destinations shift8dwords,one stream pre-add96→128 |
| Fragment words |001f00ff fca7f1f1 |Identical |
| PDS/USE/ISP/PBE/surface/state machine/client/UAPI/observer |Known-good |Identical |

The only semantic rendering variable is the indexed foreground mesh. Moving
the TA stream prevents overlap with vertex4; it does not change TA/raster state
semantics. All BO sizes/domains stay fixed. See the
[full delta/candidate manifest](two-triangle-experimental-candidate-20261006.json)
and [isolated generated tables](artifacts/two-triangle-experiment-20261006/).
Production repository kernel/serializer files are not edited. The isolated
module source comparison found exactly two changed payload include files among94
source/build files. The opaque qualified entry object was reused byte-identically;
its contents were not inspected or reconstructed.

The CPU oracle predicts256magenta pixels in `x=8..23,y=8..23`,768zero. Device
edge rules for this new mesh remain a live premise; this oracle is not GPU proof.
The original120-pixel footprint cannot satisfy the new square expectation.
Attribution and full pixel coordinates must be checked independently of exit status.
Exact square output would establish multiple foreground contributions, **not**
perspective, interpolation, a cube or a 3D object.

## Offline qualification and candidate

[Qualification record](first-3d-offline-qualification-20261006.json) records:

- 32/32 Python construction/frame-policy tests; deterministic exclusive payload generation.
- Native,UBSan,and statici486/QEMU initial and relocated images all match the independent
  Python model across all180224bytes of six BOs and49relocations.
- 1032constant-instruction roundtrips per native/UBSan/i386 variant; invalid view
  rejects before modification, full backing initialization/zero target verified.
- 183source-guard checks and7service/observer differential cases per native/UBSan variant.
- Driver repeated build byte-identical;253versioned imports/CRCs match the qualified
  driver, modinfo identical, no unversioned undefined imports.
- Two image builds byte-identical; finished `/init` hash agrees with its packaged
  hook; unchanged observer and original client/UAPI. This is offline construction
  qualification, **not** FIRSTLOAD/PRE07 or GPU qualification.

Driver Build ID: `8be2777b2a79eaa6651b89d19faf4d68cdcdc460`.
Observer Build ID: `f11d3abb072caa4e1d32836ef92ce201e9c9d126`.
Image SHA-256: `0e45ad4c9ec3eaa891253ea319348286c869644a3ba6c9dba54de39d92bd1e7d`.

**NEXT_3D_EXPERIMENT_READY** means the offline candidate and
[execution card](next-3d-execution-card-20261006.json) are ready for authorization.
Fresh STOCK/staging/manual FIRSTLOAD and complete live PRE07 remain required;
the historical FIRE #3 boot/capsule cannot be substituted. Maximum requested
invocations: **one**, no retry. SGX execution and display publication are currently
unauthorized. The card is deliberately not executable until freshly observed
boot/witness/destination bindings and separate authorization exist.

## Progression toward real 3D

1. If two foreground triangles pass, build a static multi-triangle mesh using
   only demonstrated constant shading. A new candidate needs its own delta/card.
2. Qualify CPU model/view/projection and near-plane clipping, then a bounded
   static cube with a controlled painter ordering if depth testing is still
   unknown. A CPU-transformed perspective cube is honestly described as
   **CPU TRANSFORM + SGX RASTERIZATION**; it does not establish a vertex shader matrix unit.
3. Independently recover/qualify a varying color export before claiming
   interpolation or lighting. Use known normals and directional diffuse
   `I=max(0,n·L)` with independently calculated intensities. Constant-magenta
   success supplies no unqualified machine instructions for this step.
4. Establish the rearm provider in [frame architecture](REUSABLE-RENDER-FRAMES.md),
   then several deterministic transforms with per-frame evidence and fail-stop.
   Only a successful bounded sequence justifies an animation loop.
5. Add varying/shading to the repeatable cube → **FIRST_LIT_ANIMATED_3D**.
6. Custom internal gears requires mesh, transforms, stable repeatability and
   presentation. Actual `glxgears` additionally requires Mesa/OpenGL/GLX
   integration; a custom demo must never be labeled `glxgears`.

Early presentation remains **SGX RENDER + CPU/XORG PRESENTATION**. The preserved
triangle helper checks the exact historical120-pixel source and will reject a
new square. A future new-result validator/card must be qualified explicitly;
never relax or overwrite the historical validator/source. Animation needs short
owner-managed presentation transactions, not a15-second XGrabServer per frame.
Direct/shared-buffer rendering or page flips are not prerequisites for early3D;
their allocation,visibility,ownership and synchronization contracts stay separate.

Current static cube,lighting,animated3D and gears are therefore development
targets, not established milestones. The exact next live question is bounded:
**Do both indexed foreground triangles contribute the predicted square?**

## Addendum: independently preserved visual stages

Desired visual milestone sequence:

`TRIANGLE_ESTABLISHED → INTERPOLATION_OR_SHADING_ESTABLISHED →
MULTI_TRIANGLE_ESTABLISHED → FIRST_3D_OBJECT_ESTABLISHED → FIRST_ANIMATED_3D →
FIRST_LIT_ANIMATED_3D → FIRST_GEARS`.

The already prepared two-triangle probe is retained unchanged. It resolves
foreground mesh sequencing without adding an unknown varying shader. If successful,
record MULTI_TRIANGLE_ESTABLISHED immediately; do not withhold a demonstrated fact
merely to fit the diagram. The next shader experiment then isolates varying/export
before combining shading with a cube. An interpolation-only result is not lighting.
No cube experiment should combine first-ever mesh handling,first-ever varying
transport and first-ever projection into an uninterpretable failure.

The [machine-readable stage plan](3D-MILESTONE-PLAN.json) names the independent
archives,prerequisites and validation obligations. Successful stages receive
new immutable manifests/tutorials. A subsequent prettier image never replaces
the first successful image,configuration,execution record or visual evidence.

### Cube implementation plan

After primitive and controlled shading contracts are established,use eight
mathematical corner positions with face-specific normals; duplicate corners
where differing normals require distinct vertex attributes (24face vertices,
36indices is the conventional six-face/twelve-triangle mesh,not a claim about
newly qualified SGX vertex capacity). The packet/buffer planner must check those
counts/strides and derive all changed relocations anew.

Use explicit right-handed model,view and projection matrices; define the exact
matrix/vector convention in the fixture. Transform to clip coordinates on the
CPU if no vertex-matrix shader is qualified. Clip in homogeneous space before
division,reject nonfinite/near-zero w,then apply the measured viewport convention.
The CPU oracle must test handedness,winding,near/far planes,off-screen bounds,
index connectivity,face ordering and representative projected positions.

A carefully chosen convex cube orientation can initially use deterministic
back-to-front face ordering with explicit backface selection. Label this
**PAINTER-ORDERED DIAGNOSTIC CUBE**,not depth testing. Distinct face appearance must
come from a demonstrated color/shading path. Proper depth behavior is a separate
overlapping-triangle test with deliberately reversed submission order and an
independent nearest-surface oracle. Only recognizable multi-face perspective
output with attributable pixels establishes FIRST_3D_OBJECT_ESTABLISHED.

### Bounded repeatability before animation

After the static cube and an authoritative rearm provider are qualified,prepare
five known orientations,such as0°,15°,30°,45°,60° about a declared axis. Each
frame requires its own immutable geometry/transform identity,current operation
context,accepted ledger,retirement and attributable readback. The transition
requires verified DPM/event/source/mapping readiness; see
[frame reuse requirements](REUSABLE-RENDER-FRAMES.md). Redisplaying one old image
does not establish repeated rendering even if the display is animated by CPU.

The initial frame quota is finite and independently authorized. Stop on the
first ambiguous frame; preserve all preceding successes and the failing frame.
Successful per-frame evidence establishes REPEATABLE_3D_PIPELINE. A bounded moving
cube can establish FIRST_ANIMATED_3D; only then may a separately authorized
continuous demonstration be considered. No infinite loop is added during bring-up.

Presentation should use a short Xorg owner-managed transaction for each completed
frame in one bounded rectangle,not repeat the historical15-second server grab.
Preserve the original region once,retain exact frame hashes,check owner/mode
continuity before each write,and restore/check the original region on exit while
ownership remains valid. Avoid full-screen writes. Refresh/vblank scheduling is
a new presentation contract; animation is not ready merely because XPutImage
worked once. Architecture remains **SGX RENDER + CPU/XORG PRESENTATION**.

### Lighting and gears

First qualify a deterministic directional diffuse calculation with face normals
and a fixed normalized light,for example `L=normalize(1,2,3)` and
`I=max(0,normalize((M^-1)^T n)·L)`. Define rounding,clamping and output encoding.
Compare selected vertices/pixels against a CPU oracle. For a rotating object,
fixed light and transformed normals must predict changing intensities; assigning
unrelated constant colors to faces is not a lighting calculation. If the CPU
calculates intensities and SGX transports/interpolates them,label that division
accurately. GPU arithmetic requires its own qualified shader contract.

Controlled shading establishes SHADING_ESTABLISHED; the attributable rotating
cube plus orientation-dependent predicted illumination establishes
FIRST_LIT_ANIMATED_3D. Preserve the first lit static reference as well.

Only after mesh,transformation,shading,reuse and presentation contracts hold,
prepare a small custom gears scene with deterministic tooth geometry and fixed
angular relationships. FIRST_GEARS requires recognizable animated3D gears
actually rendered by SGX; it is **custom SGX535-reMESA gears**,not `glxgears`.
Actual OpenGL gears through Mesa/Gallium/GLX remains a separate stronger milestone.
No Mesa implementation is required just to prepare these direct experimental stages.

Every major first result preserves exact GPU output,source/binary/program/state
identities,expected CPU reference,readback,execution and presentation records,
restoration,hashes and reproduction instructions. Photos/video supplement machine
evidence; report a referenced but unavailable video honestly. Each next live
experiment needs a fresh bounded execution card and separate authorization.

## Current next experiment after the square

**MULTI_TRIANGLE_ESTABLISHED** is preserved in the
[square reproduction](MULTI-TRIANGLE-REPRODUCTION.md) and
[manifest](MULTI-TRIANGLE-REPRODUCTION.json). One new SGX invocation, no display
publication, authorization consumed. The exact256/768pixel oracle is now observed.
Prior statements describing the square as a future/live premise are historical
offline preparation, not current capability status.

Next: **TWO_COLOR_TWO_DRAW_QUAD**. Same four vertices and target; split index
triples0,1,2 and1,3,2; clone the known primaryPDS template for an independent
green constant program; emit the historical group0x40 primaryPDS rebind before
the second draw. Predeclared oracle120magenta/136green/768zero. This tests
per-draw launch selection, not interpolation or lighting. No new live run or
display publication is authorized. Cube projection math/oracles remain CPU-only.
