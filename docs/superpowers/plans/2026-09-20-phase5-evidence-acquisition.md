# English Audit and Phase 5 Evidence Acquisition Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Finish a repository-wide English audit, preserve that state, then perform Phase 5 evidence acquisition without active hardware access.

**Architecture:** Part A inventories every tracked documentation file outside `references/`, fixes only project-authored language, and validates links, identifiers, hashes, and conclusions. Part B starts from the audited Phase 4.7 gate, researches public primary sources, records evidence and unresolved restrictions, then reconstructs a 16-requirement Phase 5 gate. Because Phase 4 currently leaves Gate B blocked and the whitelist empty, no active-access code or Test Vector One execution is permitted unless new evidence closes every required field.

**Tech Stack:** Markdown, CSV, Git, `rg`, the repository validation script, public web research, SHA-256 provenance records.

**Spec:** User request dated 2026-09-20 in this conversation.

## Global Constraints

- Complete and validate Part A before beginning Phase 5.
- Exclude `references/` from edits and language cleanup.
- Preserve technical conclusions, classifications, citations, evidence IDs, paths, hashes, quotations, and gates.
- Do not perform MMIO, PCI writes, power/clock changes, reset, MMU/GTT changes, firmware loading, command submission, module unbind/reload, or Test Vector One execution.
- Treat `CONFIRMED`, `INFERRED`, and `UNKNOWN` according to the existing evidence policy.
- Gate B remains `BLOCKED` unless every safety-critical requirement is supported.

## Review Focus

- Mixed-language or malformed prose must not survive outside preserved source material.
- Translation edits must not alter identifiers, links, evidence rows, hashes, or safety conclusions.
- Public references to restricted manuals prove existence or access restrictions, not undocumented contents.
- PCI revision, platform stepping, SGX core revision, build target, and errata selection must remain distinct.
- The Phase 5 gate, whitelist, skipped/authorized TV1 record, and final report must agree exactly.

---

### Task 1: Record the Phase 4 handoff and repository baseline

**Files:**
- Create: `docs/phase5-baseline.md`
- Create: `.superpowers/sdd/2026-09-20-phase5-evidence-acquisition/progress.md`

- [x] Record commit, worktree state, Gate B, whitelist, blockers, permitted operations, and prohibitions from the final Phase 4 files.
- [x] Cross-check the handoff against `phase4-7`, `phase4-6`, `phase4-5`, `test-vector-one-gate`, `phase4-go-no-go`, `unknowns`, and TVZ evidence.

### Task 2: Complete and preserve the English audit

**Files:**
- Modify: tracked documentation outside `references/` as findings require.
- Create: `docs/english-audit.md`

- [x] Inventory tracked Markdown, CSV, text, READMEs, experiments, and source-adjacent documentation.
- [x] Search for Portuguese, mixed-language prose, malformed translation tokens, and inconsistent terminology.
- [x] Fix project-authored prose with minimal semantic edits; record intentionally preserved quotations and snapshots.
- [x] Run a second independent search, validate links/evidence/hashes, inspect the full diff, and commit the completed Part A state.

### Task 3: Acquire public Phase 5 evidence

**Files:**
- Create: `docs/phase5-evidence-acquisition.md`
- Create: `docs/phase5-restricted-document-trail.md`
- Create: `docs/phase5-intel-364236.md`
- Update: `docs/evidence-matrix.csv`

- [x] Research public Imagination SGX TRM references, access restrictions, titles, document numbers, and scope claims.
- [x] Research Intel document 364236 and related SCH/Poulsbo document trails.
- [x] Archive only legally accessible metadata or documents, recording URL, date, provenance, and hashes where appropriate.
- [x] Keep unavailable contents and unsupported semantics `UNKNOWN`.

### Task 4: Reassess revision, lifecycle, power, and recovery

**Files:**
- Create: `docs/phase5-revision-errata.md`
- Create: `docs/phase5-ownership-power-recovery.md`
- Update: `docs/unknowns.md`

- [x] Recheck rev116/rev121 and PCI `0x06` without crossing identity domains.
- [x] Recheck installed-kernel equivalence, ownership, mapping lifetime, locking, PM, IRQ, suspend/resume, clocks, reset, failure behavior, and recovery.
- [x] Record exactly what evidence would close each remaining blocker.

### Task 5: Reconstruct the formal Phase 5 gate

**Files:**
- Create: `docs/phase5-first-observation-gate.md`
- Create: `docs/phase5-test-vector-one-skipped.md` unless the formal gate authorizes design.
- Create conditionally: `experiments/bringup1/TEST-VECTOR-ONE.md` only if Gate B becomes GO and the whitelist is non-empty.

- [x] Evaluate both ID registers against all 16 required fields using only `PASS`, `BLOCKED`, `UNKNOWN`, or `NO-GO`.
- [x] Derive SAFE-CANDIDATE status and whitelist mechanically from the table.
- [x] Create the blocked/skipped record when any safety-critical requirement is not PASS.

### Task 6: Final consistency and validation

**Files:**
- Create: `docs/phase5-final-report.md`
- Update: `docs/phase4-go-no-go.md` only if needed to point to the Phase 5 decision without rewriting Phase 4.

- [x] Audit classifications, gate/prose agreement, whitelist, prohibited-operation invariants, links, evidence IDs, hashes, and reference-tree cleanliness.
- [x] Run the repository validator, `git diff --check`, CSV/JSON parsing, Markdown link checks, Portuguese/malformed-language searches, and status review.
- [x] Confirm TV1 was not executed and no hardware state was modified.
