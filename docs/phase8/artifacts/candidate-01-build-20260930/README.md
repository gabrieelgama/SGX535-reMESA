# Candidate #1 integration/build and lifecycle evidence

Started 2026-09-30; completed 2026-10-01 UTC. This is an **offline** record.
Gate B BLOCKED, active whitelist `[]`; no target contact or SGX action.
See [full report](../../candidate-01-build-and-lifecycle-review.md).

## Final artifacts

| Artifact | SHA-256 | Complete target CRC check |
| --- | --- | --- |
| [Candidate #1](candidate-01/gma500_gfx.ko) | `13591674ef9fa82f185f075185d1fa09d94606ccce7253ed231627f2649c600f` | 230/230 PASS |
| [Lifecycle derivative](candidate-02-lifecycle/gma500_gfx.ko) | `91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74` | 232/232 PASS |

Both ABI QUALIFICATION PASS offline; zero missing/mismatched imports,
module_layout `0xb84efb99`, ELF32/i386, exact target vermagic. Both clean repeats
are byte-identical. The derivative includes the independent IRQ cleanup fix;
Candidate #1 remains preserved separately and is not silently substituted.
Neither is a deployment-authorized or runtime-qualified artifact. The identity
JSON `path` field is the original build-time `M=` location, reused for repeats;
select artifacts by the preserved filenames above and their hashes.

## Evidence map

- `initial-git-status.txt`, `pwd.txt`, `initial-repository-files.json`: task-start
  repository state; previous unrelated changes and artifacts were not cleaned.
- `inputs.json`, `paths.json`, `tool-identity-check.json`,
  `actual-tool-identities.json`, `build-environment.json`, `build-argv.json`:
  source/config/table/tool/environment identities and ordinary external Kbuild.
- `before-*.patch`, `after-*.patch`, `final-*.patch`, `patch-root-cause.json`,
  `gnu-patch-context-manual.txt`, `patch-application.json`: old failure and exact
  equivalent Makefile/ioctl corrections, local GNU patch contract, three-patch
  dry/application results. Fixed IRQ patch remains byte-unchanged.
- `integration-red.*`, `integration-green.*`, `division-red.*`,
  `division-green.*`: reproduced failures and passing mechanical corrections.
- `build-1.*`: genuine modpost rejection of unsupported `__umoddi3`.
- `build-2.*`, `build-3-repeat.*`, `candidate-01-repeat.json`: successful candidate
  #1 normal builds and bit comparison.
- `build-4.*`, `build-4-rejection.json`: stale input-manifest guard / mistakenly
  launched incomplete staging build, terminated and never qualified.
- `build-5.*`, `before-review-antix-irq-lifecycle.patch`: development lifecycle
  build before review found retained interrupt routing; not the final derivative.
- `lifecycle-red.*`, `lifecycle-green.*`, `lifecycle-routing-red.*`,
  `lifecycle-routing-green.*`: actual-source UBSan RED/GREEN ordering and routing.
- `lifecycle-final-patch-application.json`,
  `lifecycle-repeat-patch-application.json`: **Makefile → fixed IRQ → lifecycle →
  ioctl**, eight strict dry/application results per fresh tree, no offsets/fuzz/rejects.
- `build-6-lifecycle-final.*`, `build-7-lifecycle-repeat.*`,
  `candidate-02-repeat.json`: final derivative builds/repeat comparison.
- `*-provenance.json`, `prepared-state-before.json`, `table-installation.json`,
  `integrated-source-manifest-*.json`, `fixed-inputs-lifecycle.json`: all 7,426
  config/generated hashes unchanged, no added generated/config entries; target
  table byte-exact; actual compiler/linker/modpost/helper/source provenance.
- Each artifact directory: binary, identity, ELF sections/symbols/notes,
  vermagic/dependencies, complete import CRCs and actual-undefined coverage,
  target-table checker output, commands, Kbuild-generated `.mod.c`, integrated
  driver/IRQ/Makefile result. There was no manual version-record modification.
- `original-control.json`, `rejected-control.json`: original 222/222 PASS and
  immutable old candidate REJECT with 154 mismatches.
- `original-irq-disassembly.txt`, `original-irq-callsite-review.json`: original
  selected teardown lacks IRQ release. `free_irq` is present in the whole module,
  but its two relocations belong to Oaktrail HDMI-I2C, not Poulsbo's DRM IRQ.
- `source-contracts/`: byte-exact retained antiX Linux/DRM/IRQ/power/register
  sources supporting the narrow cleanup rule. No broad source reconstruction.
- `fixed-inputs/`: exact current 15 implementation/UAPI/generated inputs.
- `review.md`: independent read-only review and disposition.
- `regression-final.json`, `suite-final.*`, `ubsan-*-final.*`, `generator-final.*`,
  `dry-final-*.stdout`, `complete-final.*`: 205 tests, zero skips; three UBSan
  harnesses plus embedded lifecycle UBSan; unchanged deterministic hashes and
  four-label architectural refusal.
- `documentation-before/`: prior current-document bytes preserved before update;
  these snapshots are historical, not current instructions.
- `qualification-decision.json`, `final-preservation-check.json`,
  `final-working-tree-delta.json`, `final-git-status.txt`, `manifest.json`:
  final classification, hygiene, preservation and evidence identity.

Full failed/successful module/object trees remain outside the repository at
`/home/gama/sgx535-offline/candidate-01-build-20260930`; the selected qualified
output remains `/home/gama/sgx535-offline/antix-kbuild-successor-20260930/output`.
The failed predecessor and preceding candidate-integration STOP are unchanged.
Build scripts record the actual one-time steps; do not rerun staging/freeze
scripts over completed evidence. Use a separately isolated workspace for any
future reproduction. Normal build output/intermediate objects and host UBSan
executables are not copied into this bundle; the two deliberate module binaries
and generated version source are retained for qualification.

## Boundary

The lifecycle derivative cannot correct teardown of the currently active original
module. First original hot removal/recovery remains BLOCKED. Exact recovery-oops
causation is INFERRED. Runtime loader/signature/IRQ/SGX behavior remains unobserved.
Next proposed step is a **separately authorized read-only boot-route inventory**,
not removal/reload, installation, reboot or a triangle attempt.
