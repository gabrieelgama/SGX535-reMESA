# Phase 8 — first FIRE HOLD correction, 2026-10-05

## Preserved attempt and cause

Exactly one approved client invocation occurred on boot
`7122635d-5754-4be7-ad68-f89a30cba17f`. Its authorization is consumed.
The ioctl returned 0, client exited 1, operation result was -1, outcome 3,
phase 11 (HOLD), accepted ledger 0; no retirement or color/readback was established.
The complete 4268-byte response and failure capsule agree. Originals remain in
[/home/gama/sgx535-offline/phase8-source-lifecycle-live-preparation-20261005T221026Z-f243e1b4/fire-one-authorized-call/originals/](/home/gama/sgx535-offline/phase8-source-lifecycle-live-preparation-20261005T221026Z-f243e1b4/fire-one-authorized-call/originals/).
No original or historical outcome record is rewritten.

The retained kernel diagnostic is:

```text
SGX535 fixed HOLD: reached=21 failure_stage=0 source=3 raw=-1 obs_stage=21 load=00000000 status1=00000000 status2=00000008 initend=00000000
```

CONFIRMED: stage 21 is STATUS_PROCESS and source 3 is SERVICE_CHECK.
The status callback returned successfully; this is neither sampling failure nor
timeout. The service prepares/fires before entering that sample loop
([service](../../tools/psb-dri-re/frozen_fixed_service.c), `run_once_impl`).
The earliest evidenced rejection is the unknown STATUS2 predicate in
`sgx535_frozen_session_observe_status`, now line 976 of
[contract](../../tools/psb-dri-re/frozen_kernel_contract.c):
`status2 & ~(1U << 4)`. It rejects observed bit 0x8 and enters HOLD with -1.
Phase/owner/sequence checks are required before acceptance; no invalid capsule or
source interval was reported. No TA/end-render/3D acceptance was observed.

CONFIRMED upstream construction defect: the load plan kicks TA, 3D, HOST and
DHOST loads (four actions), but the old actions 22–24 polled/acknowledged/verified
only 0x7. SGX535's public register definitions name 0x8
`DPM_DHOST_FREE_LOAD`, not TA completion or BIF fault
([definitions](../poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt),
lines 125–134). All four completions can be present yet 0x8 survives the old
acknowledgement; a delayed fourth completion can also escape. Both cases are
reproduced by the synthetic actual-plan/executor regression.
The retained 0x8 is consistent with this exact defect. Source analysis establishes
the host-side fire callback/action sequence returned successfully before sampling;
it does **not** prove device TA acceptance or completion. `issued=1` alone remains
possible issuance only. No restricted entry/reporting source was inspected or derived.

## Minimum correction

The same four load kicks remain. Actions 22–24 now observe all four bits (0xf),
acknowledge only those observed load completions, and verify all four consumed
before INITEND/TA. Bootstrap validation and the backend's successful load receipt
require 0xf. The pre-TA baseline rejects any unconsumed STATUS2 event. Scene
acceptance remains strict: unexpected STATUS2 bits still cause HOLD; faults are
not masked or erased. This is consumption of observed **current-operation load**
completions, not pre-admission quiescence fabrication. No reset, added submission,
raw-status journal, identifier, new interface, request/UAPI or client change.
The existing status-validator failure diagnostic now labels failure_stage=21.

Independent checker correction: producer `frozen_capsule.h` defines CLOSED=4,
and both proc encoders export that value. The offline source checker and its old
synthetic fixture incorrectly demanded 6. They now require 4, with a compiled-enum
regression and rejection of all other states. The preserved closed-source record
is consistent after correction; this does not turn the failed operation into success.

## Qualification and reuse

[Machine-readable qualification](dhost-load-hold-fix-qualification-20261005.json)
records the native/UBSan contract, service, owner and capsule-service regressions;
8 new MMIO-model load cases in each mode; 6 source-checker and 13 capsule-evidence
tests; static i386 compilation; module ABI/vermagic/import/CRC checks; repeated
driver build; and two byte-identical finished-image constructions with exact hook
integrity binding. Synthetic tests do not establish native hardware timing.
The prior temporary qemu emulator is unavailable: new i386 runtime tests were
NOT PERFORMED, rather than silently reported as passing.

Unchanged qualification is reusable for approved client/4268-byte export, fixed
request/UAPI/workload, accepted scene-event ledger, owner/readback, capsule and
observer, source-lifecycle provider/taps, archive and preservation components.
Changed driver/image identities require fresh FIRSTLOAD/loaded-identity/source
guards. Old build-specific live PASS is not transferred to this candidate.
The opaque old entry/reporting build input is copied unchanged, not inspected.

## Authority and next boundary

The maintainer authorized offline correction; subsequently authorized hardware
access for preparation. No second FIRE is authorized: prepare one fresh retry only
after corrected candidate live qualification, then stop for explicit FIRE.
No hot replacement/rearm, no replay on the consumed old boot, no automatic boot
retry. Normal STOCK recovery, healthy fresh STOCK preflight, distinct exact staging,
one manually selected FIRSTLOAD, and passive new-boot guards precede a new card.
PRE07 is BLOCKED pending that fresh live evidence. Triangle ATTEMPTED, NOT ESTABLISHED;
historical approved client calls=1; new calls=0; `sgx_execution_authorized=false`.
