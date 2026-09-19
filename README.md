# SGX535-reMESA

Reverse-engineering and documentation project for the **PowerVR SGX535**, with an initial focus on the **Intel Poulsbo / GMA 500** implementation.

The long-term goal is to investigate whether the surviving public information about the SGX535 can be reconstructed into enough technical documentation to eventually enable a modern open-source Linux 3D driver, potentially through Mesa.

> We don't have the SGX535 programming manual.
>
> So we're reconstructing one.

## Why?

Intel Poulsbo systems, including Atom Z5xx devices, contain a PowerVR SGX535 GPU.

Linux still supports the display side of Poulsbo through the `gma500` DRM/KMS driver, but there is no modern open-source 3D driver for the SGX535.

The hardware is old, but many of these machines still work.

Their GPU should not have to remain a black box forever.

## What we found

An archaeological analysis of historical public SGX DDK sources and the Linux `gma500` driver uncovered significantly more SGX535/Poulsbo information than initially expected.

Among the findings:

- historical `sgx535defs.h`
- an explicit `pc_i686_poulsbo_do_linux` DDK target
- configuration for **SGX535 revision 121**
- historical `services4/system/poulsbo` integration
- SGX535 register definitions
- MMU/BIF information
- USE/USSE and PDS-related definitions
- host ↔ SGX communication structures
- historical command submission infrastructure
- correlations between the historical SGX DDK and the modern Linux `gma500` driver

The initial archaeology examined hundreds of commits across historical kernel-mode and user-mode SGX repositories.

Some Poulsbo-specific components were present historically and later removed from newer source trees.

## Language notice

The initial archaeology and technical documentation were written and generated in
Brazilian Portuguese (pt-BR).

An English version is planned. Source code identifiers, register names,
paths, addresses and evidence references are preserved in their original form.

Contributions improving or translating the documentation are welcome.

## Documentation

The reconstructed documentation lives in [`docs/`](docs/).

Important starting points:

- [`architecture.md`](docs/architecture.md) — reconstructed SGX architecture
- [`registers.md`](docs/registers.md) — known register information
- [`mmu-bif.md`](docs/mmu-bif.md) — MMU/BIF investigation
- [`command-submission.md`](docs/command-submission.md) — command submission
- [`usse.md`](docs/usse.md) — USE/USSE/PDS findings
- [`firmware.md`](docs/firmware.md) — firmware/microkernel investigation
- [`gma500-current-state.md`](docs/gma500-current-state.md) — what modern Linux already implements
- [`poulsbo-evidence.md`](docs/poulsbo-evidence.md) — direct historical Poulsbo evidence
- [`sgx535-missing-files.md`](docs/sgx535-missing-files.md) — recovered/missing historical components
- [`unknowns.md`](docs/unknowns.md) — things we still do not know

## Evidence policy

Reverse engineering old hardware makes it very easy to turn assumptions into fake documentation.

For that reason, findings are classified as:

**CONFIRMED** — directly supported by available source code or documentation.

**INFERRED** — strongly suggested by available evidence, but not directly documented.

**UNKNOWN** — insufficient evidence. No value or behavior should be invented.

Where possible, technical claims include their exact source, revision, file and line range.

## Current status

This is **not yet a functional Mesa driver**.

The current phase is focused on reconstructing enough reliable information about the SGX535 and its Poulsbo integration to determine what would be required for safe hardware bring-up.

Major remaining questions include:

- SGX535 USE/USSE instruction encoding
- PDS execution details
- microkernel implementation
- command stream formats
- shader compilation
- texture/sampler descriptors
- synchronization and cache behavior
- PBE/output pipeline details
- differences between SGX535 integrations and later SGX cores

A particularly important target is locating additional legally/publicly available material corresponding to the historical **SGX535 rev121 Poulsbo DDK**.

## Target hardware

Initial target:

- Intel Poulsbo / US15W
- Intel GMA 500
- PowerVR SGX535
- SGX535 rev121 where applicable

A future implementation must not assume that SGX implementations used by TI/OMAP are identical to Intel Poulsbo.

## Long-term roadmap

1. Recover and catalog public historical SGX535 material.
2. Reconstruct a source-backed SGX535 technical reference.
3. Understand the Poulsbo-specific integration.
4. Determine the minimum safe kernel/userspace interface required for 3D.
5. Perform minimal hardware bring-up experiments.
6. Execute the first known userspace workload.
7. Render the first triangle.
8. Investigate Mesa integration.

No hardware programming should be based on undocumented guesses.

## Why "reMESA"?

Because after years of the SGX535 being effectively unusable for modern open-source Linux 3D acceleration, the ridiculous long-term objective is simple:

**SGX535 + Mesa.**

## Disclaimer

This is an independent preservation and reverse-engineering research project.

It is not affiliated with or endorsed by Imagination Technologies, Intel, Texas Instruments, Mesa, or the Linux kernel project.

Third-party source code is not automatically part of this repository and remains subject to its respective licenses.

The purpose of this project is hardware preservation, documentation, interoperability, and open-source driver research.

# Of course...
Entirely made using PRoot-distro with anland-termux (wayland compositor for Android) (yes, really, bro)
