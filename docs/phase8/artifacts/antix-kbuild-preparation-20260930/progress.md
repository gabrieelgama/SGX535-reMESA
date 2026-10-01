# Isolated antiX Kbuild preparation ledger

Scope: exact-source reconstruction, olddefconfig, preparation and target equivalence only. No candidate build; no target contact.
- Initial pwd/status and repository-file hashes preserved.
- Qualified toolchain manifests, executable and package archive hashes revalidated.
- Exact retained original source archive, source-package diff, target config/table/generated tar hashes verified.

- Exact package diff applied with --fuzz=0 and no rejects. Reconstructed Makefile byte-matches captured target Makefile.
- Package supplied incompatible old table recorded then removed by normal mrproper in fresh disposable source only. No preserved source/evidence touched.
- Qualified rootless environment selected; copied captured .config into fresh separate output.

- olddefconfig completed: 8788 values equal except documented compiler-name banner.
- Resumed interrupted modules_prepare in the same isolated output after confirming no old process remains; final exit recorded. No SGX candidate compiled. No public or generated symbol table used.
