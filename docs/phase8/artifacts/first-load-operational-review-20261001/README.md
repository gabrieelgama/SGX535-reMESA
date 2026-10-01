# Offline first-load operational review — 2026-10-01

No target contact/mutation, staging, reboot, module operation or SGX action.
No image/candidate reconstruction. Gate B BLOCKED; SGX whitelist `[]`.

## Entry points

- [Qualification report](../../first-load-operational-review.md)
- [Exact procedure](../../first-load-staging-boot-recovery-procedure.md)
- [Operator checklist](../../first-load-operator-checklist.md)
- [Pinned machine-readable contract](../../first-load-operational-procedure.json)
- [Final checks](checks-final/summary.json): 266 repository tests, zero skips;
  14 boot-analysis tests, three strict UBSan harnesses, both candidate ABI checks,
  generator, deterministic dry runs and expected fail-closed --complete.
- [Commands/status](checks-final/checks.json), [runner](run_guards_final_successor.py).
- [Post-documentation qualified check](post-documentation-qualified-checks/summary.json).
- [Fresh image inspection](fresh-image-inspection): independent parser and GNU
  cpio inspection of the existing pinned image; no construction.
- [Pinned-artifact check](initial-artifact-verification.json): all 145 files in
  the prior image qualification manifest hash/size match.
- [Adversarial findings/fixes](adversarial-review-resolution.json).
- [Trace-handoff uncertainty](hook-log-handoff-evidence.json): post-pivot /run log
  delivery UNKNOWN, never assumed; absence prevents first-owner qualification.
- [Starting state](initial-git-status.txt) and [starting file hashes](initial-files.json).
- [Initial-file preservation](preservation-initial-files.json),
  [historical image manifest](historical-image-manifest-final.json),
  [links](links-check.json), [decision](qualification-decision.json).
- [Final preservation](preservation-final.json), [repository delta](working-tree-delta.json),
  [evidence manifest](manifest.json): generated after documentation updates.

## Evidence interpretation

The observation fixtures are ordinary synthetic JSON, not target captures.
A validator result explicitly says PASS PROVIDED RECORD: independently verified
raw capture/provenance must support any future real conclusion. Nothing in these
records changes UNKNOWN live first-owner/display/SSH/reset-recovery to PASS.
The retained kernel hash is a prior on-target observation; the kernel binary is
not present locally. It was not freshly rehashed in this offline task.

The image remains 50,804,481 bytes,
SHA-256 `4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`.
Embedded derivative remains
`91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74`.
Existing byte-identical repeat construction/build evidence is retained, not rerun.

## Preserved failed checks

`procedure-red.*` is the initial missing-validator test failure.
`stage-identity-red.*`/`green.*` preserve a test indentation error; these do not
establish red→green behavior. `stage-identity-red-valid.*`/`green-valid.*` contain
corrected failing/passing fresh-stock controls. `review-regressions-red.*` contains
30 failures proving the five review gaps; `review-regressions-green.*` passes17.
`checks/` is the pre-review 261-test run. `checks-final/` is the successor266-test
run after review corrections. The first final-run wrapper accidentally changed a
work-path date while updating a numeric test-count string and failed mkdir before
any checks; `guards-final.*` and `run_guards_final.py` preserve this mechanical
failure. `guards-final-successor.*` and the successor runner record the corrected
successful run. No pinned input/output image, candidate or hardware was changed
by any of these failures.

`post-documentation-checks/suite.*` preserves an incomplete verification run:
it used the default host environment, ran266 tests but skipped the i686 test.
Its zero-skip assertion rejected that run. The prerequisite is explicitly
`SGX535_I686_SYSROOT` plus qualified toolchain environment in
`test_frozen_kernel_contract.py`. No test guard changed.
`post-documentation-qualified-checks/` repeats the same suite with the already
qualified environment and requires266 tests with zero skips; its summary records
the successful outcome. The earlier complete `checks-final/` run already used
this qualified environment. Incomplete runs never supply the final PASS claim.

## Next step

Ready to request separate live authorization for this exact non-SGX staging,
one manually selected first-load boot, passive evidence and operator stock recovery,
subject to fresh stock/privilege/physical-menu guards. No current execution
permission, SGX authorization, or hot-restoration permission exists.
