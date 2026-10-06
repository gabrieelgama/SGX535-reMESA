# Reproducing the experiments

There are two useful starting points: verify the preserved result on any machine,
or reproduce a new render on the tested Mini 12. These are different operations.

## Inspect the result without hardware

From the repository root, with Python 3 installed:

```sh
python3 tools/reproduction/verify.py all
```

This checks the preserved triangle and square against their hashes and exact pixel
footprints, and checks the archived artifact inventories. It never opens a device,
contacts the target, builds a module or launches a render. Missing archives are an
error, not permission to substitute a nearby build. At this cleanup, substantial
Phase 8 archives exist in the research working tree but are not yet tracked in
its recorded Git commit. No public download location has been verified. A clone
needs the exact referenced bundle before these checks can pass; publication and
artifact-distribution decisions remain open.

See [triangle](TRIANGLE.md), [square](SQUARE.md), and [display publication](DISPLAY.md).
The full historical procedures remain linked from each guide.

## What a new hardware reproduction requires

The tested configuration is a Dell Inspiron 1210 / Mini 12 with Poulsbo
`8086:8108`, SGX535 `CORE_ID=0x01130000`, `CORE_REVISION=0x00010201` (rev121),
and 32-bit antiX `5.10.240-antix.1-486-smp`. Modules are ELF32/i386 and depend on
that kernel's symbol versions. Keep normal STOCK boot and local GRUB/power recovery
available. Loading into a running arbitrary gma500 session is not the tested path.

The successful source tree was dirty relative to commit
`f8565115977bda8a529401a7ac5cba822d1a0d36`. Source snapshots, build commands,
compiler/sysroot identities and exact modules/images are recorded with the results.
A checkout of that commit alone is **not** a complete reproducible build recipe.
The build also reused a qualified opaque entry object. Its availability and rights
must be established independently; this guide does not propose reconstructing it.
Use the preserved successful artifacts for exact historical comparison.

A fresh render still needs the first-load module/observer pair, compatible kernel,
exclusive ownership, valid startup/source lifecycle, completion handling and an
unused operation capsule. Fresh boot and destination bindings must replace the old
ones. The current public tooling does not provide a reviewed, portable, unattended
live reproduction wrapper. The historical controllers show what actually ran;
they contain consumed permissions and machine-specific paths and must not simply
be executed again. The short verifier above is intentionally offline only.

## Which controls mean what?

| Layer | Role |
| --- | --- |
| Hardware/software correctness | Address mappings, state/program encoding, exclusive resources, event handling and retirement |
| Reproduction integrity | Exact image/module/client identities, compatible kernel, same-operation output, preserved original response and pixels |
| Research diagnostics | Observer capsule, source witness, detailed receipts, offline scene/oracle checks |
| Historical campaign policy | One-shot authorization cards, controller deadlines and named FIRE checkpoints |

`FIRSTLOAD` is the experimental module's first-owner boot environment. `PRE07`
is the project's pre-execution preparation check. A `capsule` is the observer's
operation record; the `source guard` tracks startup/lifecycle and competing work.
`FIRE #3` names the historical successful triangle call, not an API or a GPU mode.
Old "Gate B" records describe earlier bring-up decisions at their date.

One-shot policy is not a claim that the physical GPU can only render once. This
implementation has not yet established safe frame-to-frame reuse, so simply
removing the restriction would not yield a qualified reusable renderer.

Keep original response, capsule, readback and streams before interpreting them.
Hash checks alone do not authenticate their live producer. Identity mismatches,
interference, missing originals, partial completion or ambiguous possible issuance
require stopping and retaining the available evidence rather than retrying.
