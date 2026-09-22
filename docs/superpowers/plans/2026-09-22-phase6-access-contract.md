# Phase 6 Access-Contract Reconstruction Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reconstruct the strongest evidence-backed contract for a first SGX535/Poulsbo identification read without performing hardware access.

**Architecture:** Start with the committed Phase 5 gate, separate source metadata from hardware claims, and test every new claim against the 16 formal gate requirements. Research is divided into Intel, Imagination, MeeGo/Moblin, and local-source branches, then consolidated into a sparse clock/power/revision/failure model and an adversarial gate review.

**Tech Stack:** Git history, local source snapshots, public primary-source research, Markdown, CSV, static link and evidence validation.

**Spec:** User request “FASE 6 — SGX535 / POULSBO MISSING ACCESS-CONTRACT RECONSTRUCTION”, 2026-09-22.

## Global Constraints

- Keep Gate B `BLOCKED` unless every required safety condition is supported by applicable evidence.
- Do not perform MMIO, PCI writes, PM/clock/reset changes, firmware loading, driver rebinding, command submission, or Test Vector One execution.
- Do not treat PCI revision `0x06`, a DDK target, or a historical driver configuration as physical SGX revision evidence.
- Do not infer restricted-document contents from metadata or access history.
- Preserve `CONFIRMED`, `INFERRED`, and `UNKNOWN` evidence boundaries.

## Review Focus

- A historical read must not become a hardware read-safety claim.
- A MeeGo/Moblin frequency name must not become an SGX clock claim without a consumer/block definition.
- A current or historical document URL must not establish inaccessible document contents.
- Generic PCI fault behavior must not substitute for Poulsbo-specific failure behavior.
- Gate prose, table states, whitelist, and final report must agree exactly.

### Task 1: Record the Phase 6 baseline and local-source map

**Files:**
- Create: `docs/phase6-baseline.md`
- Create: `.superpowers/sdd/2026-09-22-phase6-access-contract/progress.md`

- [x] Record the committed Phase 5 handoff, target identity, prohibition list, candidate offsets, current whitelist, and 16 gate requirements.
- [x] Locate local `psb_get_core_freq()` paths, clock/reset references, historical sources, and source provenance.
- [x] Verify no active hardware-access tool exists or is invoked.

### Task 2: Research Intel and Imagination document trails

**Files:**
- Create: `docs/phase6-intel-364236.md`
- Create: `docs/phase6-imgtec-documentation-trail.md`
- Update: `docs/evidence-matrix.csv`

- [x] Recheck document `364236` title, references, access history, revision/supersession metadata, and public scope without inferring contents.
- [x] Recheck publicly attributable SGX535/Series5 documentation and BRN traces, recording provenance and restriction boundaries.
- [x] Add only scoped, directly supported findings to the evidence matrix.

### Task 3: Perform MeeGo/Moblin and clock archaeology

**Files:**
- Create: `docs/phase6-meego-moblin-archaeology.md`
- Create: `docs/phase6-clock-map.md`

- [x] Search public MeeGo/Moblin source, package, and archive indexes for Poulsbo/PSB/SGX power, clock, and reset evidence.
- [x] Trace `psb_get_core_freq()` through historical and current local source, its PCI configuration access, and consumers.
- [x] Separate observed frequency decoding from a proven SGX execution-clock contract.

### Task 4: Consolidate access-contract blockers

**Files:**
- Create: `docs/phase6-access-contract.md`
- Create: `docs/phase6-power-reset.md`
- Create: `docs/phase6-revision-map.md`
- Create: `docs/phase6-errata.md`
- Create: `docs/phase6-ownership-concurrency.md`
- Create: `docs/phase6-failure-recovery.md`
- Update: `docs/unknowns.md`

- [x] Separate driver sequencing from hardware requirements.
- [x] Build sparse revision and clock maps and an explicit unknown register with required artifacts for closure.
- [x] Recheck locking, suspend/resume, CPU-failure, and recovery requirements against the exact kernel scope.

### Task 5: Rebuild and attack the Phase 6 gate

**Files:**
- Create: `docs/phase6-gate.md`
- Create: `docs/phase6-test-vector-one-skipped.md` when blocked
- Create: `docs/phase6-final-report.md`

- [x] Evaluate both candidates against all 16 requirements using only `PASS`, `BLOCKED`, `UNKNOWN`, and `NO-GO`.
- [x] Derive the whitelist and Gate B mechanically.
- [x] Perform an adversarial review of every `PASS` and record why every non-pass blocks the read.

### Task 6: Validate and preserve the evidence state

**Files:**
- Update: `docs/evidence-matrix.csv`
- Update: `docs/unknowns.md`

- [x] Validate Markdown links, CSV/JSON syntax, evidence IDs, hashes, gate consistency, phase wording, and reference-tree cleanliness.
- [x] Verify no active hardware operation occurred and TV1 was neither designed as executable code nor executed.
- [x] Commit the completed documentation and report validation output.
