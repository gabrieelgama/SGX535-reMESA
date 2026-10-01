# Offline toolchain evidence — 2026-09-30

Local provisioning/smoke evidence only; no Mini 12 contact. See
[qualification report](../../i386-gcc-toolchain-qualification.md).
`manifest.json` hashes the retained raw records (before this index was added).
All package archives, authenticated APT full indexes, extraction prefix and
compiled smoke binaries remain in the external workspace:
`/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/`.
These are cross-toolchain artifacts, not a kernel-module candidate.

`provisioned-packages.json` records 25 exact versions/archive hashes and
signed package provenance. `executable-identities.json` records ARM64 tool
hashes and dependency resolution. `smoke-results.json` records ELF32 target
and ELF64 native outputs. Commands and initial failed smoke invocations are
preserved. `environment.sh` is local environment setup only; it is not a
build/deployment command. Tests and ABI-input checks are preserved separately.
No x86 executable was run. No historical evidence was changed.
