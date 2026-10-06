# Source-lifecycle quiescence — offline qualification, 2026-10-05

[Exact qualification and source pins](source-lifecycle-quiescence-qualification-20261005.json).
[Prospective package](/home/gama/sgx535-offline/phase8-source-lifecycle-20261005T064455Z/one-shot-package.json).
Authority: the maintainer's autonomous **offline** implementation instruction, preserved in
[authority.txt](/home/gama/sgx535-offline/phase8-source-lifecycle-20261005T064455Z/authority.txt).
No target contact, client execution, ioctl or SGX invocation occurred.

## Root cause and selected provider

The old guard had a deliberately false delayed-work premise. Quiet STATUS1/2 and
2D FIFO/busy readings cannot distinguish a clean engine from an older TA/3D operation
whose event has not arrived. Trace silence cannot provide the missing starting history.

The provider now retains **the existing first initialization reset lifecycle**, before
DRM registration, plus the absence of earlier eligible work and continuous covered
producer/source history. It adds no reset, submission, completion acknowledgement,
clear, delay or recovery operation. It observes the original `psb_spank` calls/writes:
BIF/DPM/TA/USE/ISP/TSP/2D reset assertion, its posted read, original delay/release,
and existing BIF handling. The full assertion must read back exactly `0x7f`.
This is lifecycle cancellation of earlier execution, not a claim that an earlier
operation completed successfully. A later reset can never repair or rearm the witness.

The source basis is [P3-034 and the BRN23944 caveat](../poulsbo-power-reset.md),
[SGX535 definitions](../poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt)
and [the permitted reset reference](../poulsbo-data/EMGD_drm_pvr_services4_srvkm_devices_sgx_sgxreset_c.txt).
The definitions distinguish reset domains, BIF outstanding **reads**, event timer enable
and pending event kick. The public [initialization reference](../../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c)
separately kicks/wakes the microkernel; reset alone is insufficient if an autonomous
wake can subsequently resume old software queues. Accordingly, an enabled event timer
or pending kick blocks this provider rather than being disabled/cleared for PASS.
These references establish the reset-domain contract; synthetic tests do not establish
actual hardware reset effectiveness, clocks/rails or native timing.

## Causal proof and scope

The native admissibility theorem has these explicit premises:

1. The exact observer was loaded before the exact driver's first initialization;
   no earlier frozen ioctl was admitted. A single device/module/reader attachment
   and the one expected initialization-reset lifetime were observed.
2. Powered, mapped reads before and after that reset contain no non-benign event,
   2D busy/FIFO state, outstanding BIF read, BIF fault, enabled timer or pending kick.
   Reset assertion/release are verified. Observed bad pre-reset facts remain bad
   even if the original reset later changes them. BRN23944/fault recovery is not
   presumed; such states are rejected, not recovered into a successful witness.
3. Hardware conforms to the documented SGX535 reset-domain semantics. Earlier
   engine work cannot survive the accepted initialization boundary; outstanding
   reads and autonomous wake are independently checked. Neither a zero reset
   register nor BIF_READS=0 is treated as a standalone global-idle certificate.
4. In the exact driver, normal work submission is the observed 2D helper, while
   ASLE/hotplug/lid deferred work concerns display rather than TA scheduling.
   MMU mapping/invalidation and 2D clock gating do not issue TA/3D work. Reset,
   power and teardown discontinue the seed. Frozen MMIO writes require the same
   current qualified capsule lease; wrong/deferred producers are refused.
5. Existing live loaded-identity, ownership and isolation requirements exclude
   alternate kernel/privileged raw-MMIO producers. This mechanism does not
   attest arbitrary privileged hardware tampering.

Under these premises the starting witness excludes delayed earlier TA/3D work.
Any missing premise yields UNKNOWN/BLOCKED. A raw clean sample, old UNUSED capsule,
user-supplied flag, repeated polling or elapsed sleep cannot supply this proof.
The excluded entry/reporting material was copied unchanged as an opaque build input;
its contents/control flow were not inspected, reconstructed or used as a specification.

## Boundary through the operation

The root0400 read-only `/proc/sgx535_source_guard` exposes **SGXSOURCE2**.
Before any public call, opening it pins the attached driver and invokes its passive
callback. The callback takes the existing fixed IRQ lock, then the observer lock,
checks the valid mapped lifetime and reads at most nine status/control registers.
It does not power up, acknowledge, reset, drain or admit an operation. A clean sample
with valid startup history arms `prepared=1`. Preparation failures are retained.
Subsequent reads cannot rearm a lost interval; after admission/closure they read RAM only.

The interval begins with this passive prepared witness, before execution authorization.
Balanced direct 2D taps and existing reset/PM/teardown taps retain relevant competing
activity without kprobe symbol assumptions or a trace-buffer loss channel. Existing IRQ
observations are retained **before** their existing ACK, so delayed service delivery
cannot silently erase prior-source evidence. Unmatched/overflow accounting, unexpected
attachment/rebinding, bad sample and lost lifetime all block. These are covered-path
witnesses under the exact producer/isolation premises, not system-wide tracing.

Backend begin resamples under the IRQ lock **before** publishing an active backend or
admitting the capsule. It requires the previously prepared witness, no pending software
state and the same device/operation. Each frozen register write verifies the valid source
interval and current capsule/task/backend/owner lease. Activity after preparation is
not forgiven because it ended before admission. The source interval closes at the public
ioctl exit; invalid source evidence invalidates attributable capsule evidence. Crashed or
unclosed calls provide no successful closed interval. There is no retry/rebind/rearm API.

## Retention and failure

The record retains state, reason mask, active producers, driver/capsule state, startup
lifecycle stage, prepared state, asserted/released reset and startup pending/read/fault/
autonomous facts. No UUID, exported sequence or accepted raw STATUS journal is added.
The 4,152-byte capsule, 4,268-byte fixed response ABI, approved client, request and
accepted-event/retirement contracts remain unchanged.

Protected original **preparation and post-call source records** must be preserved with
the capsule/response/image/streams and bound to the same actual boot and exact loaded
module notes. The [offline decoder](../../tools/psb-dri-re/frozen_source_guard_evidence.py)
checks complete SGXSOURCE2 records: prepared UNUSED before, CLOSED source and capsule
after, valid lifecycle, no reasons/active producers, and all required fields. It always
separates supplied-file consistency from live provenance. Synthetic perfect records
cannot establish hardware or a triangle. Truncated/missing/mixed evidence, copy loss,
power loss, hang, crash or ambiguous issuance stays UNKNOWN; preserve partial originals,
STOP, and never retry after possible issuance. Source evidence supplements rather than
replaces the existing capsule/readback and archive verification.

## Focused qualification and artifacts

- **183/183 native**, **183/183 UBSan** policy checks PASS.
- **8/8 adapter tests**, zero skips: actual backend admission, original reset routine,
  pinned/serialized passive reader and producer write gate exercised with fake MMIO;
  unchanged hardware writes/reset calls/delays checked against the qualified base.
- **5/5 decoder tests** PASS, including partial/mixed/stale/lost evidence and the
  prohibition on upgrading synthetic consistency to physical provenance.
- Both modules: ELF32/i386, exact vermagic, all imports and CRCs PASS; no unsupported
  symbol/import or circular driver dependency. Independent module repeat-build
  reproducibility is not claimed.
- Two finished image builds are byte-identical. Finished-image `/init` embeds the
  packaged hook hash; observer precedes driver, packaged modules match exact pins,
  hook syntax passes and all unrelated members/early prefix remain byte-identical.

| Artifact | Bytes | New identity |
| --- | ---: | --- |
| Driver | 250,672 | Build ID `f243e1b417e8a44fae3b5be1798b40b6e4e4b090`; SHA-256 `f95872ad78cfd2a621aa31b1a81c3193a6b5ec154759683f2d496a3b09f02173` |
| Observer | 26,284 | Build ID `f11d3abb072caa4e1d32836ef92ce201e9c9d126`; SHA-256 `2e35e863d51dbc1d407feef08f08bb2edc6390af621a0f173a9b1a73ec76158a` |
| FIRSTLOAD image | 50,816,585 | SHA-256 `c621ea61622bb5c15e83e0d1ad657f2ba96ce23dc277ed6c7ac007d615f76f5d` |

These tests qualify mechanism, integration and conservative classification, **not
physical quiescence or live execution**. Unchanged component qualification is reused;
changed driver/observer/image identities and native source integration require new
live bindings. Old qualified images, malformed-image/HOLD evidence and old guard
qualification remain preserved. No historical Gate B/client decision is silently rebound.

## Next mechanical boundary

The two source requirements are now technically satisfiable by this concrete candidate.
Actual PRE07 remains **BLOCKED**: it has not been booted and has no fresh live startup,
prepared source/continuous isolation or loaded-identity evidence. The previously successful
boot lacks this provider and was not contacted or hot-replaced.

The next step requires separate bounded live-preparation authority: reviewed STOCK
recovery and a healthy passive baseline; stage the exact new package in distinct
destinations; one controlled FIRSTLOAD boot; verify actual module notes, UNUSED capsule,
prepared source witness and existing fresh continuity/isolation/preservation guards.
No further design investigation is required for this mechanism. If any source predicate
fails, preserve the record and STOP; do not reset or repeat a boot to force PASS.

**READY FOR LIVE QUALIFICATION: YES. READY FOR EXECUTION AUTHORIZATION: NO.**
`sgx_execution_authorized=false`; SGX invocations=0; task hardware interactions=0;
approved client never executed; triangle=NOT ATTEMPTED.

[Final preservation and artifact consistency receipt](/home/gama/sgx535-offline/phase8-source-lifecycle-20261005T064455Z/final-consistency.json): all 3,705 baseline files compared; only listed in-scope files changed. Git is 18 modified tracked files, 109 untracked entries, empty index (start: 17/104/empty). Nothing was staged, committed, cleaned, stashed or discarded.
