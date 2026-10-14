# Preserved README summary — October 6, 2026

Historical snapshot of the alternative README text retained during conflict-marker
cleanup on October 14. This records the status at that time; see
[the current development update](../phase9/DEVELOPMENT-UPDATE.md).

Bringing experimental 3D rendering back to Intel Poulsbo / GMA 500, one recovered
PowerVR SGX535 contract at a time. Tested on a Dell Inspiron Mini 12.

The GPU is from 2008. The triangle has finally arrived. Please do not sue it.

## What works today

The reconstructed path has produced a **32×32 magenta triangle**, then a
**square made from two indexed triangles**. Both renders completed on SGX535
rev121 with matching operation, completion and readback records. The preserved
images were also shown on the Mini 12's physical LVDS panel through CPU/Xorg
publication, with the affected screen region restored afterward.

| Capability | Status |
| --- | --- |
| Triangle rasterization and color readback | Established: 120 magenta pixels, 904 zero |
| Multiple indexed foreground triangles | Established: 256 magenta pixels, 768 zero |
| Physical display publication | Established through CPU/Xorg |
| Perspective cube, depth, interpolation and lighting | Not established in this reconstructed path |
| Repeated rendering and animation | Not established |
| Direct SGX scanout | Not established |
| Mesa / OpenGL driver | Not complete |

These are controlled experiments, **not a general-purpose graphics driver**.
The square is a useful primitive-assembly result, not yet a 3D object.

## Why this exists

Poulsbo combines Intel display hardware with an Imagination PowerVR SGX535 GPU.
Linux's gma500 display support does not supply the missing SGX 3D stack. Historical
proprietary drivers left behind enough interfaces, code and clues to investigate,
but a working display is a very different thing from a working renderer.

This project reconstructs the missing path using source archaeology, CPU-side
models and small hardware experiments. Evidence takes precedence over a plausible
register name—or a very confident AI answer. AI assists the work; it does not
turn an inference into a hardware result.

## Start here

- [Project and pipeline overview](../PROJECT-OVERVIEW.md)
- [First triangle reproduction](../reproduction/TRIANGLE.md)
- [Two-triangle square reproduction](../reproduction/SQUARE.md)
- [Publishing a preserved render to the display](../reproduction/DISPLAY.md)
- [Documentation index](../README.md) and [project history](../history/PROJECT-HISTORY.md)
- [Current 3D roadmap](../phase8/FIRST-REAL-3D-ROADMAP.md)

The tested platform is the Dell Inspiron 1210 (Mini 12), Poulsbo PCI `8086:8108`,
SGX535 rev121, running 32-bit antiX kernel `5.10.240-antix.1-486-smp`.
The guides identify exact artifacts and explain the current limits of rebuilding
from source. Do not load the experimental module into an arbitrary kernel or run
historical controllers against their old boot bindings.

## Next: something unmistakably 3D

The progression is square → controlled face/color selection → perspective cube
→ a bounded sequence of new renders → rotating cube → controlled lighting →
gears. Early demos may use **SGX render + CPU/Xorg presentation**. Direct scanout
and Mesa integration are separate work; neither needs to block the first cube.
A custom gears demo will not be called `glxgears`.

## Repository layout

- `kernel/`: experimental frozen gma500/SGX integration.
- `tools/`: decoders, construction tests, rendering models and presentation tools.
- `docs/reproduction/`: readable entry points to the successful experiments.
- `docs/phase7/`, `docs/phase8/`: detailed research, build records and experiments.
- `docs/hardware-evidence/`, `docs/phase8/artifacts/`: original captures and provenance.
- `docs/history/`: a route through earlier research without treating old checkpoints as current instructions.
- `references/`: local third-party checkouts, excluded from Git; source revisions are recorded in the research notes.

Raw evidence is deliberately left untouched. Some captures contain machine and
network identifiers; see the [privacy and publication audit](../PRIVACY-AUDIT-20261006.md)
before publishing a new evidence bundle. Large image archives may need release
assets rather than normal Git storage.

## Credits and contact

Thanks to Simon Fenney for the EMGD lead, and to the maintainers and authors of
the historical PSB, Xpsb, Linux gma500 and PowerVR-related source material cited
throughout the research. Their work is credited at the source; inclusion of a
reference does not change its licence.

Public project contact: **operatingsystemsdepression@gmail.com**.
[Support the project](https://github.com/sponsors/gabrieelgama) if you enjoy seeing
old hardware do something it was supposed to do all along.
