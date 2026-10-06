# Prospective frozen first-owner operator observation

Procedure: `MINI12-FROZEN-FIRSTOWNER-OPERATOR-OBSERVATION-01`.
Applies prospectively to Build ID `314e2f3b37195dc56df7c57dd938d78377a5ea8e`
and the already staged image SHA-256
`fe64b3dcfe74b78a6d96edcd4fd7c118c3901fcff8631afec6c14647da292e3c`
(50,805,231 bytes). Operator authorization is the 2026-10-03 decision explicitly
allowing direct observation instead of project-controlled photographic evidence.
Use only after this revision's tests and focused review complete.

## Origin and authority

The photograph rule originates in the project's [manual first-load selection
procedure](first-load-staging-boot-recovery-procedure.md#3-pre-boot-and-one-manual-selection).
The [timing redesign](first-load-capture-timing-redesign.md) treats missing menu
photography as an evidence gap separate from the failed timing qualification,
requires review before selection, and implements it in the project-owned
`cycle04-first-load-procedure.json` and `frozen_first_load_procedure_v2.py`.
It preserves evidence of the displayed entry and available STOCK default; it
does not measure GPU execution. No external photographic requirement is cited
in the available permitted sources. This is a conservative project evidentiary
requirement, revised here under the owner's explicit decision. The independent
Gate-B/exact-action opening requirement is outside this revision.

## Prospective alternative

Retain the original v2 files and all old classifications unchanged. No photograph
is supplied or claimed. This procedure does not retroactively satisfy v2 or
redeem an already performed experimental selection.

Before one new selection, save the physically present operator's report of the
exact displayed title:

`EXPERIMENTAL SGX535 rev121 FROZEN 314e2f3b37195dc56df7c57dd938d78377a5ea8e FIRST-LOAD ONLY (no SGX)`

The receipt identifies this procedure, the exact Build ID, visible candidate and
STOCK entries, explicitly false `photograph_supplied`, zero candidate boots before
selection, and the saved operator text reference. Bind it to the independently
verified staging boot, unique image path, exact size and SHA-256, four-entry
configuration, and STOCK saved default. Missing, ambiguous or conflicting fields
stop selection. Supplied-record validation alone does not authenticate a report.

Reuse the preserved 49/42-guard staging evidence. A later manual STOCK boot is a
known change in boot context: freshly check its runtime identity/ownership/health,
privilege, unchanged staged-file inode metadata and small boot configuration/default
before another machine boundary. This refresh does not rebuild, restage or repeat
the image hash/qualification. Stop on any unexpected drift.

At visible GRUB, obtain the explicit current menu observation before telling the
operator to select the candidate once. Only the operator performs reset/selection.
Retain 600-second boot watch, 1200-second capture completion ceiling, one bounded
40-second connection/35-second child capture, and no retry. After arrival obtain
normal display/userspace and local `sudo -v` observations before the existing
prepared first-owner capture. Exact loaded candidate note, Live state, ordered
hook trace, PCI/DRM/framebuffer/VT/IRQ ownership, services, taint and complete kernel
log remain mandatory. Zero SGX/hot-module actions and recovery readiness remain
mandatory. HOLD, unexpected ownership, fault, missing attribution or late evidence
stops the session. Use only the existing operator-controlled STOCK recovery boundary.

`frozen_first_owner_operator_receipt.py` validates the alternative and shares the
original candidate checker's technical checks without setting `selection_photo`
true. The original photographic validator still rejects absent photography.
Both paths grant no boot/SGX permission, opening decision or triangle claim.
