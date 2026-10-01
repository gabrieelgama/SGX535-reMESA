# Archived SGX535-reMESA handoff chronology

This preserves the prior handoff's incremental addenda and recovery commands. Its repeated “latest” and “next step” paragraphs reflect earlier checkpoints and are **not current instructions**. Start with [the current handoff](CODEX-HANDOFF.md) and the Mini 12 H0 gate reassessment instead.

# SGX535-reMESA — Codex handoff

Prepared **2026-09-26**, after P7H-032–035. This file is intended for a fresh
Codex account/session with no conversation history. It records repository state,
not a new research result. The last instruction was to **stop research and write
this handoff only**. No remaining blocker was solved during handoff preparation.

Repository root in the departing environment: `/home/gama/sgx535-gfx`.
Treat paths below as repository-relative unless explicitly absolute. A new
environment may mount the repository elsewhere. **Transfer the entire working
tree, including untracked files—not just Git HEAD.** Much of the latest evidence
is uncommitted/untracked and would be absent from a normal fresh clone.

## NEXT CODEX: START HERE

**Mini 12 H0 continuation / Gate B reassessment (2026-09-27):**
[H0 results](../../hardware-evidence/MINI12-20260927-H0/results.md),
[hashed manifest](../../hardware-evidence/MINI12-20260927-H0/baseline-manifest.json),
[module static check](../../hardware-evidence/MINI12-20260927-H0/installed-module-static-check.md),
[recovery protocol](../../hardware-evidence/MINI12-20260927-H0/recovery-protocol.md),
and [16-row gate audit](../../hardware-evidence/MINI12-20260927-H0/gate-b-reassessment.md)
are current. Correct the old wording below: two UTC-formatted host readings
differed by about 2 h 9 min 36 s, but neither clock was independently checked
against UTC. One user-approved `sudo -n dmesg --color=never` attempt returned
"a password is required"; no password was requested or supplied. The operator
then provided a readable 45,575-byte target log (SHA-256
`72974ea9c86ebf06b3af1ce392cac2023d56b8335724e1907b35e25e484ce9ec`),
whose generation command/time were not directly observed. It includes gma500
initialization and ACPI PowerButton event setup failures. The installed
`gma500_gfx.ko` and loaded sysfs Build IDs match; static ELF evidence confirms
the `8086:8108` BAR0 + `0x40000` / `0x8000` mapping, not register safety. The
operator reports physical keyboard/power-button and known-working boot, but no
serial/watchdog/switched power and **no destructive recovery test**. Gate B
remains BLOCKED, whitelist `[]`; no first SGX experiment is authorized. No
GPU submission, MMIO, reset or power manipulation occurred.

**Mini 12 H0 passive target capture (2026-09-27):** [fresh result](../../hardware-evidence/MINI12-20260927-H0/results.md)
and [hashed manifest](../../hardware-evidence/MINI12-20260927-H0/baseline-manifest.json)
supersede the pending-access status below. After two rejected key attempts,
key-only SSH succeeded. Passive DMI/CPU/PCI preflight matched the Dell Inspiron
1210, board `0X605H`, BIOS A02, Atom Z520 and `8086:8108`. The audited probe
ran **exactly once** as unprivileged `gama`, exit 0, with raw stdout/stderr,
exact command, script hash and local UTC times preserved. Fresh PCI revision
`0x06` is still not SGX core revision. The kernel, `gma500`/`card0`, BARs and
runtime-active status match TVZ-001. Debian 13.4, Xorg and installed
`gma500_gfx.ko` hash are new scoped observations. Unprivileged `dmesg` was
denied; `journalctl`/`modinfo` were absent, so kernel-log baseline is a gap.
The hosts' UTC-formatted readings differ by about 2 h 9 min 36 s; neither
clock has an independent UTC reference, so neither is diagnosed as wrong. **No** SGX
MMIO, submission, reset, power change, experimental ioctl, proprietary binary
or privilege escalation occurred. Recovery is unproved; Gate B remains BLOCKED
and whitelist `[]`. R1/R2/R3, B1/B3/B4/L12 and FG-02 do not close from passive
identity. B2 and FG-01 stay CLOSED; static triangle PARTIAL.

**First Mini 12 hardware-session preparation (2026-09-27):**
[H0 record](../../hardware-evidence/MINI12-20260927-H0/README.md),
[probe audit](../../hardware-evidence/MINI12-20260927-H0/probe-audit.md),
[Gate B checklist](../../hardware-evidence/MINI12-20260927-H0/gate-b-checklist.csv),
[observability map](../../hardware-evidence/MINI12-20260927-H0/hardware-observability.csv)
and [conditional experiment designs](../../hardware-evidence/MINI12-20260927-H0/experiments.md)
prepare the first target session. The current shell is ARM64 Debian and is **not**
the Mini 12; a fresh access method and target baseline are pending. TVZ-001 is
inherited operator-report evidence only. Passive probe tests pass (4); the
42 PDS and 114 triangle tests still pass. No new target measurement, MMIO,
submission, reset or power operation occurred. Gate B stays BLOCKED,
whitelist `[]`; R1/R2/R3 and static closure remain unchanged. If target access
arrives, run only the passive probe first, preserve raw output/status/time/hash,
then capture the audited read-only baseline. Do not run conditional experiments
or complete-image submission while the gate blocks them.
Follow-up: `gama@192.168.18.90:22` was supplied, but the first identity-only
SSH attempt failed authentication before remote execution. Raw stdout/stderr,
command, UTC times, status and hashes are under the H0 record. A dedicated
local ED25519 key was generated outside the repository; its public key was
provided for installation on the target. No password was requested or stored.
Fresh target identity, passive probe and baseline remain pending. The accepted
first-use SSH server key is not independent target identification.
After reported key installation, one key-only retry and one `ssh -v` diagnostic
still failed before remote execution. The debug trace shows the dedicated
ED25519 client key was offered and rejected by the server. Captures and hashes
are in the H0 session record; the user was asked to correct account/key
placement or permissions. Do not call this a Mini 12 measurement or run the
probe until passive identity preflight actually succeeds.

**Latest checkpoint P7H-060 — stronger clean-room contracts
(2026-09-27):** [Closure report](cleanroom-final-closure.md),
[R1/R2/R3 policy-vs-fact table](cleanroom-final-rules.csv),
[publication domain matrix](cleanroom-publication-domains.csv) and
[PDS source-domain matrix](cleanroom-source-domains.csv) record the current
boundary. The BO resolver now zeroes all bytes of every supplied user BO before
writing selected fields and relocations; all six user roles are size-checked.
Publication and bootstrap guards require all selected BO coverage, three
successful maintenance phases, qualified revision provenance and fresh TA load
mask `0xf` including LOAD3. These are **ENFORCED_CONTRACT host policies only**;
hardware payload visibility, LOAD3/INITEND readiness and complete PDS source
eligibility remain unproved. R1 OPEN, R2 OPEN, R3 OPEN; B3 CONDITIONAL, B4/B1/L12
OPEN, B2/FG-01 CLOSED, FG-02 OPEN. Static triangle PARTIAL, complete REFUSES;
Gate B BLOCKED, whitelist `[]`, hardware unexecuted/unverified. Historical
PDF is OPTIONAL FUTURE EVIDENCE, **not a project prerequisite**; do not renew
its hunt. 42 PDS +114 triangle tests pass. No references/ or hardware changed.

**Latest checkpoint P7H-059 — SGX535 PDS public-document hunt
(2026-09-27):** [Focused hunt](sgx535-pds-document-hunt.md),
[search ledger](sgx535-pds-document-hunt.csv), and
[source/hash registry](sgx535-pds-document-sources.json) preserve new
historical archive paths. Official 2008 SDK download pages lead through a
login; archived 2011/2012 public SDK documentation indexes list 11/22 PDFs but
not the named Eurasia manual. A verified public Series5 architecture guide in
the same docs tree names SDK `REL_2.10@863987` but contains no PDS/DS0/DS1
rule. The exact PDF, its original URL/bundle, and legitimate public
distribution remain UNVERIFIED. No public Series5 PDS encoder was recovered.
Hunt level 1 is an archival lead only, **not document recovery**. R3/B1/L12
and all other static statuses remain unchanged; no tests/models/hardware were
run or modified. Cache `/tmp/sgx535-doc-hunt` contains hashed public-page
snapshots and one neighbor PDF; `references/` remains unchanged.

**Latest checkpoint P7H-058 — external acquisition (2026-09-27):**
[External evidence result](final-rules-external-evidence.md),
[query ledger](final-rules-external-search.csv), and
[source/hash registry](final-rules-external-sources.json) record six new public
source reviews and nine hashed downloads. Two official SDK manifests do not
identify a PDS encoder; a new memory-management patent does not specify LOAD3;
an independent GPL SGX535 header corroborates bit8 without its consumer contract;
Linux DMA/barrier docs cover host contracts, not SGX completion domains.
**No rule closed:** R1/B3 CONDITIONAL, R2/B4 OPEN, R3/B1+L12 OPEN. FG-02 OPEN;
complete REFUSES.42 PDS +108 triangle tests pass. Access failures are recorded,
not treated as negative source evidence. No new ISA inference, provider survey,
or corpus reconstruction. Most valuable next evidence is a qualified Series5
PDS launch/operand rule; R1/R2 need exact cache/DPM completion postconditions.
The unseen Eurasia PDF is VERY LIKELY to discuss the needed class of information
based only on the public forum; its contents and distribution rights remain
unverified. Cache bodies are ephemeral in `/tmp/sgx535-final-external`; URLs,
SHA-256 and pinned SDK commits are persistent. No code or references/ changed.


**Latest checkpoint P7H-055–057:** see the final three-rule section of
[the burn-down](frozen-triangle-burn-down.md). R1 publication, R2 bootstrap and
R3 source eligibility remain unresolved; static triangle PARTIAL, FG-02 OPEN.
LOAD3 has no CPU revision guard under flags1f. Following op9 through fresh TA,
selected raster helper0x4030 and the candidate IRQ path finds no later explicit
status118:8 wait. TA waits status12c:100a40; selected raster flags15/cookie14=0
add no scene-load wait; IRQ status2 mask10 excludes bit8. The actual internal
LOAD3 consumer and any implicit dependency remain UNKNOWN. Do not assert that
LOAD3 is unnecessary, a hardware bug, or fixed by changing7 to15.

A new observation guard rejects stale pre-request status, missing LOAD3 bits,
timeouts and failed clear. It returns OBSERVATIONS_ONLY; a fresh successful
cycle still needs an applicable hardware postcondition. Five canonical PDS
families consume prior42-row/field/differential/bit-influence results without
regenerating them. No source family or L12 closes. Four families' literals were
already in the corpus; the event has15 additional distinct fixed words but no
new controlled source-selector variation. All instruction read/definition
semantics remain UNKNOWN in the pre-definition timelines.

The scoped49-record CPU wire ledger is COMPLETE: no missing emitted site was
found. The whole-path contract remains PARTIAL because service-generated scene/
TA contents and publication are unproved, not because of fake-address placeholders.
Use the existing bundle and new load-path/later-wait/family/discriminator CSVs.
The exact MODEL A/B and narrowest distinguishing evidence for each rule are
persisted. Current validation:42 PDS +108 triangle tests; all four complete
modes refuse; ELF and20 source hashes match; independent read-only review found
no defect. Index empty; references/ unchanged. No new source acquisition; psb_irq.c is now the20th recorded source
hash. The restored0x4030 decompilation stays in /tmp and is reproducible using
the retained ELF and existing `DecompileXpsb.java`/targets. No hardware access.


**Previous checkpoint P7H-052–054:** the final static closure attempt is recorded
in the last section of [the burn-down](frozen-triangle-burn-down.md). Static
triangle PARTIAL; FG-02 OPEN; B2 CLOSED. B3 publication, B4 readiness, and
B1/L12 source eligibility remain distinct obligations. Do not repeat the
producer searches to try to turn successful CPU execution into architectural
proof. The next useful evidence must distinguish one of those contracts.

New adversarial fact: TA flags0x1f kick four paths, but retained Xpsb0x3df0
polls mask7 at status118; the existing SGX535 header assigns DHOST bit8.
Raw instruction0x3f51 ORs4 for flag8. This is not a proven hardware bug or a
license to patch the mask. Xpsb0x4e10 ignores clear-poll failure in its returned
status. Op9 propagates its result; op4 returns zero after initialization writes
without a ready poll. Neither reply proves complete target readiness.

Use `frozen_triangle_closure.py` for the shared evidence ledger and
`frozen_triangle_image.py --bundle` for the joined package. Eleven auxiliary
records group into five distinct instruction streams; AUX-09/10 have no explicit
scoped launch reference but remain in the allocation audit. All complete modes
still refuse. New guards cover publication ordering/timeouts, missing source
inventory/provenance, false readiness and unsupported evidence promotion.
Validation:42 PDS and96 triangle tests pass (71 retained plus25 new). All four
complete modes refuse; both ELF hashes and19 source hashes match. Nothing is
staged and references/ is unchanged.
The source-domain group retains separate family/context obligations; no narrow
selector bit, actual outside read or persistent read was established.
Historical claims, PDS semantics, Route C policy and Gate B are unchanged.


**Previous checkpoint P7H-047–051 (retained history).** Read
[frozen-triangle-burn-down.md](frozen-triangle-burn-down.md) first. B2/FT-TARGET
is CLOSED for the selected linear ARGB8888 shared-surface software contract;
hardware validity is not claimed. B3 has ten backing roles, six user validation
entries,49 wire records, symbolic command packing and explicit provider MMU
limit/USE reservation inputs. B3 remains CONDITIONAL on device publication
(cache/TLB completion); `wmb` and same pages are insufficient. B1 inventories
all eleven auxiliary PDS records with defined CPU images but UNKNOWN complete
input domains. B4 has five revision-branch CPU evaluations and TA-cookie
packing; target-qualified bootstrap/TA-table ready state remains OPEN.

Use `frozen_triangle_image.py --bundle` for the combined model. Complete mode
must still reject. If L12 closed today, FT-AUX, FT-BO-publication and FT-SERVICE
remain; FG-02 is not otherwise complete. L12 stays OPEN/FROZEN. Do not repeat
provider surveys or PDS/Q archaeology. Next work should establish the precise
device-publication or service-ready contract, not rebuild this BO plan. Run
42 PDS tests plus71 triangle tests (24 existing,22 BO,25 contract). One existing
target test now requires closed layout and conditional backend. The source
hash list includes newly inspected existing public files; no new acquisition.
P7H-051 narrows publication to the completed selected invalidation sequence: an
existing SGX535 register snapshot names+ad4/+ae0 but lacks status+138 masks.
Retained callers ignore helper timeout failures; the clean-room model aborts.
Historical identity/holes/ISA conclusions and Gate B are unchanged.


**Previous strategy checkpoint P7H-042–046 (retained history):** L12 is OPEN / FROZEN as an external
dependency. Read [the frozen triangle specification](frozen-triangle-spec.md)
and its generated object, relocation and blocker tables first. Do not restart
source-domain research. The partial serializer now contains51 nodes and38
relocation sites (seven are aliases in the68-byte TA stream). The bounds upload
is11 dwords, correcting the old ten-word label; triangle upload after cache
difference is14. FT-ORDER closes under the explicit fresh/full-bounds/software
vertex inputs. Target/event/background CPU words and52-word raster list are
parameterized, and the144-byte command has explicit runtime parameters.
P7H-045 extends explicit CPU zero-initialization to the enumerated auxiliary
PDS images, preserving historical producer-unwritten markers. FT-AUX remains a
consumed-state coverage obligation, not an unknown CPU byte-image problem.
P7H-046 records the selected op2 service requests: engines0/1, flags4/15.
Scene-less op0 is excluded; selected handlers do not consume cookie15.
FT-SERVICE now targets revision-dependent bootstrap/ready-state requirements
(0x4af0/0x3820), not another request-number search. Start the next independent
work at FT-BO: instantiate the underlying BO/validation/USE-base plan for the
listed objects, preserving the established primary first-use model.
If L12 closed, FT-AUX, FT-TARGET, FT-BO and FT-SERVICE would remain. These are
separate from backend preservation and Gate B. Run the42 existing PDS tests plus
24 frozen-triangle tests; complete-image mode must fail. New code never opens a
device. Do not assume all source avenues for remaining service/BO work are
exhausted; the report identifies exact producers rather than another broad survey.


**Latest checkpoint P7H-041:** read
[pds-selected-source-availability.md](pds-selected-source-availability.md).
The selected source-domain rule remains UNKNOWN; neither an outside read nor
persistent readable state is established. The public SGX535 user description
supports two temporary-set questions, not selected reachability or zeroing.
Do not repeat that forum/reset/context check. Do not treat Q as a preload bound.
The existing checker now derives source blockers and control obligations from
explicit inventory columns and separately requires architectural envelope
proof. All36 previous tests are retained plus six new availability regressions.
L12 and FG-02 remain OPEN; concrete backend preservation stays CONDITIONAL.
The one discriminator is whether the complete pre-definition source domain of
`07000345;af000000` is confined to deterministic launch state. There is no
supported selector-bit number to guess and no authorization for reset/experiments.


**Latest narrow continuation: P7H-040.** Read [pds-launch-count.md](pds-launch-count.md).
The middle-word `0x00030000` is `(Q-1)<<16` with Q=4; it is NOT the
primary-reference data-count field `0x0c000000`. The quotient depends on USE
resource requirements, not PDS prefix size. Twelve scoped arithmetic cases
are explicitly labeled; they are not twelve historical executions. Background
DRI/Xpsb serializers corroborate separate fields. No architectural unit or
state-eligibility rule is established. The next L12 question is whether state
outside the initialized prefix domain is unavailable to the selected program
until defined. Do not repeat this count survey. Route C remains selected,
L12/FG-02 OPEN, GPU preservation CONDITIONAL. No prior evidence IDs changed.


**Latest checkpoint: P7H-038/039, 2026-09-27.** Read
[pds-launch-state-coverage.md](pds-launch-state-coverage.md) and
[pds-launch-state.csv](pds-launch-state.csv). P7H-038 records the selected CPU
launch words `R(S), 0x00030000, R(P)|0x0c000000`; P7H-039 leaves **L12**, the
hardware initialized-state domain of that launch, UNKNOWN. Count bits31:26
hold3, derived from CPU data count12. Neither the count nor the BO extent is a
proven internal-store bound. Linked USSE temp_count zero is not PDS temporary
initialization. Do not repeat the descriptor trace or register-name checks.
Run the ten new launch-model tests plus the existing16 image tests;
`pds_launch_state_check.py --require-closed` is expected to fail. Its default
exit0 means inventory integrity, NOT architectural closure. The next evidence
must establish L12's DS preload/definedness and other-source eligibility rule;
do not replace that with another source survey or presumed external read.
GPU backend preservation remains separately CONDITIONAL. FG-02 remains OPEN.

**Current project decision, 2026-09-27: ROUTE C selected.** Read
[cleanroom-backing-contract.md](cleanroom-backing-contract.md) first.
Historical provider identity is no longer a prerequisite for the restricted
new implementation. P7H-036 proves a parameterized deterministic CPU image;
P7H-037 leaves complete PDS launch-state coverage UNKNOWN even with an ideal
contents-preserving BO provider. A concrete new GPU provider adapter is not
implemented. Do not confuse the host serializer with that adapter or resume
historical deployment hunting as the primary task. FG-02 remains OPEN.

**Earlier restart addendum, 2026-09-27:** see the
[target-provider discriminator check](pds-target-provider-identity.md) and
[artifact table](pds-target-provider-discriminators.csv). The immediate premise
remains unresolved: no available target deployment artifact selects the legacy
provider, and no concrete replacement provider is selected. Clarification of
that intended provider was requested. The TVZ original-report manifest hash
was reconciled with the current report's later 31-byte language annotation;
the observations are unchanged. No PDS or provider survey was repeated.

1. Locate the full transferred worktree. Read this document, applicable local
   instructions, [Phase 7 provenance](../source-provenance.md), and the
   [hardware gate](../7.0-gate.md). Inspect `git status --short` and
   `git diff --cached --name-only`. Preserve every previous edit/untracked file.
   Do not reset, clean, stage, commit, or regenerate old maps automatically.
2. Verify the two retained ELF hashes using the commands below. Check that the
   latest reports/tables/tools named here were actually transferred. Missing
   untracked files are a handoff-transfer problem, not a reason to restart RE.
3. Read [pds-backing-provider.md](pds-backing-provider.md), then its
   [candidate table](pds-backing-candidates.csv), [path table](pds-backing-paths.csv),
   and [current blockers](frozen-draw-blockers.md). Use
   [pds-stack-pairing.md](pds-stack-pairing.md) for already established premises.
4. **Next inference premise: complete deterministic PDS launch-state coverage.**
   Read C10 and the ideal-provider countermodel in the Route C report. The
   committed56 bytes and CPU-packed12-dword extent are not established hardware
   read bounds. No actual outside read is asserted. Exact reads are unnecessary
   if their complete state superset can be established.
5. Preserve CPU-image versus GPU-state scope: run the new image/table/negative
   checks; do not treat a passed host snapshot comparison as cache or SGX-binding
   proof. The transition table records concrete-backend obligations separately.
6. Route C is chosen; do not ask the user to choose it again. Do not require
   historical recycled-value equivalence or target-provider identity. Do not
   repeat generic ISA, corpus, Experiment #43, PDF or historical provider work.
7. If the restricted deterministic backing contract closes, immediately resume
   FG-02 at [frozen-fragment-link-progress.md](frozen-fragment-link-progress.md),
   then the next genuine frozen-triangle blocker. Do not reopen PDS decoding just
   because its semantics remain unknown. Hardware restrictions still apply.

## Current Route C decision

Historical provider identity remains UNKNOWN/CONDITIONAL; recycled holes remain
potentially stale. The new CPU initializer/serializer explicitly defines all56
bytes, including zero holes, for supplied resolved U. Actual GPU first-use zeros
and preservation remain CONDITIONAL; no-overlap stays CONFIRMED within P7H-030.
PDS/`0x07000345`/`0xaf000000` semantics remain UNKNOWN. The next premise is
launch-state coverage (C10); backend implementation must subsequently satisfy
C1–C9. See [the exact matrix](cleanroom-backing-contract.md).

## Historical checkpoint before Route C

```text
Target backing provider:
    CONDITIONAL

Fresh backing zero:
    CONDITIONAL for target

Contents preserved to first GPU use:
    CONDITIONAL for target

First-use no-overlap:
    CONFIRMED

Nine restricted first-use holes:
    CONDITIONAL

0x07000345 read set:
    UNKNOWN

FG-02:
    OPEN

Gate B:
    BLOCKED

whitelist:
    []

hardware:
    UNVERIFIED
```

**Historical blocker before Route C: `target -> backing-provider identity`.** The retained
package metadata does not bind the frozen target to the audited historical
fresh-zero/preserved-contents provider contract. The source contract itself and
the selected first-use non-overlap have been established within their scopes.
No selected-path non-zero provider counterexample was found. No unqualified
implementation-blocker closure, functioning driver, or rendered triangle exists.
This historical question is preserved but is no longer the project's next task.

## Project, phase, and target

[SGX535-reMESA](../../../README.md) is an independent reverse-engineering,
documentation and hardware-preservation project for PowerVR SGX535, initially
Intel Poulsbo / US15W-family GMA 500. Its long-term goal is a modern open-source
Linux Mesa/Gallium 3D driver. It is currently research/early bring-up, not a
functional Mesa driver.

**Current phase: Phase 7 — PSB DRI reverse engineering / frozen-triangle
bring-up.** The static work follows retained historical userspace through
compiler, linker, PDS/USE generation, scene records, BO/relocation/fence handling
and the candidate historical kernel/XHW protocol. A minimal implementation-grade
3D specification and a Phase 8 implementation plan are not yet available;
[the static decision](frozen-draw-static-decision.md) remains Result B.

Three targets must not be conflated:

| Context | Established identity / limitation |
| --- | --- |
| Physical machine | TVZ-001 operator report: Dell Inspiron 1210, board Dell 0X605H, BIOS A02; PCI 0000:00:02.0, 8086:8108, revision 0x06, subsystem 1028:02b1 |
| Reported modern environment | antiX `5.10.240-antix.1-486-smp`, i686, driver `gma500`, DRM card0; original complete terminal output and installed-module digests absent |
| Historical research stack | retained `home:lkundrak:poulsbo/xpsb-glx` 0.18-4.1, i386 `psb_dri.so` and `Xpsb.so`, embedded `5.0.1.0046`; candidate public libdrm-poulsbo 2.3.0 and legacy PSB kernel family |

See [TVZ report](../../hardware-evidence/TVZ-001/operator-report.txt) and
[antiX source audit](../../kernel-5.10.240-antix-audit.md). Historical DDK
configuration SGX535 rev121, PCI revision 0x06, and compiler target `(3,111)`
are **not interchangeable** and do not identify the physical core revision.
The compiler pair is a software configuration/table selection.

The modern gma500 source associated with the target report has GEM/MODESET and
an empty private ioctl table. It is not the retained legacy PDS BO/CMDBUF/XHW
provider merely because it drives display on the same hardware. Its own backing
behavior cannot be substituted for legacy SGX placement/visibility without a
separate contract. The installed-module/source correspondence remains unverified.

## Evidence discipline, provenance, and safety

Use [docs/evidence-matrix.csv](../../evidence-matrix.csv) and exact source IDs,
hashes, paths, ELF VAs, line ranges and assumptions. Existing IDs are not to be
renumbered. Keep older claims as history with explicit corrections/supersession.

- **CONFIRMED**: directly supported within a stated scope. A confirmed CPU store
  or source allocator does not confirm GPU interpretation or target deployment.
- **INFERRED**: supported correlation/model with a missing proof edge.
- **UNKNOWN**: no implementation-grade evidence. Do not invent a zero, opcode,
  reserved-bit rule, provider membership, or hardware result to make a model fit.
- **CONDITIONAL** in these reports: the conclusion follows under explicitly
  named premises; it is not an unconditional promotion of the target claim.

Always separate:

1. **Historical CPU behavior:** which bytes the retained software produces and
   what a particular historical allocator source does.
2. **Architectural semantics:** what SGX535 instructions read/do. These can stay
   UNKNOWN even when every relevant software byte is reproducible.
3. **Clean-room construction:** what a new implementation may construct under
   a proven allocation/layout/visibility contract. Physically writable memory
   alone does not prove an arbitrary fill preserves intended program behavior.

Provenance boundaries:

- Retained xpsb-glx spec: `Redistributable, no modification permitted`. ELFs are
  static evidence only. Never execute, load with `dlopen`, or link them into a
  new implementation. Decompiled proprietary bodies/Ghidra databases stay
  outside the repository; derived facts and address-level maps are retained.
- Public PSB/libdrm/Mesa sources have per-file licenses. Inspect notices; a
  source RPM's label or public accessibility is not blanket permission for all
  files. Candidate Mesa 7.4.4 is a comparator, not authenticated source of every
  retained DRI layout/default.
- Older Canonical `libgl1-mesa-dri-psb` 0.24+repack+0038.1-0netbook1 has an
  **Intel Confidential** original-content notice. Its prior occurrence and
  package-provenance results are recorded, but it is quarantined and **must not
  be used as implementation evidence**. Do not reacquire it to route around this.
- The rejected `GrapheneCt/PVR_PSP2` candidate at
  `ae4df4b723f3f1aa629b1f3b276b9cca6d663891`, including
  `include/gpu_es4/eurasia/hwdefs/sgxpdsdefs.h`, is not an encoding oracle.
  Its restrictive notice and PSP2/later-core target disqualify that use here.
- Do not transfer Rogue, Series5XT, TI/OMAP or EMGD semantics to Poulsbo based
  only on similar names. Consult [provenance](../source-provenance.md) and
  [external-evidence report](series5-pds-external-evidence.md) before using leads.
- New uncertain acquisitions belong in a disposable quarantine pending notice
  and relevance review. Do not casually modify `references/`.

**Safety:** Gate B BLOCKED; whitelist `[]`; hardware functionality UNVERIFIED.
This track is static-only. No GPU submissions, active SGX/DRM ioctls, MMIO reads
or writes, reset/power/clock changes, instruction brute force, historical binary
execution, or module loading. Even CORE_ID/CORE_REVISION reads are not authorized
by the current [7.0 gate](../7.0-gate.md). Provider or ISA progress does not
independently satisfy the gate. Passive TVZ observations are inherited evidence,
not observations made during this work. Reading files and compiling authorial
constant probes without executing historical code are allowed static activities.

## Authoritative retained identities

Relative to the repository root:

| Artifact | SHA-256 |
| --- | --- |
| `references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/dri/psb_dri.so` | `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8` |
| `references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/drivers/Xpsb.so` | `da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f` |
| `references/home:lkundrak:poulsbo/xpsb-glx/xpsb-glx_0.18.orig.tar.gz` | `da180da15c38bb3dec5d70adf263c9953d63e4f88a01726f0fc867fdb7e9fab7` |
| `references/home:lkundrak:poulsbo/xpsb-glx/xpsb-glx.spec` | `60062e6850f6780e0c9a5d458b380f188188ed3ea2b58a0f5651dcd41b64c895` |

Archive MD5 `9d14c7e16a72db1923c1a7801cf347c7` matches the public OBS
inventory. The spec SHA matches OBS **xpsb-glx revision 3**, not revisions 1/2.
The ELF hashes above were reverified at the last audit and again for this handoff.

Address conventions matter: report addresses are generally **ELF virtual
addresses**, not file offsets. The disposable Ghidra import adds `0x10000`;
repository exporters derive/check that delta. In the P7G context-derived
convention, `C+offset` means raw context displacement `0x1345c+offset`; scene,
program, BO and renderer offsets are relative to their own objects. Do not apply
the context bias to every `+offset` in a document.

## Frozen triangle and Phase 7 progression

[Frozen draw](frozen-draw-closure.md): one indexed triangle, indices `0,1,2`,
new context, one scene, 32×32 RGBA color target; software-prepared position and
primary color, smooth shading, full color mask, white at all vertices. Intended
post-viewport positions: `(8,8,0.5,1)`, `(24,8,0.5,1)`, `(8,24,0.5,1)`.
Textures, fog, lighting, blend/logic/dither, alpha/depth/stencil tests, culling,
user clipping, polygon offset, queries, scissor, multisampling and hardware
vertex transformation are excluded. No prior user GL draw or clear is assumed.
Internal scene clear paths still exist and must not be excluded by that wording.
These are chosen research inputs, not measured rendering behavior.

P7B–P7F established retained DRI/Xpsb entry paths, builders, submission interfaces
and bounded candidate formats. P7G follows the single frozen draw and corrects
earlier interpretations. P7H narrows compiler/link/PDS and now provider behavior.
Read later P7H reports for the current conclusion: old checkpoint paragraphs
still saying the compiler result, terminal padding, or first-use order is unknown
are historical, not reasons to redo them.

Already closed facts beyond the PDS work:

- Historical DrawElements → VBO/TNL → private renderer callback chain is
  established (P7G-010). It is not a current blocker.
- The software vertex path `0x29dd9 -> 0x40355` is selected under historical
  `INTEL_NO_HWTNL`; this is static control-flow evidence, not an executed setting.
- For selected width eight, vertex records are 32 bytes (position/color), three
  vertices total 96 bytes; fresh indices are `00 00 01 00 02 00`.
- `0x29fbd`'s separate setup data are eight four-word positions, **not** four
  eight-word vertices (P7G-001). Do not reuse that setup as the frozen input.
- `0x2a39a` reserves eight bytes but commits only the four-byte `0xc0000000`
  terminal word (P7G-002). The second reserved word is not a missing output.
- A 40-byte host fence versus a 48-byte wire fence is not itself an ABI conflict
  (P7G-012). Dynamic GPU addresses are relocation relationships, not missing
  fixed constants.

### FG-01: CLOSED, compiler-stage output only

[Exact compiler proof](frozen-fragment-exact-output.md), P7H-010/011:
no-texture `MOV primary color -> output color; END` converts to UniFlex
opcodes `0x48` and `0x35`. Four scalar virtual-temp writes are removed by DCE;
no selected insertion later recreates them. The main and secondary compiler
USSE streams both have **zero instructions and empty bytes**. No physical
instruction register assignments or instruction allocations occur.

Key returned fields: `+0xa8=0`, `+0xac=0`, `+0xb0=0xffffffff`, `+0xb4=0`,
`+0xb8=2`, `+0xbc=0`, `+0xc0=0`, `+0x1bc=0`, `+0x1c0=0`, `+0x1c4=0`,
`+0x1c8=1`, `+0x1cc=1`, `+0x1d0=0`, `+0x1d4=0`, `+0x1d8=0`, `+0x1dc=0`.
These are compiler-result offsets. The proof does not say a complete linked
pixel program is empty or that hardware rendering is correct.

### FG-02: OPEN, link suffix and secondary emission established

[Link progress](frozen-fragment-link-progress.md), P7H-012–014:

- `0x370d8` creates a zeroed 0x88-byte link record. Selected suffix fields
  `+0x44/+0x48` are `0x00000000,0xf8000140`.
- `0x283d8 -> 0x264b7` copies no compiler instructions, copies that suffix,
  and ORs `0x00040000` into its last word. Linked output is eight bytes:
  **`00 00 00 00 40 01 04 f8`**, words `0,0xf8040140`. This is link-time output,
  not compiler output. Its complete ISA semantics are not established.
- The separate 0x1c-byte scene-program descriptor is explicitly zeroed. Its
  `+0x0c` references the allocated linked-program BO. `0x25970` receives this
  descriptor, **not** the link record; confusing these yields a false null-BO
  interpretation in the relocation helper.
- `0x39ea0 -> 0x3faa3` on the zero-input secondary path allocates/commits four
  bytes containing **`0xaf000000`**, then clears output descriptor
  `+0x0c/+0x10/+0x14`. CPU emission is confirmed, architecture is not.
- All-zero first 16 link-key bytes still rely on a candidate Mesa initialization
  correspondence for key bit 0. The suffix result needs bits 2/3 clear and does
  **not** require assuming the whole key zero. Preserve this distinction.
- Remaining 0x128-byte primary key (`0x2d015`), 0x150-byte pixel key
  (`0x2cca3 -> 0x28559`), target/program references and complete pixel-state
  words are not yet fully specified. Link suffix closure is not FG-02 closure.

### Selected primary PDS: exact 56-byte software layout

Builder `0x25970`, selected zero descriptor/temp-count branch, reserves 400 bytes
aligned to 32 but commits `0x38` = **56 bytes**. The first 12 dwords are the CPU
data-prefix extent. CPU layout labels: DS0[0..7] at +0x00…+0x1c; DS1[0..3] at
+0x20…+0x2c. These labels do not decode hardware source selectors.

| Offset | Selected CPU production / provenance |
| --- | --- |
| +0x00 | USE-address-bearing dword; three masked relocations via `0x3a82c -> 0x3773b/0x37621`; initial placeholder `0x67676767` can be patched/presumed-address encoded; final value dynamic |
| +0x04 | `0x00000000` |
| +0x08 | producer-unwritten |
| +0x0c | producer-unwritten |
| +0x10 | producer-unwritten |
| +0x14 | producer-unwritten |
| +0x18 | producer-unwritten |
| +0x1c | producer-unwritten |
| +0x20 | `0x00000020` |
| +0x24 | producer-unwritten |
| +0x28 | producer-unwritten |
| +0x2c | producer-unwritten |
| +0x30 | `0x07000345` |
| +0x34 | `0xaf000000` |

All nine holes lack a selected producer/relocation write. Do not fill them in a
historical byte image with guessed zeros. The [per-dword table](pds-buffer-byte-provenance.csv)
separates historical/recycled provenance, candidate first use, and target status.

## PDS semantic boundary — accepted, not the next task

### `0x07000345`

Favored **INFERRED** read set: `{+0x00,+0x04,+0x20}`. Exact identities as hardware
operands and, critically, **completeness** remain UNKNOWN. No hole is proved dead.

`0x07000345 XOR 0x07042345 = 0x00042000`, bits 13 and 18. CPU builders move the
initialized triplet from +00/+04/+20 to +08/+0c/+28. Other constructed classes
shift CPU indices into these positions. Correlation does not fix source count.

The old two-index model failed on `0x070b0345`. The later arithmetic decomposition
fits all three literals:

```text
word = 0x07000345 | (A << 18) | (B << 13) | (C << 16)
07000345: A=0 B=0 C=0; associated CPU writes +00/+04/+20
07042345: A=1 B=1 C=0; associated CPU writes +08/+0c/+28
070b0345: A=2 B=0 C=3; associated CPU writes +10/+14/+20
```

This is not an ISA decoder. All three are retained literals; there is no common
CPU-controlled `0x45` encoder independently varying A/B/C. Bits 16–17 are residual
`0x00030000`, not a proven bank/destination/mode/source field.

Competing models from [pds-hypotheses.md](pds-hypotheses.md):

- M1: first instruction reads only the inferred triplet; terminal has no DS read.
- M2: same triplet plus a fixed fourth producer-unwritten source, e.g. +0x08.
- M3: first instruction follows M1, but the terminal reads implicit/temp/DS state.
- M4: A/B/C are an incomplete or differently interpreted addressing/control model.

Every recovered CPU emission is compatible with M1/M2. No observed degree of
freedom varies the candidate hidden source/read count independently. CPU stores
do not observe hardware reads. A fixed field or implicit read can survive every
word-construction correlation. More correlations from these same rows cannot
promote completeness.

### `0xaf000000`

There are 11 verified DRI and four Xpsb direct memory-immediate stores. Inspected
builders put it at a **subprogram end**, and `0x3faa3` emits it alone with zero
local data dwords. The mixed event builder has it at +0x50 and +0x90 inside one
larger BO; it is not necessarily the end of a whole BO. No verified `0xad`/`0xae`
direct-immediate neighbor was found in the bounded check.

These are CPU grammar facts. Opcode name, no-source behavior, zero-valued explicit
selectors, implicit state reads and behavior in the selected primary remain
architecturally UNKNOWN. Standalone emission makes some interpretations less
plausible; it does not prove zero reads.

### Experiment #43 — completed negative investigation

[Report](pds-experiment43.md), [store table](pds-experiment43-retained-stores.csv),
P7H-023/024. It sought an independent natural experiment outside the 42-row
corpus that could separate M1/M2, with priorities bits 16–17 and terminal variants.

- Verified retained direct stores: 23 total. DRI selected word ×3, shifted word
  ×3, event `0x070b0345` ×1, terminal ×11; Xpsb selected word ×1, terminal ×4.
- Event builder `0x392ab` stores **whole literal `0x070b0345` at ELF 0x39719**,
  into +0x8c, followed by terminal at ELF 0x39723 into +0x90. Callers do not
  independently control bits 16–17 at that store. Result: literal/invariant,
  not a recovered semantic CPU variable.
- The broader immediate inventory and 28 direct outbuf callers were screened.
  24 already had bounded exports; `0x2ee34`, `0x38b5c`, `0x38d87`, `0x391e0`
  were checked with focused disassembly. No new verified distinguishing family.
- Raw DRI terminal-byte hits (21) are not all instructions: only 11 are verified
  stores. Do not count unaligned metadata/table bytes as new emitters.
- Older Canonical lpia package `0.24+repack+0038.1-0netbook1` supplied the same
  three `0x45` values and terminal. It is quarantined due to its Confidential
  notice and does not supply implementation facts. Package SHA:
  `14e870f4273e84759630499efeda4948116c29f3ba7d08e2b5f028a5da3ef8a2`;
  ELF SHA `427627909b0c528d154d1b247688f16536162a520d43d472e8a4f404c02e2a6e`.
- A referenced older Fedora xpsb-glx 0.11 link was unavailable; no attributable
  alternate recovered. Absence of retrieval is not evidence about its contents.

Conclusion is bounded: **no observed independent variation separates the models**.
It is not a mathematical census of every hypothetical runtime-composed word.
Do not repeat Experiment #43 without a genuinely new, provenance-suitable emitter
or contradictory evidence. It is not the present next objective.

## Buffer lifecycle: what can occupy the holes

[Lifecycle report](pds-buffer-lifecycle.md), [call graph](pds-allocation-callgraph.csv),
[reuse predecessors](pds-reuse-predecessors.csv), P7H-025–028:

```text
new context 0x4bd92 -> 0x371f0 -> 0x48592
  context+0x804 pool 0x4297b
    one kernel BO, 30 fixed slots; separate host descriptors
  owner 0x27a1e -> owner+0x6c outbuf 0x38a9b
    driBOData 0x44c90 -> pool allocate 0x42f93
    map 0x430c3 -> mapped BO base + fixed slot offset
    reserve 0x38762 -> align32(cursor), 400-byte reservation
    selected builder 0x25970
    commit 0x384ab -> record56, advance cursor56
  unmap/release -> free list or fence-pending list -> same slot reused
```

Each slot is 0x20000 or 0x40000; backing BO size is 30× that = 0x3c0000 or
0x780000. Outbuf backing alignment is 0x1000; the kernel BO creation's page
alignment argument is separately zero. The 0x54-byte host outbuf and 0x3c-byte
host wrapper are calloc-like; this does not clear GPU payload. Metadata/list
links/fences live outside payload.

Release `0x42e37` returns an unfenced slot immediately or queues it pending its
fence. `0x42c98` reclaims it after fence wait/poll. Map returns the original fixed
offset, with no clear. Reset `0x3865f -> driBOData(NULL)` may retain an existing
slot and reset its cursor without copying/clearing. Rotation increments a host
generation counter, not an in-payload stamp. Synchronization controls reuse time,
not contents. Cache hits reuse the existing program descriptor without filling
holes. Later unmap, relocation and submission do not normalize them.

The same relevant pool/outbuf has established PDS, scene-vertex and index users:
`0x29fbd -> 0x3f94c` copies 128 scene-vertex bytes; `0x37409` copies indices;
ordinary `0x27fa0` also uses the index allocator. This does not mean normal first
draw vertex uploads precede the selected primary in that pool: P7H-030 separately
establishes normal vertex payloads use context+0x810 there. Do not conflate
general reuse reachability with first-use occupancy.

The 56-byte commit records descriptor length and advances a cursor; it is not a
56-byte clear/copy, new kernel BO, or proven hardware access bound. Data extent
12 dwords is separate. In the selected later pixel-state packing, the primary
base is relocated and `(data_dwords & 0xfc) << 24` supplies background
`0x0c000000`; that branch does not consume the 56-byte length as a GPU bound.
No recovered per-hole live mask proves the nine dwords unaddressable.

All 56 bytes are physically CPU-writable within the reservation; allocator
metadata is elsewhere. Nevertheless, **`memset(holes,0)` was not accepted as
equivalent historical behavior**: a recycled slot can contain surviving earlier
data, and an unknown live input could require a value other than zero. Writability
is not semantic permission. No exact predecessor image follows from the frozen
shader alone. “Potentially stale/arbitrary” is not proof every 32-bit value was
actually observed at every hole.

## Fresh-backing alternative and pairing

The alternative does not try to prove holes dead. It tries to reproduce an
established **restricted first-use historical byte image**: new BO/TTM backing
starts zero, no earlier write touches the selected range, and mapping/placement
preserve those zeros. Then a clean-room construction can reproduce that image
without deciding which of its nine holes is read. This does not solve arbitrary
implicit state outside the modeled allocation, steady-state reuse, or the whole
triangle automatically.

[Stack pairing](pds-stack-pairing.md), P7H-029–031:

- Candidate libdrm 2.3.0 and PSB 4.41.1 generic `drm.h` are byte-identical:
  SHA `e4e0c5dce5558d4aa09f33fa236832e354275ccc1612a7c4a1a906758bb51f41`.
- [ABI table](pds-abi-pairing.csv): 143 rows = 115 i386 wire-layout metrics,
  16 ioctl values, 12 binary/path observations. The checker compiles a temporary
  freestanding i386 constants object and reads `.rodata`; it never executes it.
  All 115 checks passed. A deliberately incorrect fence size was rejected.
- Retained BO call: `drmBOCreate(fd,30*slot_size,0,NULL,0x20000001,4,&bo)`.
  Map mode is 3 (read/write). Null `buffer_start` selects type_dc, not imported
  user pages. PDS is MEM_PRIV2 / memory type5 / bit29. Hint4 is DONT_FENCE.
- The retained DRM version check accepts major4/minor>=0, ignores patch, and
  does not select an allocator implementation. Matching `5.0.1.0046` is not a
  loaded-module digest. The older libdrm private PSB header differs; generic
  header equality is not a blanket private-ABI or whole-stack compatibility proof.
- Candidate PDS is non-fixed TTM/CMA. Each absent page is allocated through
  `alloc_page(GFP_KERNEL | __GFP_ZERO | GFP_DMA32)`.
- CPU faults and PSB MMU binding reference the same TTM page array. Non-fixed
  PDS/local moves unbind/rebind it. No selected bounce buffer, partial copy or
  payload metadata replaces the contents. Cache policy/transition operations
  are part of the source contract; `psb_invalidate_caches` itself is a no-op and
  must not be cited as the payload flush.

### First-use no-overlap is already CONFIRMED

Scope: successful newly calloc-created context, first successful scene,
zero-input fragment cache miss, no prior draw/clear/feedback, no depth attachment,
no scene/allocation retry, and the traced private path. New context requests a
new per-context pool/BO, not a reused old context pool. This is a userspace path
proof; it does not authenticate the target's kernel supplier.

[Timeline](pds-first-use-timeline.csv), 18 total rows across two branches:

| Event | color+0x24 != 1 | color+0x24 == 1 |
| --- | --- | --- |
| Event data | [0x00,0x10) | [0x00,0x10) |
| Event PDS | [0x20,0xb4) | [0x20,0xb4) |
| Internal clear primary | absent | [0xc0,0xf8) |
| **Nested clear secondary** | absent | [0x100,0x104) |
| Texture-replace primary | [0xc0,0x100) | [0x120,0x160) |
| Texture secondary | [0x100,0x104) | [0x160,0x164) |
| Preterminate state data | [0x104,0x10c) | [0x164,0x16c) |
| Preterminate state PDS | [0x120,0x15c) | [0x180,0x1bc) |
| **Selected primary** | **[0x160,0x198)** | **[0x1c0,0x1f8)** |
| Selected secondary | [0x1a0,0x1a4) | [0x200,0x204) |

Offsets are slot-relative, not absolute GPU addresses. The nested clear secondary
is essential. Both slot capacities fit without rotation; largest reservation end
through primary is 0x350. Reservations may overlap future ranges but do not
write them. Actual predecessor stores/relocations do not overlap selected holes.
The primary is allocation7 or9, not allocation1. The arithmetic model tests
both branches/capacities, all nine holes in each, and insufficient-capacity
rejection. Do not redo this proof absent a new contradictory provider/layout fact.

## Completed backing-provider audit

[pds-backing-provider.md](pds-backing-provider.md), P7H-032–035 is the latest historical provider
research result. Its [acquisition JSON](pds-backing-acquisition.json) has 117
attempt records; [file comparisons](pds-backing-file-comparison.csv) have 84
observations; [candidates](pds-backing-candidates.csv) have 12 rows; [paths](pds-backing-paths.csv)
have 10. These are audit scopes, not the count of every possible provider.

Checked sources:

1. Existing RPM Fusion PSB 4.41.1 candidate.
2. **All seven OBS PSB source revisions**, same original tar, nine distinct
   patch contents inspected. Retained userspace spec exactly matches OBS rev3;
   kernel tar SHA equals the candidate. OBS psb rev7 source ID:
   `f9d9498beee28ac72ca14a4b562e2942`.
3. Pinned earlier `gregkh/psb-kmp` 4.40 snapshot at
   `98b5307e5158a9ac401b29128ddd1184ae06b4d7`; initial import
   `0c4490c22402ba9fdc31a763c02e1948ec10f47d` TTM allocator also matches.
4. **Seven recovered Ubuntu source archives**: 4.37 original, 4.41.1 original,
   4.41.6 original; 4.42.0-0ubuntu2~1004um2+karmic, ~1004um3.1, ~1004um3.2,
   ~1010um5. Launchpad source publication/file APIs recovered files despite
   directory 403s. Empty file lists for other publications are recorded limits.
5. Historical Linux v2.6.32 page allocator to establish explicit zero-flag
   behavior, not a presumed modern userspace/security guarantee.

Important source/package digests:

| Input | SHA-256 |
| --- | --- |
| PSB 4.41.1 original (OBS/RPM/Ubuntu identical) | `12fe32e61b313882bd72a1eeda87afceb5b5b326761392edddf294b404efb5cd` |
| libdrm-poulsbo 2.3.0 original | `482fa4e4904c7ec5a926627b587288f4c5c16191681e02ad44ee646af0c97c8c` |
| PSB 4.37 original | `acec3193bdad233a87bbefd9c6a01873abe1fbf1c3a701a399072eb74f8807ab` |
| PSB 4.41.6 original | `daf02e159dab223ca14545a4e7af7e91f09f65272a5dbd34891b9a50af2bdd45` |
| 4.42 ~1004um2+karmic | `5d0750fdef9e272b699a325e40dc0cecbd6cfcb91b841656453025c4ce8ba6a4` |
| 4.42 ~1004um3.1 | `37d5c6718ed6585de8c73fc1906820e4647ff9e0c6c2240ce224a176730ebb0b` |
| 4.42 ~1004um3.2 | `70cef3de5d7f38c65ca5fc09aeaedc585bd63e2bc5aaa033bf98e0706548101b` |
| 4.42 ~1010um5 | `56f04bec8bd8625a13acb6e456dc403adfc7dcb16d0d5e8b5108ba6ba48609bc` |

Use machine-readable acquisition records for automated checks.

The selected backing code matches across the later checked sources, including
review of Ubuntu `10_change_prefix.dpatch`: it changes symbols, not allocation
semantics. Raw text patches were applied without running dpatch/package scripts.
One pre-applied I2C hunk was rejected in each 4.42 disposable run; this was
reported, not hidden as a successful package build. Other patches have display,
AGP compatibility, module naming, credential, IRQ/locking or ACPI scope. This
does not certify whole-driver correctness or all private ABI fields.

4.37 explicitly zero-allocates too, but omits an older `flush_agp_mappings` call
that later code retains below Linux2.6.25. Full old-kernel visibility equivalence
is conditional there; do not promote every surveyed configuration indiscriminately.

**Selected placement proof:** mask0x20000001 intersects only memory type5. Both
preferred and busy-priority loops apply the same memory-bit filter. VRAM/stolen,
generic TT/GATT and APER are not fallback choices merely because they occur in
priority arrays. PDS is TTM-backed but is **not** generic memory type TT. PDS/local
eviction preserves the TTM and original requested mask; later PDS revalidation
retains contents. Memory pressure can fail/retry, not silently choose stolen.

Linux's physical page reuse does not defeat this contract: `__GFP_ZERO` causes
`prep_new_page -> prep_zero_page -> clear_highpage` even when a frame was used
previously. Do not confuse a fresh userspace range, new userspace BO, new kernel
BO/TTM, and newly populated pages. Recycled slots in an existing BO are not zeroed
again. Plain `GFP_KERNEL` allocations elsewhere are not this selected payload.

Validation at the completed audit: **195 acquisition/derived-file hash checks**,
**30 Debian source checksum/size checks**, both retained ELF hashes, prior
source/package hashes, CSV schemas/unique IDs, 456 Phase7 local links, script
parse/compile, positive and negative regressions, `git diff --check`. **Debian
signatures were not authenticated.** Download equality is not a trusted installed
module identity. `references/` unchanged, index empty, no commit.

### Why still CONDITIONAL

The audit proves a finite source family **S** has the required selected contract.
It does not prove that all viable target implementations **K** equal S, nor that
the supplying target provider belongs to S. Matching wire ABI does not fix a
private allocator implementation. Same project/year or unversioned dependencies
do not establish co-installation, boot selection or binary/source correspondence.
The DDX spec's `Provides: psb-kmp` / `psb-kmod-common` is not a kernel build pin.
DKMS can build different modules against different kernels.

No selected-path non-zero counterexample was found, and no fictitious one was
invented. That absence does not fill the target-membership edge. Existing target
evidence names a different modern display interface; it does not select a legacy
BO implementation. The remaining missing fact is an authenticated/traceable
provider binding or a concrete equivalent replacement contract—not another PDS
operand mask, first-use offset or conveniently zeroing source tree.

## Historical routes and discriminators (Route C is now selected)

### A. Bind historical target to audited provider family

Find an attributable target/deployment artifact that selects the legacy provider
and links its build to audited source/patches. Exact package release is not
inherently required if the used path's semantic equivalence is proved. Do not
interpret another matching source tar as evidence that it was loaded.

### B. Prove a real equivalence class

Define the viable providers from **target evidence**, not from only downloaded
packages. Prove fresh-zero and content preservation for every member, including
patch/configuration alternatives. Resolve the 4.37 pre2.6.25 caveat if that
configuration remains viable. Do not claim an exhaustive K by silently discarding
unidentified variants or failed downloads. The already audited S need not be
revisited unless target evidence selects a new configuration or contradicts it.

### C. Establish a clean-room replacement provider contract

A new explicit provider could guarantee fresh-zero payload and preservation
independently of historical installed identity. This route requires concrete
allocation, mapping, placement/SGX address binding, lifetime/synchronization and
failure semantics, with valid writable bounds and no hidden payload metadata.
It must support the selected allocation/layout and preserve initialized values
and relocations, not just promise `memset`. Distinguish the chosen new contract
from a claim that the old driver used it. Modern gma500 GEM alone does not supply
the missing SGX interface. Do not implement hardware access or weaken Gate B to
make this route appear complete.

Highest-value discriminators **not yet exhausted/established**:

| Discriminator | What it would settle / limitation |
| --- | --- |
| Offline rootfs/package database or installation manifest for the intended legacy target | Which PSB module, DDX and libdrm packages were selected, rather than available together |
| Actual supplying module artifact with SHA/build ID/vermagic and attributable source/build/patch configuration | Provider implementation identity; vermagic/name alone are insufficient |
| DKMS build record/source checksum and boot/initramfs/module-selection records | Which implementation was built and selected; a source package merely installed is weaker |
| Actual linked `libdrm.so.2`, DDX artifact, dependency/loader path and package ownership | Resolve library/provider handoff and remaining exact-built-stack ambiguity, not merely SONAME |
| A provider-specific feature/allocator discriminator in a newly attributable artifact | Could restrict K without requiring exact release identity; the already checked DRM major/minor test cannot do so |
| Explicit intended clean-room target provider plus reviewable allocation/SGX-mapping contract | Select route C if no historical deployment binding exists; not evidence of old installation |
| Original TVZ evidence/installed binary identity if supplied offline | Improve modern target-source attribution; does not automatically turn gma500 into the legacy provider |

These are requests for specific evidence, not an instruction to run target
commands. No new probe/ioctl/module load is authorized. Start with files already
available; if one artifact is missing, name it precisely rather than ask broadly
for “documentation” or resume an ISA search.

## SUCCESS CONDITION

Current Route C closure additionally requires the complete launch-state coverage
and concrete backend refinement described in
[cleanroom-backing-contract.md](cleanroom-backing-contract.md). CPU-zero holes
alone do not discharge those obligations. Historical deployment binding is not
a prerequisite for the chosen route.

If target-provider membership or an equivalent clean-room deterministic backing
contract reaches implementation-grade confidence, record separately:

```text
PDS semantics: UNKNOWN
0x07000345 read set: UNKNOWN
historical recycled holes: STALE / potentially arbitrary
restricted first-use holes: ZERO / CONFIRMED
exact read-set dependence: REMOVED as an implementation blocker
```

Close only the restricted nine-hole implementation dependency. Do not say the
instructions were decoded or prove reads outside the modeled extent. **Immediately
resume FG-02**: combine the known suffix, secondary record and deterministic
primary image with remaining primary/pixel keys, render-target-dependent words,
task/control state, scene/background records, BO dependencies and relocations.

Next genuine blockers remain in [frozen-draw-blockers.md](frozen-draw-blockers.md):
FG-03 also includes **six vertex-PDS holes** at +08/+0c/+18/+1c/+28/+2c; do not
automatically transfer this fragment-specific timeline proof to them. FG-04 is
complete scene/target/background/raster state (including the 52-word list);
FG-05 is full object/relocation/fence inventory; FG-06 is XHW/bootstrap lifetime
and selected protocol; FG-07 is complete used built-stack compatibility. Known
32×32 XHW scene-info simplifications do not close the entire bootstrap. No
functional triangle may be claimed until actually established under the gates.

## IF THE PROOF FAILS

For the current Route C task, use P7H-037: the next inference premise is complete
deterministic PDS launch-state coverage, even assuming an ideal BO provider.
Do not fall back to the historical discriminator below; it is retained only
as the pre-Route-C checkpoint.

Reduce the remaining ambiguity to **one precise discriminator**. Example:
“The existing target record has no artifact identifying the implementation that
services BOCreate(mask0x20000001,buffer_start0); the public source family is proven,
but no module/build binding selects it.” Or identify a specific alternate
placement/copy/configuration selected by new evidence. State which artifact or
contract clause separates the alternatives. Do not start another broad provider
survey, manufacture a non-zero implementation, or return to generic ISA research.

## DO NOT DO THIS

- Do not guess generic PDS opcodes, source count or terminal semantics.
- Do not redo the 42-row extraction/semantic/XOR analysis to increase confidence
  using the same emissions; correlation still cannot prove completeness.
- Do not redo Experiment #43, including bits16–17 of the whole event literal,
  without a genuinely new independent producer or contradiction.
- Do not generically search for
  `Eurasia.3D Input Parameter Format.1.3.37a.SGX535 1.2.External.pdf` or make it
  a prerequisite. Its unavailability is accepted for the current route.
- Do not assume producer-unwritten means zero, or CPU-writable means a chosen
  fill is semantically equivalent.
- Do not confuse fresh suballocation with new zero-populated kernel backing.
- Do not treat modern gma500 as the legacy provider, or project/package
  co-location, dates, matching version strings or ABI equality as deployment.
- Do not use quarantined Confidential material or rejected PSP2 definitions as
  implementation evidence; do not silently import later-core semantics.
- Do not redo the full ABI pairing/115 layouts, first-use overlap analysis,
  nested clear-secondary discovery, seven OBS revisions or recovered Ubuntu
  source survey as a new research task. A one-time integrity regression is fine;
  rederivation requires new relevant evidence.
- Do not reopen FG-01, the historical frontend edge, corrected record widths,
  or reserved-but-uncommitted termination padding because an older report says
  they were unknown. Read the later corrections.
- Do not reset/clean/stash away prior work, regenerate curated maps blindly,
  stage, commit, or execute historical packages/binaries. No hardware actions.

## Evidence-ID map

The latest IDs are **P7H-036–037**; next new evidence should use an unused ID
after checking the matrix. This handoff adds no new P7H claim.

| IDs | Established scope / important limit |
| --- | --- |
| P7H-001/002 | Frozen UniFlex input and then-current compiler-output boundary; later 010/011 close selected output |
| P7H-003 | Candidate XHW ioctl direction/dispatch concern narrowed; not full stack authentication |
| P7H-004 | Selected scalar lowering |
| P7H-005 | Selected XHW init path |
| P7H-006 | Fragment link record |
| P7H-007/008/009 | XHW revision option, selected 32×32 scene-info result and candidate initial TA flags; scoped protocol facts |
| P7H-010/011 | Empty main/secondary compiler USSE and downstream compiler metadata; FG-01 closed |
| P7H-012 | Exact link-time suffix |
| P7H-013 | Zero-input secondary PDS fast path |
| P7H-014/015 | Selected primary PDS CPU output; historical constant-prefix extent arithmetic, not instruction read set |
| P7H-016/017/018 | Literal reuse in DRI/Xpsb and nonzeroing outbuf behavior |
| P7H-019/020 | CPU index-formula crosschecks; selected pair-read hypothesis remains inferred |
| P7H-021/022 | Event-word arithmetic refinement and terminal CPU grammar, not hardware read count |
| P7H-023 | Experiment43 retained negative result; whole literal at0x39719 |
| P7H-024 | Older package occurrence/provenance only; Confidential quarantine exclusion |
| P7H-025 | 30-slot mapped-pool lifecycle, free/fence recycling, no clear |
| P7H-026 | Individual hole provenance and CPU-writable extent; functional fill equivalence unknown |
| P7H-027 | Forward descriptor/commit/submission handoff and shared data users |
| P7H-028 | Candidate PDS fresh-zero route; historical statement had first-use uncertainty later resolved by030 |
| P7H-029 | Generic ABI identity, 115 metrics, observed retained call agreement; no target provider identity |
| P7H-030 | Restricted successful first-use no-overlap at0x160/0x1c0, both internal-clear branches |
| P7H-031 | Candidate page-array/content identity across CPU/PDS/local mapping |
| **P7H-032** | Exact retained OBS spec/archive lineage; seven OBS source revisions and patch audit; dependencies do not pin deployment |
| **P7H-033** | Bounded historical provider/file/patch equivalence; old4.37 cache caveat and build limits retained |
| **P7H-034** | Selected mask excludes stolen/TT/APER fallback; PDS/local preservation and historical explicit page zeroing |
| **P7H-035** | Target membership remains UNKNOWN; modern source interface distinction; target conclusions conditional |
| **P7H-036** | Route C decision; explicit host initialization and exact parameterized56-byte image; negative regressions; no GPU-provider implementation claim |
| **P7H-037** | Complete deterministic PDS launch-state coverage unproved even under ideal BO preservation; next Route C premise; no actual outside read asserted |

## Reading order and persistent artifact index

Read in this order rather than chronologically replaying the investigation:

1. This file; [current provider decision](pds-backing-provider.md);
   [blocker table](frozen-draw-blockers.md); [gate](../7.0-gate.md).
2. [Provider candidates](pds-backing-candidates.csv), [paths](pds-backing-paths.csv),
   [acquisition log](pds-backing-acquisition.json), [file comparisons](pds-backing-file-comparison.csv),
   [audit tool](../../../tools/psb-dri-re/pds_backing_audit.py).
3. [Stack pairing](pds-stack-pairing.md), [ABI table](pds-abi-pairing.csv),
   [first-use timeline](pds-first-use-timeline.csv),
   [ABI checker](../../../tools/psb-dri-re/pds_abi_layout_check.py),
   [first-use model](../../../tools/psb-dri-re/pds_first_use_model.py).
4. [Lifecycle](pds-buffer-lifecycle.md), [byte provenance](pds-buffer-byte-provenance.csv),
   [allocation graph](pds-allocation-callgraph.csv), [predecessors](pds-reuse-predecessors.csv).
5. [FG-01 exact output](frozen-fragment-exact-output.md),
   [FG-02 current progress](frozen-fragment-link-progress.md),
   [symbolic trace](frozen-fragment-symbolic-trace.md). Earlier
   [compiler checkpoint](frozen-fragment-compiler-checkpoint.md) and
   [link checkpoint](frozen-fragment-link-checkpoint.md) are historical context.
6. [Frozen closure](frozen-draw-closure.md), [objects](frozen-draw-objects.md),
   [formats](frozen-draw-formats.csv), [state atoms](state-atoms.csv),
   [static decision](frozen-draw-static-decision.md), [format readiness](format-readiness.md),
   [unknowns](unknowns.md), [function map](function-map.csv).
7. For boundary comprehension only: [selected hard boundary](selected-pds-hard-boundary.md),
   [hypotheses](pds-hypotheses.md), [Experiment43](pds-experiment43.md),
   [Experiment43 stores](pds-experiment43-retained-stores.csv),
   [comparative encoding](pds-comparative-encoding.md), [search ledger](pds-readset-search-ledger.md),
   [external evidence](series5-pds-external-evidence.md).
8. Existing corpus artifacts, not a request to regenerate:
   [instruction corpus](pds-instruction-corpus.csv), [semantic corpus](pds-semantic-corpus.csv),
   [field constraints](pds-field-constraints.csv), [differentials](pds-differential-matrix.csv),
   [bit influence](pds-bit-influence.csv). Actual tools are
   [pds_field_constraints.py](../../../tools/psb-dri-re/pds_field_constraints.py),
   [pds_semantic_constraints.py](../../../tools/psb-dri-re/pds_semantic_constraints.py),
   [pds_experiment43_survey.py](../../../tools/psb-dri-re/pds_experiment43_survey.py),
   [pds_literal_survey.py](../../../tools/psb-dri-re/pds_literal_survey.py), and
   [pds_byte_sweep.py](../../../tools/psb-dri-re/pds_byte_sweep.py).
   There is **no established minimal ISA decoder**; do not assume a requested
   historical deliverable named `pds-minimal-decoder.py` exists or is authoritative.
9. [Xpsb index](../xpsb-re/README.md), [version pairing](../xpsb-re/version-pairing.md),
   [XHW frozen trace](../xpsb-re/xhw-init-frozen-draw.md),
   [static buffer](../xpsb-re/static-buffer.md), [bootstrap dependencies](../xpsb-re/bootstrap-dependencies.md),
   [Xpsb function map](../xpsb-re/function-map.csv), [source/package provenance](../xpsb-re/source-provenance.md).
10. [Phase7 index](../README.md), [Phase7 source index](../source-index.json),
    [Phase7 provenance](../source-provenance.md), [evidence matrix](../../evidence-matrix.csv),
    [frozen source hashes](frozen-source-checks.json), [public source catalog](../../poulsbo-data/sources.json),
    [antiX manifest](../../phase4-2-data/manifest.json), [TVZ manifest](../../hardware-evidence/TVZ-001/manifest.json).

For targeted future static work, use [tool instructions](../../../tools/psb-dri-re/README.md),
[DRI targets](../../../tools/psb-dri-re/targets.txt),
[Xpsb targets](../../../tools/xpsb-re/targets.txt), `DecompileSelected.java`,
`DecompileXpsb.java`, `ExportPsb.java`, `TraceFrozenDraw.java`, `TraceDrawDispatch.java`,
and retained `docs/phase7/psb-dri-re/analysis/` inventories. `dri_map.py` alone
replaces the curated function map; do not casually run it over this worktree.

## Worktree preservation and ephemeral state

Before this handoff there were 18 modified tracked files and many untracked
reports/CSVs/scripts (including the latest provider work), plus an existing
`tools/psb-dri-re/__pycache__/`. Nothing was staged. This is cumulative prior
work, not disposable output to clean. Preserve it all. The handoff itself is a
new untracked file until the user decides otherwise. No commit is authorized.

`/tmp` is **not persistent** across accounts/machines. The next session must not
assume any of these exist:

| Ephemeral path | Contents | Persistent recovery information |
| --- | --- | --- |
| `/tmp/sgx535-backing-provider` | 117-attempt acquisition cache, original public archives/specs/API responses, source extracts, patched trees, disposable fetch helper and patch logs | Acquisition JSON has every attempted URL and successful hash; comparison CSV has84 derived-file hashes; provider report and recovery commands below explain extraction/patching. Full API response bytes/patch logs/fetch helper are not committed. Mutable API response hashes may no longer be reproducible. |
| `/tmp/sgx535-pairing` | Candidate kernel4.41.1, libdrm release10 source RPMs/tars, firmware inventory | [Xpsb provenance](../xpsb-re/source-provenance.md) has URLs/hashes; kernel original also available from pinned OBS acquisition. Firmware is inventory evidence, not needed for the immediate provider task. |
| `/tmp/sgx535-libdrm-psb` | libdrm2.3.0 extracted from release11, same original as release10 | RPM/tar URLs/hashes in provenance; common header digest above; source-file checks in frozen-source-checks.json |
| `/tmp/sgx535-xorg-psb` | Public DDX0.32.0 comparator source | URL/RPM/tar hashes in Xpsb provenance; not exact retained build authentication |
| `/tmp/sgx535-mesa-7.4.4` | Public Mesa comparator | URL/release hash in Xpsb provenance; conditional layout/default scope remains |
| `/tmp/sgx535-ghidra-re/PsbDriStatic` and other disposable Ghidra projects | Analysis databases, machine-dependent Ghidra state | Retained ELFs, Java exporters, targets and VA-bias rules persist; recreate only focused exports needed for a new question. Do not import databases into the repo. |
| `/tmp/sgx535-psb-decompile` and Xpsb decompile directories | Bounded proprietary decompiler text | Derived findings, source VAs, target lists persist; bodies intentionally not committed. Exact incidental Ghidra analysis state is not preserved. |
| `/tmp/sgx535-exp43-quarantine` | Confidential older package/ELF, occurrence-only experiment cache | Hashes, original URL, restriction and negative outcome in Experiment43 report. Do not restore/use it for implementation reasoning. |
| `/tmp/...` transient constants objects/test CSVs/pycompile cache | Validation scratch | Authorial scripts and exact commands below; safe to recreate outside worktree |

No unrecorded ephemeral result is being offered as proof of target identity.
Important conclusions and source origins/digests are persistent. A hash without
retained bytes is an integrity expectation, not a guarantee a remote archive or
mutable API will remain available. Do not weaken recorded hashes to make a
fresh download pass.

## Reproducible validation commands

Commands below are for a future session, from the repository root. These are
static checks, not permission for hardware actions or broad research reruns.
Python3 is needed; the ABI checker also needs clang capable of emitting an i386
object (no i386 runtime is needed). Source-RPM extraction may need rpm2cpio/cpio.
Do not execute package hooks. Use a disposable directory for downloads/extraction.

### Worktree and retained hashes (always)

```sh
git status --short
git diff --cached --name-only
git status --short -- references
git diff --check
sha256sum \
  references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/dri/psb_dri.so \
  references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/drivers/Xpsb.so
```

Compare with authoritative digests above. The cached diff must be empty and
references must have no unintended changes; do not delete prior work to force
either result. For a check that fails rather than relying on visual comparison:

```sh
python3 - <<'PY'
from pathlib import Path
import hashlib
base = Path('references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx')
checks = {
 'dri/psb_dri.so': '74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8',
 'drivers/Xpsb.so': 'da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f',
}
for name, expected in checks.items():
    assert hashlib.sha256((base/name).read_bytes()).hexdigest() == expected, name
print('retained ELF hashes verified')
PY
```

### Existing arithmetic/audit self-tests (no regeneration)

```sh
python3 tools/psb-dri-re/pds_first_use_model.py
python3 tools/psb-dri-re/pds_backing_audit.py
PYTHONPYCACHEPREFIX=/tmp/sgx535-handoff-pycache python3 -m py_compile \
  tools/psb-dri-re/pds_first_use_model.py \
  tools/psb-dri-re/pds_abi_layout_check.py \
  tools/psb-dri-re/pds_backing_audit.py
```

Do not use the first-use model's `--write` merely to silence a mismatch. The
backing audit checks allowed memory types, an adversarial mask with VRAM added,
and rejection of a changed authorial zero-allocation fixture. The fixture is
not a historical non-zero provider. With recovered cache:

```sh
python3 tools/psb-dri-re/pds_backing_audit.py \
  --artifacts-root /tmp/sgx535-backing-provider
```

Expected:195 acquisition/derived-file hashes;30 `.dsc` size/checksum rows;
signatures not authenticated; target membership still NOT ESTABLISHED.

### Reacquire the recorded provider cache only if validation needs it

This replays existing URLs; it is **not a new provider survey**. A changed mutable
API response must stop hash verification rather than be silently accepted. The
source archives and derived facts are usually the relevant durable inputs.

```sh
python3 - <<'PY'
from pathlib import Path
import hashlib, json, urllib.request
root = Path('/tmp/sgx535-backing-provider')
rows = json.loads(Path('docs/phase7/psb-dri-re/pds-backing-acquisition.json').read_text())
for row in rows:
    if row.get('status') != 200:
        continue  # Recorded failure; do not turn it into a new search.
    path = root / row['name']
    if path.exists():
        data = path.read_bytes()
    else:
        request = urllib.request.Request(row['url'], headers={'User-Agent':'SGX535-static-research'})
        with urllib.request.urlopen(request, timeout=60) as response:
            data = response.read()
    actual = hashlib.sha256(data).hexdigest()
    if actual != row['sha256']:
        raise SystemExit(f'Changed artifact; preserve old expectation: {row["name"]}: {actual}')
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
print('recorded successful acquisitions restored and hashed')
PY
```

Recreate the derived directories expected by the comparison CSV. Requires Python
tarfile's safe `filter='data'` support and `patch`. Do not run dpatch shebangs:

```sh
python3 - <<'PY'
from pathlib import Path
import tarfile, subprocess
root = Path('/tmp/sgx535-backing-provider')
names = ('drm.h','drm_ttm.c','drm_bo.c','drm_bo_move.c','drm_vm.c',
         'psb_buffer.c','psb_mmu.c','psb_drm.h','psb_drv.c','psb_drv.h')
for archive in sorted((root/'launchpad').glob('*.tar.gz')):
    label = archive.name.removesuffix('.tar.gz')
    with tarfile.open(archive) as tf:
        for name in names:
            member = next(m for m in tf.getmembers() if m.name.split('/')[-1] == name)
            out = root/'sources'/label/name
            out.parent.mkdir(parents=True, exist_ok=True)
            out.write_bytes(tf.extractfile(member).read())
        if '4.42' not in archive.name:
            continue
        dest = root/'patched'/label
        if dest.exists():
            raise SystemExit(f'Existing patch tree: inspect/preserve, do not patch twice: {dest}')
        dest.mkdir(parents=True)
        tf.extractall(dest, filter='data')
    src = next(dest.iterdir())
    patches = src/'debian/patches'
    for line in (patches/'00list').read_text().splitlines():
        name = line.strip()
        if not name or name.startswith('#'):
            continue
        diff = (patches/name).read_bytes().split(b'@DPATCH@', 1)[1]
        result = subprocess.run(['patch','-p1','--batch','--forward'], input=diff,
                                cwd=src, capture_output=True)
        if result.returncode:
            text = result.stdout.decode(errors='replace')
            if name != '06_i2c-intelfb.dpatch' or 'previously applied' not in text:
                raise SystemExit(f'Unexpected patch failure: {label}/{name}\n{text}')
            print(f'{label}: recorded already-applied I2C hunk rejection')
print('derived source trees restored; run pds_backing_audit.py to check exact file hashes')
PY
```

The direct gregkh paths are acquired by the manifest. The restored84 compared
files include seven originals, four patched trees and the pinned gregkh subset.
An unexpected patch mismatch is evidence to inspect, not permission to force it.

### Candidate headers and 115 ABI checks

Recover source archives using [Xpsb provenance URLs/hashes](../xpsb-re/source-provenance.md).
The kernel original is already `obs-kernel-original.tar.gz` in the provider
cache and must have the canonical `12fe32...` digest. For the common libdrm
header, one recorded public OBS origin is in the acquisition/spec metadata;
the known RPM Fusion source RPM is a direct recovery route:

```sh
mkdir -p /tmp/sgx535-libdrm-recovery
curl -fL --output /tmp/sgx535-libdrm-recovery/libdrm.src.rpm \
  'https://ftp.gwdg.de/pub/linux/rpmfusion/nonfree/fedora/updates/testing/11/SRPMS/libdrm-poulsbo-2.3.0-11.fc11.src.rpm'
sha256sum /tmp/sgx535-libdrm-recovery/libdrm.src.rpm
```

Expected RPM SHA `d2173d8063ee35f496321bcf814d3abc2bc5541e47ab36aa135c87cdd9bb7ea0`.
Verify before extracting; `rpm2cpio ... | cpio -it` lists members without
installation. Extract in that disposable directory, then extract original tar
`libdrm-poulsbo_2.3.0.orig.tar.gz` (digest above) to `/tmp/sgx535-libdrm-psb`.
No spec/build/install scripts need to run. Exact extraction/check command:

```sh
(cd /tmp/sgx535-libdrm-recovery && rpm2cpio libdrm.src.rpm | cpio -id --quiet)
mkdir -p /tmp/sgx535-libdrm-psb /tmp/sgx535-kernel-recovery
tar -xzf /tmp/sgx535-libdrm-recovery/libdrm-poulsbo_2.3.0.orig.tar.gz \
  -C /tmp/sgx535-libdrm-psb
tar -xzf /tmp/sgx535-backing-provider/obs-kernel-original.tar.gz \
  -C /tmp/sgx535-kernel-recovery
python3 tools/psb-dri-re/pds_abi_layout_check.py \
  /tmp/sgx535-libdrm-psb/libdrm-2.3.0/shared-core/drm.h \
  /tmp/sgx535-kernel-recovery/psb-kernel-source-4.41.1/drm.h
```

Use a fresh disposable extraction directory; inspect archive inventory first.
The checker itself enforces the full common-header SHA. For the existing
negative regression, modify a **temporary table**, never the repository table:

```sh
python3 - <<'PY'
import csv, subprocess, tempfile
from pathlib import Path
table = Path('docs/phase7/psb-dri-re/pds-abi-pairing.csv')
with table.open(newline='') as stream:
    rows = list(csv.DictReader(stream))
row = next(r for r in rows if r['category']=='wire_layout'
           and r['item']=='drm_fence_arg' and r['metric']=='size')
row['libdrm_value'] = str(int(row['libdrm_value']) + 4)
with tempfile.TemporaryDirectory(prefix='pds-abi-negative-') as tmp:
    path = Path(tmp)/'incorrect.csv'
    with path.open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]), lineterminator='\n')
        writer.writeheader(); writer.writerows(rows)
    result = subprocess.run(['python3','tools/psb-dri-re/pds_abi_layout_check.py',
        '/tmp/sgx535-libdrm-psb/libdrm-2.3.0/shared-core/drm.h',
        '/tmp/sgx535-kernel-recovery/psb-kernel-source-4.41.1/drm.h',
        '--table',str(path)], capture_output=True, text=True)
    assert result.returncode != 0 and 'Layout mismatch' in result.stderr, result
print('deliberately incorrect ABI layout rejected')
PY
```

### CSV schemas, evidence definitions, local links, and saved source hashes

```sh
python3 - <<'PY'
from pathlib import Path
import csv, re, urllib.parse, json, hashlib
root = Path('.')
tables = list((root/'docs/phase7').rglob('*.csv')) + [root/'docs/evidence-matrix.csv']
for path in tables:
    with path.open(newline='') as stream:
        rows = list(csv.reader(stream))
    assert rows and len(set(rows[0])) == len(rows[0]), path
    assert all(len(row)==len(rows[0]) for row in rows), path
with (root/'docs/evidence-matrix.csv').open(newline='') as stream:
    rows = list(csv.DictReader(stream))
for n in range(1,38):
    assert sum(bool(re.match(rf'P7H-{n:03d};',r['notes'])) for r in rows)==1, n
for path in (root/'docs/phase7').rglob('*.md'):
    text = re.sub(r'```.*?```','',path.read_text(),flags=re.S)
    for match in re.finditer(r'\]\(([^\n)]+)\)',text):
        dest = match.group(1).split(' "',1)[0].strip('<>')
        if re.match(r'^[a-zA-Z][\w+.-]*:',dest) or dest.startswith('#'):
            continue
        dest = urllib.parse.unquote(dest.split('#')[0])
        if dest:
            assert (path.parent/dest).exists(), (path,dest)
print('CSV widths/headers, P7H defining IDs, local Markdown file links passed')
# Optional: previous source files, only after restoring their recorded paths.
sources = json.loads((root/'docs/phase7/psb-dri-re/frozen-source-checks.json').read_text())
for entry in sources['sources']:
    path = Path(entry['path'])
    if not path.exists():
        print('NOT CHECKED: restore recorded public source first:',path)
        continue
    assert hashlib.sha256(path.read_bytes()).hexdigest()==entry['sha256'], path
PY
git diff --check
git diff --cached --name-only
git status --short -- references
```

The local-link check validates file targets, not remote availability or every
Markdown heading anchor. Missing `/tmp` source files are explicitly NOT CHECKED;
do not call that a full source-hash pass. Use recorded package recovery first.
Existing corpus scripts may write CSVs; they are intentionally not part of the
default restart validation commands.

## Handoff verification and limits

This handoff is derived from repository reports/tables and existing scratch
artifact descriptions, with no new archaeology or attempt to solve provider
identity. The handoff write is the only new project change in this final task.
Its local Markdown file links, retained ELF hashes, `git diff --check`, clean
`references/` status and empty index are checked before returning to the user.
The next agent must preserve the exact conditional decision and advance the
Route C launch-state coverage premise, rather than replay the completed PDS investigation.

Handoff checks completed: 102 local file links resolved; all five embedded Python
command blocks parsed; P7H-001–035 each have one defining matrix row; both retained
ELF hashes matched. `git diff --check` and an explicit whitespace check of this
untracked document passed. `references/` has no Git changes and nothing is staged.
No source archive was downloaded and no new archaeology was performed while
preparing this handoff.
