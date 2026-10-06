# Demonstrable SGX535 3D: continuation plan

**DEMONSTRABLE_3D_ESTABLISHED = NO.** Target: a recognizable rotating perspective
cube on the physical Mini12 LVDS panel,with machine-attributable SGX frames.
Lighting is desirable but is not a prerequisite for the first public3D result.

This extends the [existing roadmap](FIRST-REAL-3D-ROADMAP.md) without changing its
sealed qualification package or the [pending square execution card](next-3d-execution-card-20261006.json).
The [machine-readable continuation](DEMONSTRABLE-3D-PLAN.json) and
[new CPU-only qualification](demonstrable-3d-CPU-qualification-20261006.json)
record the new work. No live preparation,render or display publication is authorized here.
The historical first triangle/display evidence is not reinterpreted or requalified.

## Fewest unresolved variables

1. Execute the existing **two indexed triangles/square** only under its separate
   authorization boundary. Exact attributable square output establishes
   MULTI_TRIANGLE_ESTABLISHED. Preserve it in a new archive/tutorial before extending it.
2. Prepare **TWO_COLOR_TWO_DRAW_QUAD**: the same square geometry,split into two
   indexed draw packets,with distinct immutable magenta and green constant programs.
   This isolates per-draw fragment-program selection and simple color distinction.
3. Once that contract holds,prepare a static **CPU-transformed perspective cube**,
   reusing the known vertex path,constant fragments and linear color target.
   No texture,interpolation,depth attachment or new shading arithmetic is required.
4. Establish an authoritative post-frame rearm provider,then a finite five-frame
   rotation sequence. Every displayed frame must be a new attributable SGX render.
5. Present the sequence through bounded CPU/Xorg transactions;measure timing.
   Only successful recognizable physical motion plus the machine evidence supports
   DEMONSTRABLE_3D_ESTABLISHED. Controlled continuous rotation follows bounded proof.
6. Add deterministic directional lighting,then lit rotation and custom gears.

Do not add a mandatory interpolation gate to the cube if constant face colors
are sufficient. Do not skip per-draw selection,resource reuse or attribution
when they remain unknown. Collapse stages only when a preserved earlier result
already establishes the relevant contract.

## Supported constant-face route; exact unproven contract

The [preserved constant constructor](../../tools/psb-dri-re/experimental_fragment_constant.h)
derives from historical `0x39ac4 → 0x30ffd → 0x30d15 → 0x30fb7`. Its32-bit immediate
formula is `word0=color&0x1fffff`,
`word1=0xfca40001|(((color>>21)&31)<<4)|((color>>26)<<12)`.
FIRE #3 confirms the magenta instance; it does not independently confirm every color.

[Symbolic face-color plan](../../tools/sgx535_demo/face_constant_plan.py) reuses
the [historical state serializer](../../tools/psb-dri-re/frozen_triangle_image.py).
Changing only `triangle_state[9]`,the primary-PDS reference,produces cache-difference
mask`0x40` and the four-word upload:

```text
0x40, R(secondary_pds), 0x30000, R(primary_pds_next)|0x0c000000
```

The corresponding existing state-copy constructor emits
`a0000000 28a13001 a0200200 fb274000`. These are CPU encoding relationships,
not proof of hardware switching or a general ISA decoder. Keep compiled attribute
state,vertex programs,ISP tests,PBE and surface unchanged. Use separate program
and PDS objects; never overwrite one shared program while the GPU could consume it.

The future probe needs a real planner with complete aligned objects,relocations,
an exact expanded whitelist and byte-differential construction tests. The current
fixed whitelist must not simply be disabled to admit more records. No such
candidate or executable TA stream has been produced in this continuation.

After a verified square establishes its fill behavior,the color probe predicts
120magenta pixels in the original upper triangle,136green in the complementary
lower triangle,and768zero. Both colors in the correct geometry support per-draw
selection. A monochrome square,missing half,wrong color,HOLD or ambiguous result
has a separate interpretation; none permits automatic tuning/retry.

This is the smallest current face-distinction hypothesis. Different fixed colors
do **not** establish interpolation,lighting or shader arithmetic.

## CPU cube reference implemented now

[Reference implementation](../../tools/sgx535_demo/cube_reference.py) defines eight
object-space corners,six outward-oriented faces and twelve triangles. Model
rotation is `Ry(30°+angle) Rx(20°)`; view translates the camera to `(0,0,6)`;
perspective uses45° vertical FOV,aspect1,near1,far12. Conventions are right-handed
column vectors,clip coordinates`-w..w`,and a y-down32×32 viewport.

The independently preserved fixtures contain,for each vertex:
object,world,view,clip,reciprocal-w,and projected screen coordinates. CPU clipping
operates on homogeneous coordinates before division. Tests include all six clip
planes,near/far mapping,nonfinite data,zero-w rejection,perspective shrinkage,
face connectivity,winding and rigid rotation. The first reference frame has
three visible faces and six triangles.

The [CPU reference archive](artifacts/demonstrable-3d-CPU-reference-20261006/)
contains frames0°,15°,30°,45°,60°,complete transforms and independently hashed
reference pixels. All five CPU images differ. Flat magenta/green/yellow face
colors make the orientation recognizable; these images are **CPU predictions**,
not GPU outputs. Never publish them as evidence that SGX rendered the cube.

The screen float32 files are reference packing,not qualified SGX payloads.
For a first no-depth diagnostic candidate,the lowest-risk policy is to retain
the demonstrated GPU position z=.5,w=1 while using CPU-projected x/y;retain true
clip/view depth in metadata for CPU ordering. Do not silently discard that
distinction. A later projected-z/depth experiment requires its own explicit delta.
Keeping w1 does not establish perspective-correct GPU varying interpolation.

This is actual mathematical3D geometry projected by the CPU and intended for
SGX rasterization. It is not a claim that SGX transforms vertices. The future
SGX-produced cube readback must agree with projection/face relationships before
FIRST_3D_OBJECT_ESTABLISHED is recorded.

## Ordering without hardware depth

For this convex cube and external camera,CPU outward-normal tests select visible
faces. Draw them back-to-front by view-space depth;document this as
**PAINTER-ORDERED DIAGNOSTIC CUBE**. This isolates geometry/projection without an
unknown depth attachment. Hardware depth testing remains false.

Proper depth gets a separate overlapping-triangle probe: distinct colors and
depths,deliberately unfavorable draw order,known nearer-surface oracle. Do not
claim depth from a correctly ordered convex cube.

## Repeatability and physical motion

The [frame architecture](REUSABLE-RENDER-FRAMES.md) remains authoritative:
current startup witness/capsule/entry are one-shot. Retirement and clean status
cannot by themselves rearm them. Required new provider: post-frame delayed-work
exclusion,DPM/event readiness,visibility/ownership and continuous isolation.
Do not delete `fixed_attempt_used` to make animation run.

The first live sequence,only after that provider is qualified,has five known
orientations and a finite invocation quota. Preserve per-frame geometry/program
hashes,operation context,accepted events,retirement,complete response/readback and
safe transition. Distinct transforms and output changes must agree. Repeated
XPutImage of one captured SGX image establishes presentation only,not rendering.

Successful bounded repetition establishes REPEATABLE_3D_PIPELINE. Attributable
moving frames establish FIRST_ANIMATED_3D. A separate controlled continuous run
uses an explicit time/frame quota and clean stop;never an unbounded bring-up loop.

Early presentation is **SGX RENDER + CPU/XORG PRESENTATION**. Use one bounded
rectangle,save its original contents once,perform short owner-managed updates,
check mode/owner continuity,and restore/check on exit while ownership is valid.
Do not repeat15-second grabs or overwrite the full screen. Preserve backups if
ownership is lost;do not restore into a different owner's drawable.

Measure separately GPU render latency,presentation latency,and displayed frames
per monotonic elapsed second. Report actual completed/displayed frame counts,
dropped frames and timing,not an estimated FPS. Direct scanout is not required.

## Lighting and public claim discipline

Lighting reference uses known transformed normals and a fixed normalized light:
`I=max(0,n_world·normalize(1,2,3))`. Reference intensities are already calculated,
but are not applied to the diagnostic CPU face colors and are not a rendered
lighting milestone. Rotation-only normals use the model rotation;nonuniform
scaling would require the inverse-transpose contract.

If CPU lighting drives later SGX face constants,call it **CPU DIRECTIONAL LIGHTING
+ SGX FLAT-FACE RASTERIZATION**. Verify predicted orientation-dependent output
against independent intensities. That is not SGX lighting arithmetic. If a
qualified varying/arithmetic shader becomes available,verify it separately.
Only controlled calculation/input-to-output evidence supports SHADING_ESTABLISHED
or FIRST_LIT_3D;unrelated face colors do not. Successful repeated lit geometry
then supports FIRST_LIT_ANIMATED_3D.

| Claim | Current state |
| --- | --- |
| First triangle and physical CPU/Xorg presentation | ESTABLISHED,unchanged |
| CPU cube transforms/projection/clipping/reference packing | OFFLINE_QUALIFIED_CPU_ONLY |
| SGX rasterized CPU-transformed3D cube | NOT_ESTABLISHED |
| SGX vertex transformation | UNKNOWN |
| SGX programmable perspective work | UNKNOWN |
| SGX depth testing | NOT_ESTABLISHED |
| SGX constant magenta fragment output | ESTABLISHED |
| Per-draw face-color selection | HISTORICALLY_SUPPORTED,NOT_ESTABLISHED |
| Controlled lighting/shading calculation in GPU | UNKNOWN |
| New SGX frame sequence/reuse | NOT_ESTABLISHED |
| CPU/Xorg presented an SGX animated cube | NOT_ESTABLISHED |
| Direct/shared SGX scanout | NOT_ESTABLISHED |

## Milestone archives and public reproduction

After each successful live stage,preserve a new immutable archive,tutorial and
manifest;record source/binary/module/image/scene/program identities,expected and
actual pixels,lifecycle and provenance,presentation timing/restoration,hashes and
available photo/video. Update capability state only from that evidence. Failed
successors do not invalidate earlier milestones.

Keep3D bring-up in **Phase8**,because Phase9 is currently Mesa integration. On
the first actual public3D success,create `docs/phase8/FIRST-3D-REPRODUCTION.md`
and `.json` from authoritative successful records. Do not fill them now with
hypothetical success. Preserve the first cube,first animated cube,first lit
object and first gears independently. A physical video should show the Mini12,
the recognizable cube,and motion;machine records must establish that its frames
were genuinely rendered by SGX. Unavailable media must be marked unavailable.

Custom gears follows the cube,not precedes it. Label it **SGX535-reMESA custom
gears demo**. Actual `glxgears` requires the real OpenGL/GLX application running
through the reconstructed graphics stack;it is a stronger separate milestone.

Future public-summary facts supplied by the maintainer:project start2026-09-16;
first established triangle2026-10-06;elapsed20days;first physical observation
2026-10-06 04:11 BRT(UTC−3),the centered160×160 copy at560,320.
The earlier64,64 attempt remains unobserved. Before a3D post,fill the exact
established capability,presentation architecture,measured timing,limitations and
reproduction links from successful records. No world-first claim is supported.

**Current stop boundary:** unchanged square card,maxone invocation,fresh PRE07
and separate authorization required. All later proposed experiments also require
their own bounded cards. No new SGX candidate,hardware contact or live attempt
was made in this continuation.
