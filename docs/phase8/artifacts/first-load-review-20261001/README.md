# Offline first-load review evidence — 2026-10-01

Decision: **First-load alternative/fallback UNKNOWN/BLOCKED; Gate B BLOCKED;
whitelist `[]`; no target contact or SGX fire.**
Read [the qualification report](../../first-load-boot-qualification.md).

## Inputs and source provenance

- `initial-pwd.txt`, `initial-git-status.txt`, `initial-repository-files.json`:
  task-start state and hashes of 1,903 preexisting Git-visible files.
- `retained-inputs.json`: exact original/candidate/derivative, captured target
  config/table, integrated driver and boot-log hashes. Both qualified candidate
  hashes remain unchanged; no rebuild or repeat ABI qualification was needed.
- `source-contracts.json` and `source-contracts/`: byte copies from the unchanged
  equivalence-qualified exact antiX source. PCI core, module loader, vgacon,
  framebuffer and selected gma500 acquisition code are available for review.
  No captured source or generated build state was modified.
- `original-modinfo.*`, `lifecycle-derivative-modinfo.*`,
  `inspection-commands.json`: **local** read-only ELF metadata extraction;
  no remote commands. Same internal name/target alias, dependency list and
  absence of encoded softdeps do not establish target external loading policy.
- `retained-boot-timeline.json`: exact numbered H0 dmesg lines. Its source is
  operator-provided retained target dmesg, not a fresh root capture. Registration
  after root mount does not identify the original loader or initramfs membership.
- `proof-classification.json`: conclusions and proof classes, including UNKNOWN
  loading/fallback and conditional first-load design. No bootloader or initramfs
  from the target was captured in this turn.

## Executable offline checks

`run-guards.py`, `checks.json`, individual `*.stdout`/`*.stderr` and
`guard-summary.json` preserve argv/exit codes and verification output. To repeat,
copy the runner into a **new outside work directory**, not this immutable record.

- **215 scoped tests PASS, zero skips**, including seven new first-load tests.
- First-load native strict GCC/UBSan executes actual PCI matcher,
  `__pci_device_probe`, `local_pci_probe` and `blacklisted` function bodies.
  Kernel PM calls, scheduling and the device probe callback are controlled
  boundaries. Dynamic PCI IDs, concurrency, hardware probe execution, native
  kernel structure layout and target boot scripts are not simulated/proved.
- Both real ELF modules are checked against the retained Mini 12 modalias and
  internal name. Removing the existing-owner guard and ignoring a blacklist
  entry each produces an assertion failure, required by the mutation test.
  Mutations affect temporary native harnesses only; no production source changes.
- Existing 10 IRQ lifecycle tests/13 scenarios remain green.
- Three separate freshly compiled strict contract/service/IO UBSan harnesses
  PASS; generator check PASS.
- Both dry runs SHA-256:
  `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
- `--complete` exit 1, stderr exactly
  `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE` (expected refusal).

## Repository preservation

`working-tree-delta.json` records additions and changed preexisting paths;
only five current-document pointer additions are allowed among the 1,903
preexisting files. Production code, patches, candidate binaries, Attempt 03,
failed recovery and prior finalized qualification bundles are unchanged.
`final-diff-check.*` records the final whitespace check. `manifest.json` hashes
all files in this bundle except itself. No stage, commit or push occurred.

The next missing fact is the selected stock boot/initramfs loading chain and
known-good fallback, requiring a separately authorized read-only observation.
There is no approved experimental boot image, module insertion or SGX action.
