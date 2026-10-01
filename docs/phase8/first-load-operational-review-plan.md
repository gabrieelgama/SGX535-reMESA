# First-load staging / boot / recovery operational review plan

Execution: inline, strictly offline. The operator's current procedure-review
request is the scope; no new image, driver, target mutation or deployment.

## Deliverables and checks

- [x] Verify the pinned image, historical manifest, embedded derivative/closure,
  hook/init ordering, stock provenance and exact GRUB text without rebuilding.
- [x] Specify exclusive staging destinations, destination read-back/fsync,
  unchanged stock/default guards, manual-only selection and STOP at each boundary.
- [x] Specify mandatory first-owner evidence, bounded observation, no SGX/hot
  recovery, operator reset boundary and independent stock-recovery criteria.
- [x] Add offline record validators with failing tests for overwrite/symlinks,
  default changes, missing trace, wrong loaded identity/IRQ, false recovery and
  accidental extra experimental boots/SGX actions. They cannot contact a host.
- [x] Adversarially review persistence, physical access and evidence loss; mark
  practical boot/recovery/trace delivery UNKNOWN until the scoped live test.
- [x] Rerun repository, image/hook/boot helpers, ABI, UBSan/generator/dry guards;
  preserve every historical file and record the exact repository delta.

## Review focus

- custom.cfg was absent in the capture: existing file of any type is STOP,
  not permission to append or overwrite unreviewed entries.
- Verify existing /boot parents and use exclusive, no-follow file creation;
  destination hashes after sync, not just successful transfer.
- Same kernel/cmdline cannot identify the initramfs used; require selection
  evidence plus hook trace and loaded derivative note.
- Hook output is redirected to volatile /run; post-pivot delivery is UNKNOWN,
  and missing trace must not be called first-owner success.
- Stock entry itself calls savedefault: select only its captured normal ID;
  never promise that all of grubenv/filesystem/device state is reset automatically.

## Execution boundary

Finish with procedure/checklist and explicit readiness/UNKNOWN predicates.
A future first-load authorization is separate from SGX Gate B and the empty
SGX whitelist. Physical menu access and exact stock guards are mandatory before
one experimental selection. No permission is created by this review.
