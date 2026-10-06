# How the triangle arrived

The project began on **16 September 2026**. Its first established SGX535 triangle
arrived on **6 October**, twenty days later. Most of that work was understanding
interfaces and ownership rather than writing a large shader or a Mesa driver.

## Phase 1: identify what Linux was missing

The initial 16 September investigation compared Linux gma500 display support with
PowerVR services/DDK sources. It mapped the host protocol and separated the
display GTT from the SGX MMU. The [dated architecture map](../architecture.md)
records that checkout's limitations; it did not establish GPU execution.

## Phase 2: recover the SGX535-specific sources

[Source archaeology](../source-archaeology.md) found `sgx535defs.h` in historical
TI repository revisions, despite its absence from the initially examined branch.
It also found an explicit Poulsbo D0 build selecting SGX535 rev121. These discoveries
made same-core investigation possible without treating the OMAP integration as
Poulsbo. The [recovered-file inventory](../sgx535-missing-files.md) retains provenance.

## Phase 3: follow the Poulsbo integration

The [17 September integration map](../poulsbo-architecture.md) connected Linux's
display-only interface to the historical services, memory and command machinery.
The [historical ABI](../poulsbo-historical-abi.md), [MMU notes](../mmu-bif.md) and
[evidence matrix](../evidence-matrix.csv) record the bridge/CCB, address-space and
platform distinctions. This enabled later controlled bring-up; source configuration
was still not a measurement of the physical board.

## Phases 4–5: turning access into a question that could be answered

Passive probe construction and review made power, ownership, register access and
recovery explicit. These phases did not establish rendering. Missing documentation
and uncertain access semantics were recorded rather than replaced by guessed
writes. [Phase 4 audit](../phase4-audit.md), [final pre-bring-up review](../phase4-7-final-pre-bringup-audit.md)
and [Phase 5 report](../phase5-final-report.md) preserve the reasoning and the
blocked decisions at their time.

## Phase 6: documentation and access boundaries

The work searched Intel and Imagination documentation trails and cross-platform
SGX material, distinguishing evidence for this chip from family comparisons.
It established why some attractive routes still lacked an access or recovery
contract. [Phase 6 report](../phase6-final-report.md), [source inventory](../phase6-1-source-inventory.md)
and [documentation exhaustion](../phase6-2-documentation-exhaustion.md) preserve
both useful leads and negative results. They are not claims that later work remained blocked.

## Phase 7: recovering a controllable rendering path

Historical PSB/Xpsb material exposed command/state relationships, PDS programming,
addressing and scene construction. The [Phase 7 index](../phase7/psb-dri-re/README.md)
leads to the unique ABI, ISA, MMU and source evidence. Those detailed records remain
valuable even when a shorter reproduction guide no longer needs every checkpoint.

## Phase 8: completion, color, then the panel

The first authorized FIRE attempt reached HOLD. The later DHOST completion-consumption
work corrected a lifecycle defect; the [triangle tutorial](../phase8/FIRE3-TRIANGLE-REPRODUCTION.md)
records predecessor evidence and the scope of that conclusion.

FIRE #2 completed TA, end-render, memory-free and retirement, but all 4,096 color
bytes were zero. It retained no TA-output memory. Subsequent analysis did not
recover a complete normal-root grammar or a CPU-publication contract for those
bytes, so the earlier geometry coverage remains unknown.

FIRE #3 tested one change: eight fragment-program bytes replaced by a historical
constant-magenta sequence. One call produced the expected triangle—120 magenta
pixels and 904 zero—with matching lifecycle and output provenance. Its [tutorial](../phase8/FIRE3-TRIANGLE-REPRODUCTION.md)
and [manifest](../phase8/FIRE3-TRIANGLE-REPRODUCTION.json) preserve the actual procedure,
including failed preparation controllers rather than recasting them as GPU rules.

The preserved pixels were then shown through CPU/Xorg on the Mini 12 LVDS panel.
The first observed success was the centered 160×160 enlargement at `(560,320)`;
the maintainer recorded **2026-10-06 04:11 BRT**. The earlier small placement was
not seen. [Visible-triangle reproduction](../phase8/VISIBLE-TRIANGLE-REPRODUCTION.md)
keeps the machine records separate from the human timestamp.

The next render used four vertices and six indices to form two foreground triangles.
It produced the exact 16×16 magenta square: 256 magenta pixels, 768 zero. A separate
CPU/Xorg publication visibly showed the square and restored its destination.
[Square reproduction](../reproduction/SQUARE.md) and [display result](../phase8/square-display-result-20261006.json)
preserve both stages independently.

## What follows

Multiple triangles are established; a perspective object and repeated SGX frames
are not. The [current roadmap](../phase8/FIRST-REAL-3D-ROADMAP.md) progresses through
controlled face/color selection, cube geometry, transforms, frame reuse, animation
and lighting. Mesa, direct scanout and actual OpenGL gears remain separate claims.
