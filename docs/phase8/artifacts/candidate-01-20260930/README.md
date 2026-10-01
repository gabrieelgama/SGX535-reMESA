# Candidate #1 offline build evidence — 2026-09-30

**NEW CANDIDATE: NOT BUILT. ABI QUALIFICATION: NOT ESTABLISHED.**
**Target contact NONE; Gate B BLOCKED; whitelist `[]`.**

See [report](../../candidate-01-offline-build-qualification.md). Fresh staging:
`/home/gama/sgx535-offline/candidate-01-20260930/module`. No Kbuild/modpost ran,
no patches were applied, no target table was installed into the successor.

- `pwd.txt`, `initial-git-status.txt`, `initial-repository-files.json`: initial workspace/repository identity; preexisting changes preserved.
- `inputs.json`, `paths.json`, `source-identity.json`: immutable authoritative source/archive/config/table identities and separate staging/preparation paths.
- `prior-evidence-check.json`, `preservation-check.json`: preceding manifests verified, qualified generated state unchanged, failed predecessor unchanged, old candidate unchanged.
- `tool-identity-check.json`, `actual-tool-identities.json`, `build-environment.json`: qualified executable hashes, freshly observed versions/targets, intended Kbuild environment. No target compiler/linker was consumed by candidate Kbuild because the integration guard stopped first.
- `prepared-state-before.json`: pre-task hash inventory of 7,426 qualified config/generated-state files, all verified unchanged.
- `original-gma500-source.json`, `fixed-inputs.json`, `fixed-inputs/`: exact 71-file original-source inventory and fifteen byte-exact fixed implementation/UAPI/include inputs. Three original integration files are retained for context analysis.
- `patch-application.json`: first strict Makefile dry-run argv, exit 1 and output, preserved without alteration.
- `patch-context-diagnosis.json`, `original-Makefile`, `retained-makefile.patch`: exact missing EOF blank-line proof, checked against the archive member.
- `other-patch-dry-runs.json`: independent strict IRQ PASS and ioctl FAIL dry runs; neither applied.
- `ioctl-patch-context-diagnosis.json`, `retained-ioctl.patch`, `retained-irq.patch`, `psb_drv.c`, `psb_irq.c`: exact retained contexts and verbose diagnostic result. Ioctl old contexts occur byte-exactly, but strict GNU patch rejects the hunks; precise tool rejection cause remains UNKNOWN.
- `inventory-reader-note.txt`: temporary evidence reader initially assumed all manifest values were objects. It was corrected to accept the retained hash-string schema as well; no source/tool/evidence input was altered. That inspection-only scripting error is separate from the integration guard failure.
- `verification/`: complete commands, stdout/stderr and exit codes for 193 Python tests, generator check, three strict UBSan builds/runs, two dry runs, expected --complete refusal and both module/table controls.
- `qualification-decision.json`: truthful terminal decision; absent candidate identity/import fields remain null, not synthetic PASS values.
- `final-git-status.txt`, `final-verification.json`, `manifest.json`: repository delta/document/whitespace checks and evidence hashes.

No `.ko` or host-harness executable is stored here. Harness binaries remain
in the external work directory. No relaxed patch application, implementation
change, CRC edit, target operation, staging, commit or push occurred.

**Next step OFFLINE:** separately scope retained Makefile/ioctl integration
patch correction/review, preserving fixed-service semantics, followed by strict
no-fuzz application checks. Only then resume compilation and full candidate
qualification. Deployment and the separate hot-transition problem remain
unauthorized/outside this task.
