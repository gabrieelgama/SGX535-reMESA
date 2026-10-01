# Isolated antiX Kbuild preparation/equivalence evidence — 2026-09-30

**EQUIVALENCE: FAIL / HARD STOP. No candidate compiled; no target contact.**

See [report](../../antix-kbuild-preparation-equivalence.md). This directory
preserves new local evidence, not a target observation or a deployment bundle.
Gate B remains BLOCKED; whitelist `[]`.

- `inputs.json`, `paths.json`: immutable source/config/table identities and isolated paths.
- `initial-git-status.txt`, `initial-repository-files.json`: pre-task working-tree inventory.
- `source-extract.json`, `package-patch*`, `package-source-before-clean.json`: exact-source reconstruction; old package table recorded then removed in disposable source.
- `build-environment.json`, `mrproper*`, `olddefconfig*`, `modules_prepare*`: actual Kbuild argv/environment/logs/exit codes. The first preparation invocation has no recoverable exit record; the recorded resumed invocation exited 0.
- `interrupted-preparation-state.json`: partial state before resumption; exact interruption cause UNKNOWN. Its selected-file inventory did not include the syscall header.
- `raw-generated-comparison.json`, `classified-comparison.json`, `comparison-diffs/`, `config-semantic-guards.json`: unnormalized comparisons and explicit classifications.
- `prepared/`: preserved prepared config/header bytes, including the unchanged **truncated** syscall header; not approved build inputs.
- `syscall-truncation-diagnosis.json`, `source-diagnosis/`: source-level explanation of non-atomic generator writes and Kbuild reuse; compile.h is a full-kernel identity product.
- `syscall-diagnostic/`: fresh generation into a separate diagnostic file matches target. It did **not** replace the failed prepared output.
- `prepared-elf-identities.json`: actual native ARM64 Kbuild helpers and target ELF32/i386 preparation object.
- `verification/`: 193-test suite, three strict UBSan builds/runs, generator, two dry runs, expected --complete refusal and reference/table check. Compiled harness binaries remain outside the repository.
- `manifest.json`: hashes and sizes of retained local evidence.

Source archive, package diff and captured target reference bytes were not
modified. Captured target table remains the sole qualified CRC reference;
it was not copied into the failed output because the equivalence gate failed.
The complete source/output/toolchain remain at `paths.json` locations.
No SGX implementation, fixed UAPI, historical evidence or test predicate changed.
No staging, commit or push.
