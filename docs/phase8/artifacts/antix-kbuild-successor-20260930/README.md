# Successor antiX preparation/equivalence evidence — 2026-09-30

**KERNEL PREPARATION: PASS. CONFIG/HEADER/LAYOUT EQUIVALENCE: PASS.**
**No candidate compiled; no target contact. Gate B BLOCKED; whitelist `[]`.**

See [report](../../antix-kbuild-successor-equivalence.md). This is new local
successor evidence; the [failed preparation](../antix-kbuild-preparation-20260930/README.md)
and its output remain unchanged.

- `initial-git-status.txt`, `initial-repository-files.json`: pre-task repository snapshot.
- `inputs.json`, `source-identity.json`, `paths.json`: authoritative immutable inputs, selected reconstructed source hashes and distinct output/work paths.
- `failed-output-before.json`, `failed-output-preservation.json`: every failed-output entry's hash/link/mode/inode/mtime; comparison with the full after-snapshot has identical digest. Full after-snapshot remains in the external workspace to avoid duplicate evidence bytes.
- `qualified-tool-identity-check.json`, `actual-tool-identities.json`, `build-environment.json`, `environment-cleanup.json`: retained qualification hash checks, actual compiler/version identities and Kbuild environment.
- `olddefconfig*`, `modules_prepare*`: exact argv, stdout/stderr, timestamps and exit codes. Normal fresh Kbuild invoked the syscall generator; no header was copied into the successor.
- `olddefconfig-guard.json`, `compare-successor.py`, `complete-equivalence.json`, `comparison-diffs/`: complete raw/semantic comparisons, byte hashes, classifications and analysis procedure. No meaningful normalization or implementation change. To repeat analysis, copy the script/paths/environment into a new diagnostic work directory; do not execute in this immutable evidence directory.
- `prepared/`: prepared configuration and header bytes, four Kbuild syscall dependency records and generated module linker-script/command provenance. No compiled kernel/module/harness binaries are copied here.
- `source-contract/`: exact source rules for syscall generation, full-kernel compile.h and module linker-script preparation.
- `verification/`: fresh 193-test suite, three UBSan builds/runs, reference CRC check, generator, two dry runs and expected --complete refusal.
- `final-verification.json`, `manifest.json`: preservation, documentation checks, repository delta and evidence hashes.

The target Module.symvers is unchanged and was NOT yet installed into the
successor output; installation and candidate build belong to the separately
scoped next step. Preparation/header equivalence does not qualify an artifact,
a hot transition, runtime SGX operation or deployment. No staging/commit/push.
