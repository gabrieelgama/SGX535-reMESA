# Historical Phase 8 index before the documentation cleanup

This mixed earlier planning with later results. Use the [current index](../phase8/README.md) for status.

# Phase 8 current milestone index

Current continuation: [authoritative handoff](../phase8/CODEX-HANDOFF-20261006.md).

- **KNOWN_GOOD_TRIANGLE_READBACK / TRIANGLE_ESTABLISHED:** [FIRE #3 reproduction tutorial](../phase8/FIRE3-TRIANGLE-REPRODUCTION.md), [full identity manifest](../phase8/FIRE3-TRIANGLE-REPRODUCTION.json), [immutable archival evidence](../phase8/artifacts/FIRE3-TRIANGLE-20261006/).
- **DISPLAY_PUBLICATION_ESTABLISHED / GPU_RENDER_CPU_PUBLICATION_ESTABLISHED:** [visible centered result](../phase8/display-publication-centered-result-20261006.md), [machine-readable milestone](../phase8/display-publication-centered-result-20261006.json), [sealed evidence](../phase8/artifacts/display-centered-one-publication-20261006/). Operator saw the enlarged triangle; saved display contents restored byte-for-byte. No new SGX render.
- Frozen physical-display reproduction: [tutorial](../phase8/VISIBLE-TRIANGLE-REPRODUCTION.md), [full-hash manifest](../phase8/VISIBLE-TRIANGLE-REPRODUCTION.json). Baselines: KNOWN_GOOD_TRIANGLE_RENDER / KNOWN_GOOD_TRIANGLE_DISPLAY_PUBLICATION.
- **DISPLAY_VISIBILITY_NOT_ESTABLISHED:** [single-attempt sealed result](../phase8/display-publication-result-20261006.md). Software pixels and restoration matched; the operator did not see the triangle. Historical [display architecture and implementation](../phase8/display-publication-20261006.md), [offline qualification](../phase8/display-publication-qualification-20261006.json) and [consumed execution card](../phase8/display-publication-execution-card-20261006.json).
- Direct SGX scanout remains **not established**.
- Centered scope: [5× display-only preparation](../phase8/display-publication-centered-20261006.md), [consumed exact card](../phase8/display-publication-centered-execution-card-20261006.json). Offline61/61 PASS; fresh live bindings and one authorized centered publication completed.
- Additional [video publication](../phase8/display-publication-video-result-20261006.md) completed once under separate authorization; operator confirmed visible/recorded/restored, exact pixel and restoration checks PASS.
- Three historical display-only attempts under three separate authorizations; no retry within any authorization. All authorizations are consumed. No further SGX execution or live display mutation is authorized.

Earlier checkpoint/design/history: [2026-10-05 handoff](../phase8/CODEX-HANDOFF-20261005.md). Historical authorization flags in archived cards/scripts do not grant current authority.

## Offline extension toward real3D (2026-10-06)

The [capability matrix and roadmap](../phase8/FIRST-REAL-3D-ROADMAP.md),
[reusable-frame architecture](../phase8/REUSABLE-RENDER-FRAMES.md),
[qualified two-triangle candidate](../phase8/two-triangle-experimental-candidate-20261006.json),
[qualification record](../phase8/first-3d-offline-qualification-20261006.json), and
[next execution card](../phase8/next-3d-execution-card-20261006.json) supersede next-step
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

Follow-on [visual milestone plan](../phase8/3D-MILESTONE-PLAN.json): interpolation/shading,
multiple triangles,cube,bounded repeatability,animation,controlled lighting,lit
rotation,and custom gears. Every first success gets independent artifacts and
reproduction instructions. Actual OpenGL gears is a separate later milestone.

Latest target: **DEMONSTRABLE_3D_ESTABLISHED** (not yet achieved),a recognizable
rotating perspective cube on LVDS. [Continuation plan](../phase8/DEMONSTRABLE-3D-PLAN.md),
[machine-readable claims/next contracts](../phase8/DEMONSTRABLE-3D-PLAN.json),and
[CPU-reference qualification](../phase8/demonstrable-3d-CPU-qualification-20261006.json).
The square card and completed qualification stay unchanged. New CPU cube
fixtures do not establish a GPU cube;per-draw constant selection and frame reuse
remain the next narrowly identified contracts.

## Established two-indexed-triangle square

**MULTI_TRIANGLE_ESTABLISHED**: [reproduction](../phase8/MULTI-TRIANGLE-REPRODUCTION.md),
[full-hash manifest](../phase8/MULTI-TRIANGLE-REPRODUCTION.json). One authorized call on boot
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

One separately authorized [square publication](../phase8/square-display-result-20261006.md)
used the sealed256pixel SGX source through CPU/Xorg.10×nearest-neighbor
presentation produced a centered160×160magenta square; operator saw and recorded
it. Machine readback matched25,600magenta pixels and exact restoration of all
409,600saved bytes. [Full result](../phase8/square-display-result-20261006.json) and
[card](../phase8/square-display-execution-card-20261006.json) preserve the actual procedure.
Display taskSGX invocations0; no directSGXscanout. Authorization consumed;
no further live display or SGX execution authorized. Historical first triangle
and centered first physical triangle records remain unchanged.
