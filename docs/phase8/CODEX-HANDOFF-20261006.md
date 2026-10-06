> Public documentation: [index](../README.md), [triangle](../reproduction/TRIANGLE.md),
> [square](../reproduction/SQUARE.md), [display](../reproduction/DISPLAY.md),
> and [history](../history/PROJECT-HISTORY.md). This handoff retains detailed dated
> checkpoints; use the public guides for the shortest explanation of current results.

# Authoritative Phase 8 continuation — 2026-10-06

**KNOWN_GOOD_TRIANGLE_READBACK. Triangle ESTABLISHED in FIRE #3.**

**Current visible milestone: DISPLAY_PUBLICATION_ESTABLISHED and
GPU_RENDER_CPU_PUBLICATION_ESTABLISHED.** The separately authorized centered
publication visibly showed the saved triangle; operator confirmation and exact
software readback agree. Original display restored byte-for-byte. See
[sealed centered result](display-publication-centered-result-20261006.md) and
[machine-readable result](display-publication-centered-result-20261006.json).
One centered attempt, no retry, task SGX invocations0; its authorization is consumed.
No further display or SGX operation is authorized. Direct SGX scanout is not established.

Permanent physical-display reproduction: [actual successful procedure](VISIBLE-TRIANGLE-REPRODUCTION.md)
and [full-hash companion manifest](VISIBLE-TRIANGLE-REPRODUCTION.json).
Frozen baselines: **KNOWN_GOOD_TRIANGLE_RENDER** and
**KNOWN_GOOD_TRIANGLE_DISPLAY_PUBLICATION**. Experiments must use isolated copies,
declare every delta and retain these immutable original artifacts.

Subsequently, one separately authorized [video publication](display-publication-video-result-20261006.md)
reused the same centered display-only presentation. Camera readiness preceded launch;
the maintainer confirmed visible/recorded/restored. Expected pixels and all102400
restored bytes matched; remote9-file/local39-file seals verify. No new SGX invocation,
rendering change or retry. That additional authorization is also consumed.

The first established SGX535 triangle is now permanently recorded in the
[reproduction tutorial](FIRE3-TRIANGLE-REPRODUCTION.md) and
[full-hash manifest](FIRE3-TRIANGLE-REPRODUCTION.json). The repository archive
contains exact successful module/client/UAPI/image bytes, source snapshots,
canonical scene/address plans, actual transfer/FIRSTLOAD/controller/launch scripts,
receipts, all18 original result files and the95/81-file seals. Original offline
archives and all68 FIRE #2 files remain immutable. HEAD alone is insufficient:
the campaign used dirty source inputs; the manifest records that provenance limit.

Known-good driver Build ID: `85ec06b428c99fac7f9127919b7a488d204f4a77`.
Observer Build ID: `f11d3abb072caa4e1d32836ef92ce201e9c9d126`.
Image SHA-256: `3d9eb6baa3d821b4ff98e60ea83f1a94022b5fb2881038d2ad91b95cdfb7448e`.
Successful/current observed boot: `f1ab6606-0561-445f-a397-28a028516cd7`.
Kernel `5.10.240-antix.1-486-smp`, Dell Inspiron1210, SGX535rev121.
Only rendering delta: fragment words `00000000 f8040140` → `001f00ff fca7f1f1`.

FIRE #3 exactly1 launch/ioctl; exit/ioctl/errno0; phase9 RETIRED, ledger0x7;
TA completion/end-render/3D-memory-free/retirement confirmed. Full4268 response,
4152 capsule and4096 readback agree with current-operation provenance.120 pixels
`0xffff00ff`,904zero; `y=8..22,x=8..30-y`; bbox `(8,8)..(22,22)`; full-image
reference mismatch0. **CONSTANT_FRAGMENT_HYPOTHESIS_SUPPORTED.** FIRE #2 coverage
remains retrospectively unknown; do not reopen the established FIRE #3 triangle.

FIRE #3 permission is consumed. Capsule/source are CLOSED. No new render, retry,
reboot/reload/rearm was performed or authorized by the display task.
`sgx_execution_authorized=false`.

## Historical first small-region display attempt

**DISPLAY_VISIBILITY_NOT_ESTABLISHED** after the one authorized display-only
attempt. See [sealed result](display-publication-result-20261006.md). Exact software
publication of 120 magenta pixels and restoration were confirmed; the maintainer
reported “Triangle not seen; display normal.” Physical publication is not claimed.
The display authorization is consumed; no further publication or SGX is authorized.

The prior qualified plan remains a historical record:
See [map/implementation](display-publication-20261006.md),
[qualification](display-publication-qualification-20261006.json) and
[exact execution card](display-publication-execution-card-20261006.json).

Read-only live investigation identified Xorg modesetting software publication,
VTtty7, active CRTC34/FB77, LVDS1280×800,depth24/bpp32,pitch5120. `/dev/fb0`
gma500drmfb has pitch8192 and is not used as a presumed visible destination.
Selected route: preserved SGX pixels → CPU core XPutImage through current Xorg →
existing mapped scanout. It does not issue SGX or acquire DRM master.

One proposed32×32 patch at `(64,64)`;120magenta foreground pixels are at
`y=72..86,x=72..158-y`,bbox `(72,72)..(86,86)`. Source alpha is dropped into
RGB24/XRGB; zero becomes black. Save region before any write; serialize other X
clients, publish once, hold15seconds, restore/check before releasing ownership.
The card binds current boot/module notes/Xorg lifetime/root/visual/CRTC/FB/mode and
exact source/tool hashes. Ownership/evidence loss fails closed and prevents retry.

48 synthetic tests PASS; independent native/UBSan/statici486-QEMU layout oracle
byte-identical and agrees with Python; native/i386 Xlib ABI checks PASS. These are
CPU construction/cleanup tests, not physical-display proof. Live inspection mode
passed without any display write. No driver/observer/image/client/UAPI/render-state
change. Existing unrelated dirty work/index preserved; no staging or commit.

Milestones:

| Milestone | State |
| --- | --- |
| TRIANGLE_ESTABLISHED / KNOWN_GOOD_TRIANGLE_READBACK | YES |
| DISPLAY_PUBLICATION_ESTABLISHED | YES — subsequent centered attempt |
| GPU_RENDER_CPU_PUBLICATION_ESTABLISHED | YES — subsequent centered attempt |
| DIRECT_SGX_SCANOUT_ESTABLISHED | NO |

The small-region result alone did not establish physical visibility. Its evidence
remains unchanged. The subsequent centered result above establishes the visible
milestone without reclassifying the historical observation. No SGX render or
repeated publication is authorized. Historical receipts are consumed records only.

## Subsequent centered presentation preparation

The maintainer selected a new display attempt with its scope to be specified.
A [centered 5× scope](display-publication-centered-20261006.md) and
[new card](display-publication-centered-execution-card-20261006.json) are prepared:
same preserved image,160×160 CPU enlargement at `(560,320)`,15-second hold,
new exclusive destinations, backup/restore, no retry.61/61 synthetic tests and
native/UBSan/statici386-QEMU full-buffer oracle PASS. That preparation involved no
live attempt. The maintainer subsequently approved the exact scope; fresh bindings
passed and the one centered attempt completed as recorded above. Known-good
rendering and the previous attempt evidence stay unchanged.

## Offline extension toward real3D (2026-10-06)

The [capability matrix and roadmap](FIRST-REAL-3D-ROADMAP.md),
[reusable-frame architecture](REUSABLE-RENDER-FRAMES.md),
[qualified two-triangle candidate](two-triangle-experimental-candidate-20261006.json),
[qualification record](first-3d-offline-qualification-20261006.json), and
[next execution card](next-3d-execution-card-20261006.json) supersede next-step
planning, not the frozen first-triangle/display evidence.

**NEXT_3D_EXPERIMENT_READY**: offline candidate only. Driver
`8be2777b2a79eaa6651b89d19faf4d68cdcdc460`,observer unchanged
`f11d3abb072caa4e1d32836ef92ce201e9c9d126`,image
`0e45ad4c9ec3eaa891253ea319348286c869644a3ba6c9dba54de39d92bd1e7d`.
No staging or new hardware contact occurred. Current live state has not been
reobserved; the historical consumed FIRE #3 boot is not a new execution binding.
Maximum requested new invocation count1, no retry. Fresh STOCK/FIRSTLOAD/PRE07
and explicit authorization remain required. No display publication follows
implicitly. The next predicted image is a square, not an established3D object.

KNOWN_GOOD_TRIANGLE_RENDER and KNOWN_GOOD_TRIANGLE_DISPLAY_PUBLICATION remain
independently reproducible. Existing implementation and sealed artifacts are
unchanged; experimental payload lives in an isolated module/archive. Frame reuse,
perspective/varying/depth/lighting and continuous presentation remain unqualified.
Task SGX invocations0; hardware interactions0; no new live authorization.

Maintainer addendum is captured in the [visual stage plan](3D-MILESTONE-PLAN.json)
and roadmap: preserve first cube,repeatable frames,animated cube,controlled lighting,
lit animated cube,and custom gears independently. Static3D precedes animation;
initial repeatability is a bounded five-orientation design,not authorization.
The visible-triangle tutorial/manifest now records human-observed first appearance
`2026-10-06 04:11 BRT` / `07:11 UTC`,minute precision,distinct from machine timing.
No successful original receipt or seal was changed.

## Demonstrable3D continuation (offline only)

Current development target: **DEMONSTRABLE_3D_ESTABLISHED**,not yet achieved.
See [continuation plan](DEMONSTRABLE-3D-PLAN.md),
[claim/experiment manifest](DEMONSTRABLE-3D-PLAN.json),and
[new CPU-reference qualification](demonstrable-3d-CPU-qualification-20261006.json).
These extend planning without changing the sealed square package/card or its
completed qualification. The previous roadmap remains preserved; interpolation
is optional for the first cube if controlled per-face constants suffice.

Implemented CPU model/view/projection,homogeneous clipping,face ordering and
five deterministic cube orientations.20new math/reference tests and7symbolic
constant-selection tests PASS;native/UBSan/i386-QEMU agree on40float32 vertex
records. These are CPU predictions,not SGX cube outputs. No cube candidate/module
or image was produced. The exact missing static-cube contract is per-draw constant
fragment selection;the next proposal after a verified square isolates that on
unchanged square geometry. Animation still needs an authoritative reuse provider.

Keep new cube tests in `tools/sgx535_demo/cube_cpu/tests/` so the frozen square
suite retains its32-test scope;no square qualification was rerun. Every older
seal and protected candidate/card/source identity is unchanged. No new SGX or
hardware action occurred. The only currently ready live experiment remains the
existing square card,maxone invocation,with fresh PRE07 and separate authorization.

## Latest square live preparation — preserved STOP

See [square preparation STOP](two-triangle-live-preparation-stop-20261006.md) and
[machine record](two-triangle-live-preparation-stop-20261006.json). Exact square
candidate boot `83ee4ff8-a7f1-4468-b348-d10627f38d17` passed FIRSTLOAD/PRE07
and immediate passive guards. Offline launch-card structural validation then
stopped before client dispatch. New SGX invocations0; authorization unconsumed.
Seven offline adapter checks pass; no live retry. A fresh continuity/continuation
is required across this STOP. Established FIRE3 and visible-display baselines
remain unchanged; multiple triangles are not yet established.

## Established two-indexed-triangle square

**MULTI_TRIANGLE_ESTABLISHED**: [reproduction](MULTI-TRIANGLE-REPRODUCTION.md),
[full-hash manifest](MULTI-TRIANGLE-REPRODUCTION.json). One authorized call on boot
`83ee4ff8-a7f1-4468-b348-d10627f38d17`:exit/ioctl/errno0,phase9,ledger7,
TA/end-render/3D-memory-free/retirement and same-operation attribution confirmed.
Exactly256magenta/768zero; full square x8..23,y8..23.18originals sealed before
interpretation;139-file independent campaign archive verified. No display
publication. Authorization consumed; further SGX execution not authorized.

The immutable first triangle and centered LVDS publication remain established.
Next offline probe: two-color/two-draw primaryPDS rebinding before a
CPU-transformed perspective cube. Interpolation, depth, repeated frames and 3D
object milestones remain unestablished. Earlier square STOPs are historical
preserved records; the separately authorized continuation succeeded once.

## Subsequent square display-only milestone

One separately authorized [square publication](square-display-result-20261006.md)
used the sealed256pixel SGX source through CPU/Xorg.10×nearest-neighbor
presentation produced a centered160×160magenta square; operator saw and recorded
it. Machine readback matched25,600magenta pixels and exact restoration of all
409,600saved bytes. [Full result](square-display-result-20261006.json) and
[card](square-display-execution-card-20261006.json) preserve the actual procedure.
Display taskSGX invocations0; no directSGXscanout. Authorization consumed;
no further live display or SGX execution authorized. Historical first triangle
and centered first physical triangle records remain unchanged.
