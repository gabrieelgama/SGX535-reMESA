# 20 priority unknowns — Phase 7.0–7.5 assessment

Updated 2026-09-23 after the static Phase 7.0–7.5 review. No MMIO, PCI configuration write, reset, firmware, or workload was executed. IDs P3/P4/P5/P6/P7 refer to the
[matrix](evidence-matrix.csv).

| # | Unknown | Current evidence / exact lack | Blocks |
|---|---|---|---|
| U01 | What are the target identity and physical revisions? | TVZ-001 records PCI revision `0x06` and subsystem `1028:02b1`; graphics/SCH stepping and physical SGX core revision remain UNKNOWN | MMIO and later |
| U02 | `CORE_ID`/`CORE_REVISION` are safe to read in real stepping? | confirmed historical use, read-side-effect contract absent | first MMIO |
| U03 | Which PM state guarantees valid SGX clocks? | runtime PM PCI does not equal internal clock (P4-004/P4-012) | any MMIO |
| U04 | Which registers are readable when SGX is gated/off? | no applicable source defines this | any MMIO |
| U05 | Which lock serializes generic SGX reading? | Target 5.10.240 uses distinct PM, IRQ, MMU and 2D locks; no universal SGX read protocol established (P7-002–P7-005) | Concurrent MMIO |
| U06 | How to avoid race condition with IRQ and KMS? | Target IRQ handler releases `irqmask_lock` before its SGX reads; modeset, fbdev and work paths remain relevant (P7-001/P7-003/P7-005) | status/IRQ reads |
| U07 | Why is the aperture `0x8000` on Linux and `0x4000` on DDK? | both confirmed, revision/configuration relationship UNKNOWN | MMIO range |
| U08 | Which directory-list formula applies to each context? | Linux/PSB setup selects `0xc3c` for argument 1; DDK/EMGD list 1 selects `0xc38`. Linux/PSB teardown expression gives `0xc88` for stored context 1, named `TWOD_REQ_BASE` in the SGX535 header. All are source expressions; their hardware rationale is UNKNOWN (P7-004/P7-009/P7-014/P7-015). | MMU/contexts |
| U09 | Why do init/remove mix `gatt_start` and `mmu_gatt_start`? | target source loads stolen PFNs at the first and removes them at the second (P7-017); rationale UNKNOWN | MMU/memory |
| U10 | Complete sequence of clock/power/reset by revision? | gma500 has incomplete PM; historical OSPM missing | reset/init |
| U11 | Which errata/BRNs are valid in real silicon? | builds rev121/126 do not measure the board | reset/BIF/workload |
| U12 | Physical relationship and coherence between GTT, GATT, stolen and BIF? | partially confirmed mechanisms, end-to-end contract absent | address space |
| U13 | Semantics of fault/status: latch, clear, ordering, and ack? | handler shows practice, not contract | observation BIF/IRQ |
| U14 | Is there a platform watchdog and trusted SGX recovery? | no applicable CPU-read completion, failure or recovery contract found; disabled `TRAP_SGX_PM_FAULT` continues to `ioread32` (P7-006) | active operations |
| U15 | Complete init/deinit Scripts from DDK Poulsbo? | kernel consumes interfaces; corresponding payload/UM missing | bootstrap |
| U16 | Firmware/microkernel SGX535 verifiable and licensable? | compatible payload not available | firmware/CCB |
| U17 | Exact PDS program and bootstrap protocol? | The later xpsb-glx ELF contains PDS-related userspace generation paths (P7B-009); bootstrap program and execution protocol remain UNKNOWN | PDS/firmware |
| U18 | ABI CCB, sync, relocations and full cache? | The later ELF directly shows two PSB request numbers/sizes, a relocation builder and a scene submit path (P7B-004/P7B-007/P7B-012); complete 3D/CCB/sync ABI remains UNKNOWN | submission |
| U19 | ISA/encoder USSE and PDS SGX535 with origin? | The later ELF contains a historical USC/USSE generator and PDS-related paths (P7B-008/P7B-009); a complete clean specification, revision applicability and licensable source remain UNKNOWN | own execution |
| U20 | Streams TA/3D, DPM, tiling, formats/PBE and modern isolation? | Historical TA index-list and scene-submit paths are located (P7B-012); complete formats, DPM and isolation remain UNKNOWN | workload/render/Mesa |

## Immediate blockers of the first MMIO read

The target's passive PCI identity is recorded, but its physical SGX revision is not. The immediate blockers are U02, U03/U04, U05/U06, and the revision/errata part of U11. They require an applicable SGX535/Poulsbo register contract, power/clock/reset requirements, a gma500-owned exclusion protocol, documented MMIO failure behavior, and recovery evidence.

## Changes since Phase 3

- gma500 ownership is now identified; this shows that a separate module is unsafe, but it does not provide a universal lock.
- `CORE_ID`/`CORE_REVISION` are no longer a “conditional candidate” and remain
formally **UNKNOWN** for read-safety.
- Test Vector Zero was implemented and does not depend on MMIO.
- GTT and stolen-memory values are left unavailable when the selected passive interface does not expose them.
- Firmware/ISA continue to be later blockers, not from passive inventory.

## Phase 5 evidence-acquisition result

- Intel document `364236` is **CONFIRMED** to be titled *Intel System Controller HUB External Design Specification* (P5-001). A 2021 Intel support response placed access behind a privileged Resource and Documentation Center account (P5-002); anonymous retrieval now ends at a public 404 page (P5-005).
- Public IMG material shows detailed SGX535 documentation being discussed through private developer support, but does not supply or authenticate a register-access contract (P5-003).
- No recovered public source establishes read-only or side-effect-free semantics, the required power/clock/reset state, a graphics PCI `0x06` to SGX revision mapping, applicable BRNs, CPU read failure behavior, or a complete recovery contract. Failure to find a warning is not evidence of safety.

## Phase 6 access-contract result

- `gma_get_core_freq()` is not a passive source of SGX clock evidence: it writes a root-bridge PCI configuration selector before reading its result (P6-001). In the target gma500 path, the resulting `core_freq` is consumed by backlight PWM setup (P6-002). It does not establish the SGX execution clock or register-interface accessibility.
- The Poulsbo DDK and EMGD mirror carry 200 MHz SGX timing configuration values (P6-003). These are software configuration values, not measurements of the Dell hardware or proof of a clock-enable condition.
- The DDK warning against an SGX dump while unpowered (P6-004) reinforces that an unchecked power state is not acceptable. It does not provide a safe state, a passive proof method, or a candidate-register exception.
- The physical SGX revision, PCI-to-SGX revision mapping, applicable BRNs, read semantics, side effects, power, clock, reset, locking, PM exclusion, failure behavior, and recovery contract remain `UNKNOWN`. The detailed Phase 6 register is in [phase6-unknown-register.md](phase6-unknown-register.md).

## External artifact of greatest value

The most valuable item for the immediate blocker is an authorized register, power, reset, and errata manual for **SGX535 integrated into Poulsbo**. It must define `CORE_ID`/`CORE_REVISION` access attributes and side effects, the required power/clock/reset state, failure behavior, and revision coverage. For later bootstrap work, the matching UM/microkernel/initializer package for the Poulsbo DDK 1.14 remains the highest-value artifact.

## Phase 7.0–7.5 boundary

[Phase 7 gate and source index](phase7/README.md) rechecked the target-version gma500 lifecycle and historical DDK/PSB/EMGD paths. U01–U06, U08–U17 remain open within their stated hardware scope. U07, U18–U20 were not experimentally revisited. Source expressions resolved more precisely for U08/U09, but did not resolve their physical meaning. No new `OBSERVED` or `REPRODUCED` fact exists. FIRST-OBSERVATION-DESIGN and Gate B remain BLOCKED; CORE_ID/CORE_REVISION remain SAFE-CANDIDATE NO, whitelist `[]`. Phase 7.1 did not run. The [Phase 7 final report](phase7/phase7-0-5-final-report.md) lists exact requirements before any hardware execution.

## Phase 6.2 evidence routes

Phase 6.2 does not resolve U01–U06 or U11. It records that an authorized Intel RDC/support request, an Imagination support inquiry, and carefully scoped public professional questions are legitimate evidence routes. Their availability does not establish that any recipient can provide the missing contract. Public-source archaeology has diminishing returns for the immediate hardware-safety properties; the status is `RE-PREPARE`, while Gate B remains `BLOCKED`. See [the Phase 6.2 exhaustion assessment](phase6-2-documentation-exhaustion.md) and [RE-GATE](phase6-2-re-gate.md).

## Recovered xpsb-glx evidence (later static pass)

The [Phase 7 binary track](phase7/psb-dri-re/README.md) adds P7B-001–P7B-014. It constrains U15 and U17–U20 on the **historical userspace** side, as detailed in the [binary-specific unknown register](phase7/psb-dri-re/unknowns.md). It does not establish the Dell's installed userspace, physical SGX revision, safe identification-register read semantics, clock/power/reset requirements, firmware identity or recovery behavior. The first-observation gate, whitelist and CORE_ID/CORE_REVISION SAFE-CANDIDATE states are unchanged.

## Xpsb and candidate libdrm/PSB ABI follow-up

The [Xpsb static pass](phase7/xpsb-re/README.md) adds P7C-001–P7C-013 without replacing P7B. The open DDX 0.32.0 source creates the 32-byte `PsbDRIRec`; the retained DRI binary checks that size and reads matching offsets. The retained `Xpsb.so` is a separate X-server module: it accepts the DDX mapping and DRM fd, initializes historical XHW/BO state, and has direct SGX register writes. The public `libdrm-poulsbo` 2.3.0 source supplies a candidate BO/fence ioctl map. Exact binary/source pairing, full bootstrap, static program meaning, command formats and firmware provenance remain open; see [P7C unknowns](phase7/xpsb-re/unknowns.md). This historical code does not establish safe MMIO read semantics on the Dell. U01–U06, U10/U11 and U14 remain UNKNOWN; Gate B is BLOCKED, CORE_ID/CORE_REVISION are not SAFE-CANDIDATE, whitelist `[]`.

The later P7D static pass narrows these historical-stack gaps without changing the hardware gate. The [440-byte source trace](phase7/xpsb-re/static-buffer.md) confirms an immutable `.rodata` source, BO copy/lifetime and a separate composite-template accessor; the uploaded BO's actual consumer and GPU semantics remain UNKNOWN. The [stronger kernel-source pairing](phase7/xpsb-re/version-pairing.md) finds `5.0.1.0046` in both retained ELFs and the public 4.41.1 PSB header with agreeing direct-request sizes, but exact built DDX/libdrm/kernel pairing remains UNKNOWN. The public [psb-firmware 0.30-4 package](phase7/xpsb-re/bootstrap-dependencies.md) contains only the MSVDX/video file, resolving that package-specific question without proving whether another SGX bootstrap payload exists. P7D-001–P7D-008 and [P7D unknowns](phase7/xpsb-re/unknowns.md) preserve the remaining boundaries. Gate B is still BLOCKED; whitelist `[]`.

The later P7E pass resolves one historical control-flow edge: a mode-3 frame-tracker exit calls the installed renderer finalizer, which submits the retained scene through the index-0 command wrapper ([P7E draw trace](phase7/psb-dri-re/draw-to-submit.md)). This narrows U18/P7B-U02/P7D-U03 without closing them: the complete GL frontend invocation, generated PDS/USSE and TA formats, exact paired ABI, SGX bootstrap and synchronization contract remain UNKNOWN. [Clean-room readiness](phase7/psb-dri-re/clean-room-readiness.md) is **NOT YET SPECIFIABLE**. Hardware U01–U06/U10/U11/U14 and Gate B remain unchanged; no hardware state was modified.

The P7F [bounded-path trace](phase7/psb-dri-re/bounded-path-closure.md) narrows U17–U20 further. One conditional scene branch has fixed vertex/index copies, two derived eight-byte USE/USSE instruction slots and selected TA literals; these do not specify a complete valid draw or target-revision behavior. It corrects the earlier `0x00030ffd` buffer label from PDS to USE/USSE (P7F-003) while retaining the separate PDS-related output path. Exact program/packet semantics, broad scene state, bootstrap, BO/fence values and the historical GL entry remain UNKNOWN. [Result B](phase7/psb-dri-re/preimplementation-decision.md), hardware UNKNOWNs and Gate B are unchanged.

## Frozen triangle static checkpoint (P7G)

[P7G](phase7/psb-dri-re/frozen-draw-closure.md) closes the historical GL-entry portion of P7B-U02/U18 and removes the falsely identified terminal-padding gap. It corrects the old setup geometry to eight four-dword positions. Seventeen state atoms, conditional width-eight vertex output and a three-index TA record are constrained. U17–U20/P7B-U02 remain **partially constrained**, not resolved: selected fragment words/register metadata, scene pixel/background records, PDS semantics, complete BO/fence lifecycle and bootstrap are still missing. The [FG-01–FG-07 table](phase7/psb-dri-re/frozen-draw-blockers.md) gives exact producers and next static sources; it does not claim all local binary evidence is exhausted. Candidate-kernel XHW readiness is now a confirmed software prerequisite, not a hardware power/reset guarantee. The physical core revision and all hardware-safety UNKNOWNs are unchanged; Gate B BLOCKED, whitelist `[]`.

The [focused fragment checkpoint](phase7/psb-dri-re/frozen-fragment-compiler-checkpoint.md) narrows FG-01's *input*: the primary-color MOV's UniFlex destination class/index, source class/index and swizzle are now mapped, and the conditional `OutputsWritten & 4` insertion does not run for this draw. The final instruction count comes from compiler function `0x001c95f7` after lowering. The transformed IR, final USSE words and resource metadata remain **UNKNOWN**; this is not an SGX revision or hardware-safety finding (P7H-001/P7H-002).

For FG-07, the candidate historical DRM core's private-ioctl dispatch resolves one narrower concern: its disabled full-word check and caller-directed input copy allow the candidate XHW-init call despite the descriptor's opposite direction annotation (P7H-003). The exact built libdrm, DDX and kernel pairing remains **UNKNOWN**; this source-only correction does not imply the old stack worked on the Dell or that current gma500 accepts these ioctls.

P7H-004 narrows FG-01 to ordinary four-component scalar lowering of the selected primary-color MOV. This is an internal compiler stage, not final USSE words/counts or resource metadata. P7H-006 gives conditional field formulas for the fragment link object, but cannot instantiate its compiler-derived fields. P7H-005 narrows FG-06: the retained Xpsb initializer supplies operation zero and the communication-BO handle, while the candidate kernel consumes those fields and later requests scene-info/bind-fire operations 1/2. Its three unused 32-bit input fields were not initialized in the retained routine, but the candidate operation-zero handler does not read them. The [current frozen-draw decision](phase7/psb-dri-re/frozen-draw-static-decision.md) keeps Result B and separates unfinished static compiler/scene work from missing PDS consumption evidence and exact pairing. U17–U20 remain partially constrained. Gate B BLOCKED, CORE_ID/CORE_REVISION not SAFE-CANDIDATE, whitelist `[]`.

P7H-007 adds a dependency, not a revision measurement: historical `Xpsb_set_vopt` derives an option from SGX-mapping-relative `+0x14`; `Xpsb_scene_info` uses that option to choose a scene-sizing branch. P7H-008 checks both branches for the frozen 32×32 target: cookie words 0–14, size and clear-page outputs coincide, so the unknown option does not block this specific sizing calculation. Cookie word 15 is unwritten there, but the candidate initial successful bind/fire branch does not read it; later OOM behavior remains UNKNOWN. The Dell's SGX core revision and option value remain UNKNOWN. This does not permit reading the register or infer PCI revision `0x06` → any SGX revision.
