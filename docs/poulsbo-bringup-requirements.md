# Requirements for first controlled bring-up

Phase 3 — static analysis, 2026-09-17. `CONFIRMED` means the source declares/implements; `INFERRED` is interpretation; `UNKNOWN` is gap. P3 IDs refer to [matrix](evidence-matrix.csv), with repository, commit, file, and lines. The snapshots preserve the original numbering; see [provenance](poulsbo-data/sources.json). No procedure below was executed on the GPU.

## Decision

**We do NOT have enough information to authorize at this stage an active bring-up of the SGX535, with ownership takeover, reset, MMU programming, or bootstrap.** This is an **INFERRED** assessment of the gaps below, not a claim of project infeasibility.

We have enough material to specify a passive inventory and prepare a future limited identification test. Inventory does not equate to a core bring-up; the lack of firmware does not block inventory nor, by itself, an ID read. The criteria must be proportional to the experiment.

| Future step | Exact blockers | What solves them |
|---|---|---|
| Inventory without new accesses | placa/ambiente and logs not yet provided | already existing records of PCI, driver, kernel/BIOS, exported resources, and boot |
| New SGX ID reading | B1 review/target platform; B2 ownership and access power; B3 reading security and proven recovery | identify board; define cooperation with driver; base allowlist and prepare console/reboot |
| Reset/MMU controlled | B1–B3 plus B4 sequence clock/reset/errata; B5 context index; B6 memory/DMA/coherence and display protection | applicable sources/trace to review and testable plan with limits |
| Bootstrap microkernel | previous ones plus B7 scripts/PDS/payload/ABI and license; B8 timeouts/ack/fault/recovery | corresponding verifiable package and complete startup/shutdown protocol |
| Rendering | previous ones more streams/ISA/layouts/estado/sync/isolamento | new engineering phase, out of scope |

B1 corresponds to U01; B2 to U03/U04; B3 to U02/U16; B4 to U03/U05; B5 to U06/U09; B6 to U07/U08/U20; B7 to U10–U14; B8 to U15/U16. See [20 unknowns](unknowns.md). The P3-026 divergence blocks writing new contexts, not collecting an existing log. Do not require a full ISA to read an ID, nor declare a safe reset just because it exists in gma500.

## Menor experimento proposto — atualizado pela Fase 4

**Step zero, passive:** it is now implemented in `tools/sgx535-probe` as Test Vector Zero. It collects PCI IDs/revision/subsystem, text resources published by the kernel, IRQ, driver, DRM node, kernel, architecture, and runtime status without opening BAR, PCI config, or DRM. The guarantee of 'does not modify state' is limited to the probe: `gma500` already actively initializes hardware during bind.

**Next step remains BLOCKED:** `CORE_ID` and `CORE_REVISION` have confirmed offsets and historical usage, but Phase 4 did not find a read-side-effects contract, power/clock, or sufficient locking. They do not form an approved whitelist. If these gaps are closed, the instrumentation should live in gma500 and use a compiled, specific operation; never generic access to MMIO.

The record must contain ID PCI/stepping, BAR base/len, exact offset, raw value, separate interpretation, known power state, timestamps, and erro/timeout. No offset scans, reset writing, fault testing, CCB, firmware, PDS, or stream3D. Do not insert unlimited retries. After collection, the temporary resource must be removable; in case of failure, stop the sequence and recover using the previously prepared mechanism. Reboot is a recovery plan, not a demonstrated guarantee.

This MMIO stage **is not technically approved as ready**. No MMIO patch or sequence has been written.

## Upcoming sources and highest-value artifact

1. **Legitimate source package UM + microkernel/PDS + corresponding initializer for `pc_i686_poulsbo_d0_linux`, SGX535 rev121, IMG DDK1.14 build3699939**, with explicit license, generated options, filled scripts, hashes, and ABI headers. It is the highest-value external artifact to complete the bootstrap that the local KM only consumes. It has not been demonstrated that this package is publicly available.
2. Exact implementations of `psb_powermgmt.h` / `sys_pvr_drm_export.h` used by the DRM_EXT branch of this DDK, with the host driver version; a homonymous header from another family is not enough.
3. Userspace/Xorg that serves XHW from PSB5.0.0.0045, especially handlers scene-bind/fire, TA-memory, OOM, and DPM reset. It serves to reconstruct historical boundaries; it is not an ABI to adopt.
4. Manual/errata Intel SCH of the target revision that describes access requirements, clock/power/reset/IRQ and the relationship with BIF; EMGD feature matrix does not replace this manual.
5. In the fixed EMGD mirror: build/config generated, bridge dispatch, common/sysconfig.c, plb init/power and sgxreset.c; correlate internal version PVR1.5.15.3226 with an authenticable Intel package. No binary should be treated as firmware compatible just by name.

It is not necessary to resolve streams3D/texturas to prepare inventory. However, to execute any payload, source, license, revision, layout, and stop mechanism need to be checked beforehand.
