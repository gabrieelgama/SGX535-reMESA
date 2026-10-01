# Frozen artifact / first-original-removal Gate B evidence

2026-10-01 UTC. Offline only. **Gate B BLOCKED; whitelist `[]`; target contact
NONE.** No new kernel artifact was built. Existing candidates are referenced by
their frozen hashes; they are not copied, modified or deployed here.

Parent report: [lifecycle-first-removal-gate-review.md](../../lifecycle-first-removal-gate-review.md).

- `initial-pwd.txt`, `initial-git-status.txt`, `initial-repository-files.json`:
  task-start state, including every preexisting Git-visible file hash.
- `previous-bundle-preservation.json`: all 254 preceding finalized artifact
  hashes checked; historical evidence is unchanged.
- `verify-artifacts.py`, `artifact-verification.json`, candidate `readelf`,
  `nm`, `modinfo`, `versions` and `objdump` logs: fresh frozen identity,
  ELF/vermagic/Build ID, full undefined-symbol/version coverage, target CRCs
  and preserved-repeat byte equality for both artifacts.
- `candidate-difference-*.diff`, `candidate-*-selected-binary-calls.json`:
  exact source and compiled selected-path difference. Two additional imports
  are the IRQ-release and KMS-poll helpers; fixed SGX service is unchanged.
- `check-lifecycle-paths.py`, `strict-application-and-provenance.json`:
  fresh strict four-patch application and exact preserved-source comparison;
  all fixed inputs/module source/config/generated-state hashes unchanged.
- `source-contracts/`, `additional-source-contracts.json`: five additional
  exact-source PCI/base/DRM-ioctl contracts, completing the selected removal
  chain. Other exact IRQ/DRM/gma500 contracts remain in the unchanged preceding
  bundle. `retained-shared-irq.stdout` copies the historical H0 shared-IRQ
  observation; it is not a new target read.
- `extracted-lifecycle.inc`, `extracted-irq-callback.inc`, `focused.*`:
  actual-source lifetime test inputs and 10 passing tests/13 scenarios.
  Kernel boundary doubles do not execute real IRQ concurrency or hardware.
- `lifecycle-mutation-check.json`, `omit-*.stderr`: both deliberately broken
  temporary safe-removal paths abort as required. Production patches and
  binaries were never mutated. Core dumps disabled for these probes.
- `run-guards.py`, `checks.json`, `guard-summary.json`, `suite.*`, `ubsan-*`,
  `generator.*`, `dry-*`, `complete.*`: 208 tests, zero skips; three fresh
  strict UBSan harnesses; unchanged golden/dry-run results and fail-closed
  four-label refusal. No host executable is published in this bundle.
- `fresh-tool-identities.json`, `build-environment.json`: retained qualification
  role/version assertions verified unchanged; no toolchain reprovisioning.
- `gate-b-predicates.json`: all 31 scoped readiness predicates, with evidence
  limits. PASS for an accepted risk is not an architectural claim.
- `review-result.md`: separate read-only review, no Critical/Important finding.
- `development-notes.md`: two local test/helper preparation errors and their
  corrections; no candidate, production or target state affected.
- `before/`, `scoped-diff-*.txt`, final state/preservation records: exact task
  delta, unrelated/historical-file preservation and whitespace verification.
- `manifest.json`: final bundle SHA-256 index, excluding itself.

No SSH, module/display/VT/PCI change, DRM open, MMIO, ioctl, TA/raster fire,
readback, reset, recovery, staging, commit or push occurred. A separately
authorized read-only first-load/fallback inventory is the next evidence step;
neither a new transition nor an SGX attempt is authorized by this record.
