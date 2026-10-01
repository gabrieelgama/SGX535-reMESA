# Experimental first-load image 01 — offline evidence

**PASS OFFLINE** for construction, closure/CRC/hash provenance, actual guarded
pre-udev hook, isolated non-saving entry, stock preservation and repeatability.
**Live boot/first-owner/display/SSH/reset-recovery UNKNOWN. Gate B BLOCKED;
whitelist `[]`; no target contact/mutation or SGX fire.**

Read the [qualification report](../../experimental-first-load-image-qualification.md).
Nothing in this directory is staged or installed on the Mini 12.

## Final artifact

[build-03/initrd.img-sgx535-firstload-01](build-03/initrd.img-sgx535-firstload-01)
50,804,481 bytes, SHA-256
`4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`.
[GRUB text](build-03/proposed-custom.cfg) is a proposal only, no saved/default change.
[Actual hook](build-03/sgx535-first-load.sh) is called by the
[modified init](build-03/modified-init) before all init-top scripts/udev.

## Provenance and commands

- Initial pwd/status/file hashes; corrected-inputs.json: all authoritative input
  identities, captured kernel hash-only and preserved stock-image identity.
- construction-environment.json: actual native Python/zlib/cpio identity and
  implementation/test hashes. No target program was executed on this host.
- run_corrected_build.py, corrected-build-commands.json, build-3/4 stdout/stderr:
  fresh actual corrected builds. Copy runners to a NEW directory when repeating.
- verified-corrected-repeat.json: authoritative full-byte comparison of actual
  build03/04, with shell syntax checks of both corrected hook/init files.
- build-03/module-qualification.json: all 1,076 versioned-import records / 1,066
  undefined symbols for the selected ten modules, target CRCs, vermagic, hashes.
- build-03/payloads.json: exact dependency insertion order, paths and hashes;
  utilities.json: preserved hardlinked BusyBox applet payload provenance.
- candidate-01-abi/lifecycle-abi outputs: existing binaries rechecked unchanged
  against the pinned target table. No build/rewrite of either artifact occurred.

## Independent inspection and adversarial controls

- independent-inspection-corrected/: GNU cpio commands/stdout/stderr, full image
  manifest, exact stock delta, qualification. The preserved prior parser is a
  second independent reader; all old records except init remain raw-byte exact.
- negative-images/results.json: four actual mutated archives rejected before
  proof creation (nonexecutable init, colliding inode, corrupt derivative,
  changed early prefix). Mutant bytes remain in the isolated outside work path.
- review-resolution.json and red/green tests: independent review found two
  Important gaps (IRQ attribution and inode/mode verification). Both fixed;
  emitted shell/helpers/archive verifier are tested, not a different model.
- final-focused.stderr: 34 focused tests PASS; insertion failure at every step,
  guard failures, post-load mismatch, init HOLD continuation and mutations.

## Verification and preservation

final-checks/ records 249 repository tests, zero skips; 14 analysis tests; three
fresh strict UBSan harnesses; generator; unchanged twice dry-run hash; expected
`--complete` exit 1 with PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE. Later final
post-documentation checks are recorded separately. Production driver/UAPI,
patches, candidates, captured stock files and all historical evidence are unchanged.

allowed-checkpoint-updates.json permits only six new current notices in preexisting
Phase 8 documents, with byte-exact restoration of their old contents on removal of
the notice. working-tree-delta.json proves all other initial Git-visible files
unchanged. manifest.json hashes every bundle file except itself.

## Superseded records, deliberately retained

preserved-superseded-builds.json identifies immutable pre-review images/build01/02,
outside work path and their old hash. They are not the final qualified image.
The corrected construction runner accidentally measured old build01/02 for its
repeat JSON/syntax checks; that record was detected and preserved, **superseded by
verified-corrected-repeat.json**, which independently reads actual build03/04.
No binary/header/CRC was edited to correct the measurement. Superseded logs and
scripts remain available for audit. All staging must select the final pinned hash.

**Next step: OFFLINE exact staging/boot/manual-reset-to-stock recovery review.**
No live first-load boot or SGX attempt is authorized by this bundle.
