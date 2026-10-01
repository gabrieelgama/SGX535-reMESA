# Phase 7.0–7.5: controlled reverse-engineering preparation

**Final Phase 7 status: COMPLETE WITH UNRESOLVED EVIDENCE BOUNDARY.**
The [final handoff](psb-dri-re/CODEX-HANDOFF.md) is the current checkpoint:
the selected program and clean-room host image are reproducible, source
containment remains unproved, and Gate B is BLOCKED with whitelist `[]`.
The entries below preserve earlier checkpoint chronology; their older
no-MMIO-observation and next-step statements are superseded by the final handoff.

**Mini 12 H0 continuation:** the [read-only session](../hardware-evidence/MINI12-20260927-H0/results.md)
now includes an operator-provided, hashed kernel-log snapshot, a narrow
[installed-module check](../hardware-evidence/MINI12-20260927-H0/installed-module-static-check.md),
and [individual Gate B reassessment](../hardware-evidence/MINI12-20260927-H0/gate-b-reassessment.md).
The 2 h 9 min 36 s clock difference is only a host-target offset; neither clock
was externally verified. The operator reports physical recovery access but did
not test it. Gate B remains BLOCKED, whitelist `[]`; no SGX experiment occurred.

**Mini 12 H0 passive capture (2026-09-27):** [fresh target result](../hardware-evidence/MINI12-20260927-H0/results.md)
matches TVZ-001 across overlapping identity and software fields. The audited
probe ran once, exit 0, as unprivileged `gama`; raw output, exact commands,
timestamps and hashes are preserved. Kernel-log access remains a baseline gap.
No SGX MMIO or GPU command occurred. Gate B remains BLOCKED, whitelist `[]`;
R1/R2/R3 and static closure are unchanged.

**First Mini 12 hardware session preparation (2026-09-27):** the
[H0 session record](../hardware-evidence/MINI12-20260927-H0/README.md)
audits the passive probe, preserves the 16-item Gate B checklist and sketches
conditional R1/R2/R3 discriminators. This ARM64 workspace is not the target;
fresh Mini 12 access and baseline are pending. No new SGX observation occurred;
Gate B remains BLOCKED and whitelist `[]`.

**P7H-060:** [Strong clean-room closure report](psb-dri-re/cleanroom-final-closure.md)
eliminates historical stale-hole, timeout-continuation and mask-`0x7`
dependencies as new-driver policies. Three hardware implications still prevent
R1/R2/R3 and static closure. The historical PDF is optional; Gate B stays
BLOCKED. 42 PDS +114 triangle tests pass.

**P7H-059:** [SGX535 PDS document hunt](psb-dri-re/sgx535-pds-document-hunt.md)
found archived public SDK documentation paths, but not the named Eurasia PDF
or a qualified public PDS encoder. R3/B1/L12 remain OPEN; no hardware touched.

**P7H-058:** [External three-rule acquisition](psb-dri-re/final-rules-external-evidence.md) leaves R1/R2/R3 unresolved; 42 PDS +108 triangle tests pass; static triangle PARTIAL and Gate B BLOCKED.

**Route C update (P7H-036/037):** the restricted new implementation no longer
requires historical target-provider identity. The
[clean-room backing contract](psb-dri-re/cleanroom-backing-contract.md) and
checker establish a deterministic parameterized CPU image, including the nine
zero holes. Complete PDS launch-state coverage remains unproved even with an
ideal BO provider, and concrete GPU-provider refinement remains conditional.
FG-02 stays OPEN; ISA UNKNOWN; Gate B BLOCKED; whitelist `[]`.

The immediate handoff is Phase 6.2 commit `67fe07b`; earlier baselines are `9c5b28e` and `16899a1`. Its Gate A is GO, Gate B BLOCKED, RE-GATE RE-PREPARE, both identification-register SAFE-CANDIDATE states NO, and whitelist `[]`. No TV1 design or execution was carried forward.

This run completes documentary review in order. Each gate is recorded before dependent work proceeds:

| subphase | record | scope |
|---|---|---|
| 7.0 | [architecture](7.0-experimental-architecture.md), [ownership](7.0-ownership-locking.md), [failure/recovery](7.0-failure-recovery.md), [gate](7.0-gate.md) | decide whether a safe first observation can be designed |
| 7.1 | [observation and gate](7.1-first-observation.md) | conditional; blocked by 7.0 |
| 7.2 | [identity/revisions/errata](7.2-revision-errata.md) | static reconstruction |
| 7.3 | [power/clock/reset](7.3-power-clock-reset.md) | static state-transition model |
| 7.4 | [BIF/MMU/address space](7.4-address-space.md) | static translation model and divergences |
| 7.5 | [firmware/microkernel](7.5-firmware-microkernel.md) | static provenance and initialization dependencies |
| review | [red team](phase7-0-5-red-team.md), [final report](phase7-0-5-final-report.md) | audit gates, sources and scope |

Sources, exact revisions, hashes and licenses are indexed in [source provenance](source-provenance.md); new evidence IDs resolve through the [project matrix](../evidence-matrix.csv). Source code evidence describes software, unless a hardware guarantee is explicitly cited.

CONFIRMED means directly established within the stated source scope; INFERRED explains a deduction; UNKNOWN means unestablished. OBSERVED is a target measurement, REPRODUCED requires repeated equivalent measurements, NOT-TESTED means no attempt, and BLOCKED denotes a gate decision. Static source inspection creates no OBSERVED hardware facts.

The recorded machine is Dell Inspiron 1210 / 0X605H / BIOS A02, graphics `0000:00:02.0`, `8086:8108`, PCI revision `0x06`, subsystem `1028:02b1`, class `030000`, IRQ 16. BAR0 is `d8100000–d817ffff`, BAR1 `1800–1807`, BAR2 `d0000000–d7ffffff`, BAR3 `d8380000–d839ffff`. Kernel reported: `5.10.240-antix.1-486-smp`, i686, gma500/card0. These are inherited [TVZ-001 report facts](../hardware-evidence/TVZ-001/README.md), not new measurements or proof of the present machine state.

No Phase 7.6 work is included.

The later retained-ELF [FG-02 continuation](psb-dri-re/frozen-fragment-link-progress.md) derives an eight-byte link-time fragment suffix and a four-byte selected secondary record from the closed FG-01 compiler result. The [selected PDS boundary](psb-dri-re/selected-pds-hard-boundary.md) records the producer-unwritten committed data words and the missing SGX535 instruction read set. These CPU-side findings do not change Gate B or establish a runnable first triangle.

The [PDS read-set search ledger](psb-dri-re/pds-readset-search-ledger.md) follows the selected literal through DRI and Xpsb emitters, compares nearby PDS-shaped words and records public-source leads and rejections. P7H-016–018 establish reuse patterns and an outbuf path without zeroing; the hardware read set remains UNKNOWN.

The [external Series5 PDS evidence report](psb-dri-re/series5-pds-external-evidence.md) records a further targeted search of public code, package, archive and patent routes. It did not establish a provenance-qualified SGX535 source-selector or terminal read rule. FG-02 and the first-triangle specification remain blocked at the selected PDS read set.

The later [CPU semantic-corpus analysis](psb-dri-re/pds-hypotheses.md) accounts arithmetically for the `0x070b0345` counterexample and records a competing fixed-extra-source model. Its [42-row classification](psb-dri-re/pds-semantic-corpus.csv) is static software evidence only; the `0x07000345`/`0xaf000000` GPU read sets remain UNKNOWN. FG-02 and Gate B remain blocked.

[Experiment #43](psb-dri-re/pds-experiment43.md) checks retained emitters outside
that corpus and a quarantined older related package for a new controlled PDS
variation. No distinguishing `0x45`/`0xaf` variant was found. Bits 16–17 of
the event `0x070b0345` word are literal/invariant at their retained producer;
the selected GPU read set remains UNKNOWN and FG-02 stays open.

The later [PDS buffer lifecycle trace](psb-dri-re/pds-buffer-lifecycle.md)
separates first backing allocation from fenced batch-pool slot reuse. It
establishes that the nine primary holes can inherit stale mapped bytes while
the complete 56-byte region is CPU-writable. A deterministic new-driver byte
image is possible; its equivalence to the historical PDS task is still
UNKNOWN. Gate B and whitelist `[]` are unchanged.

## Later recovered Poulsbo DRI binary

The [xpsb-glx `psb_dri.so` static-analysis track](psb-dri-re/README.md) was added after the 7.0–7.5 source review. It preserves the earlier gates and evidence IDs. Historical DRI and command behavior can be reconstructed from its bytes without executing the binary; the first-observation gate remains BLOCKED and no hardware result was added.

The final pre-implementation P7E pass resolves the historical binary's indirect mode-3 scene-finalization/submission edge in [the draw trace](psb-dri-re/draw-to-submit.md). [Format readiness](psb-dri-re/format-readiness.md) and [clean-room readiness](psb-dri-re/clean-room-readiness.md) still find a minimal 3D userspace path **not yet specifiable** without undocumented semantics. This is static evidence only; Gate B and whitelist `[]` are unchanged.

The follow-on [P7F bounded-path pass](psb-dri-re/bounded-path-closure.md) recovers two conditional USE/USSE instruction slots, a fixed six-index scene branch and additional validation/bootstrap constraints. It corrects an earlier label that had treated the USE instruction buffer as PDS. The [strict decision](psb-dri-re/preimplementation-decision.md) is still Result B; no implementation-grade specification or Phase 8 plan was created.

The [P7G frozen triangle](psb-dri-re/frozen-draw-closure.md) extends the existing binary track without restarting it. Result B remains: fragment output/link metadata, complete state/scene programs, selected resources and bootstrap still prevent an implementation-grade specification. Gate B is BLOCKED; no hardware observation occurred.

The focused [P7H static decision](psb-dri-re/frozen-draw-static-decision.md) includes the now-closed [FG-01 compiler result](psb-dri-re/frozen-fragment-exact-output.md): the frozen MOV/END input produces no main or secondary USSE instruction bytes. Fragment linking and scene pixel state remain open. The candidate-family [XHW init request](xpsb-re/xhw-init-frozen-draw.md) and convergent 32×32 scene-info branches remain as recorded; cookie word 15 is unwritten. Result B, Gate B BLOCKED and whitelist `[]` remain unchanged.

P7H-029–031: [PDS stack pairing / first use](psb-dri-re/pds-stack-pairing.md)
proves candidate generic DRM ABI equality and selected first-range
non-overlap, including nested clear-secondary emission. Actual target
fresh-zero allocation semantics remain conditional; no ISA or safety gate
is reclassified.

P7H-032–035: [PDS backing-provider audit](psb-dri-re/pds-backing-provider.md)
adds exact OBS lineage, additional public provider/patch comparisons, and
placement exclusions. The bounded historical contract is proven; the frozen
target's supplying-provider membership remains CONDITIONAL. No ISA or gate change.

P7H-038/039: [selected launch-state coverage](psb-dri-re/pds-launch-state-coverage.md) records CPU launch packing and the unproved L12 launch-definedness rule. The checked inventory stays OPEN; no PDS semantics or GPU backend claim is promoted.

P7H-040: [launch count distinction](psb-dri-re/pds-launch-count.md). Middle-word Q−1 and reference-word PDS data size are separate; L12 and FG-02 remain OPEN.

P7H-041: [selected PDS availability](psb-dri-re/pds-selected-source-availability.md); source eligibility remains UNKNOWN, with explicit checker separation of source questions and control obligations.

The [independent frozen-triangle map](psb-dri-re/frozen-triangle-spec.md) (P7H-042–046) freezes L12 and advances target/background, state uploads, TA order and submission templates. The specification remains PARTIAL; its hypothetical L12-closed view retains four independent static-contract blocker groups.

[P7H-047–051 blocker burn-down](psb-dri-re/frozen-triangle-burn-down.md) closes
the selected static target-layout contract and adds ten backing roles, six
user validation entries and49 wire relocations. Three independent groups remain
even if L12 closes: auxiliary inputs, device publication and service ready state.
