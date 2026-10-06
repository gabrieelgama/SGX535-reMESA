# Non-SGX first-load cycle03 evidence

Authorization: one fresh first-load/recovery cycle using already-staged pinned
files. No restaging, overwrite, hot transition, fixed ioctl, SGX submission or
retry. [Result](RESULT.md) separates root observations from operator reports.

Only the fresh STOCK preflight connection ran. It passed 53 root guards and
existing creation-receipt checks. `preflight/` contains raw output, command,
source, status and decoded records. `preflight-decision.json` and
`pre-boot-decision.json` record the decisions made before manual selection.

`experimental-deadline-stop.json` preserves both userspace/readiness reports.
No experimental connection was made; there is no fabricated `experimental/`
capture. `operator-photo-correction-and-recovery-boundary.json` preserves the
explicit correction that the photo was not captured. The earlier witness is
retained, not erased.

`operator-stock-incomplete-readiness-01.json` records “ready stock.”
`stock-capture-readiness-stop.json` records why it did not permit a capture and
the operator's request for a future timing redesign. `operator-final-stock-display.json`
records normal STOCK userspace/display as an operator observation. No recovery
connection or boot-identity capture was made. Full first-owner/recovery proof
remains NOT ESTABLISHED.

`capture_once.py`, its tests and source pins preserve the reviewed controller.
Test output is local/mocked, not live ownership evidence. The controller was
never called for either boot capture. Its recovery fallback can retain a missing
experimental ID as UNKNOWN; it cannot grant full recovery qualification.
`progress.json` is the final ledger, not a historical pre-boot snapshot.

`offline-checks-after-deadline-stop/` and `run-offline-guards.py` preserve exact
commands/status/output for 307 repository tests, 14 boot-analysis tests, three
UBSan harnesses, generator, procedure, candidate CRC checks and deterministic
dry runs. `deadline-guards.*` records 11 unchanged wrapper/readiness tests;
`controller-final.*` records six controller tests. `--complete` keeps its expected
exit 1 and four PARTIAL labels.

`offline-display-boundary.json` records source hashes and the current scanout
handoff gap. It neither executes nor qualifies a new display operation.
`documentation-before/` and scoped diffs preserve earlier checkpoint documents.
`RESULT-in-progress-before-final.md` keeps the earlier in-progress report.
`offline-document-write-failure.json` records a local wrong-directory write
failure and its correction; it caused no target action.

`final-artifact-checks.json`, `final-verification-summary.json`, local-link checks,
Git status and diff-check output distinguish new documentation/evidence from
unchanged artifacts and production code. The timing proposal is unapproved;
current 120-second guards remain unchanged. No commit or push.
