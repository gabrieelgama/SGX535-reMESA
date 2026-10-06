# Architecture B implementation and offline qualification — 2026-10-05

**PROVENANCE MECHANISM QUALIFIED, within the recorded offline scope.**
**PRE07 live readiness: BLOCKED. READY FOR EXECUTION AUTHORIZATION: NO.**
The changed candidate has not been staged or first-owner booted; no fresh
candidate identity, producer-origin, isolation, destination, privilege or recovery
observations exist. Hardware contact and these live preparation steps were
explicitly excluded from this implementation authorization.

The bounded implementation review passed under the maintainer's express delegation.
[APPROVE_MINIMUM_IMPLEMENTATION](current-operation-result-provenance-implementation-maintainer-20261005-314e2f3b.json)
records that authority and its offline-only scope. The final independent focused
review and coordinator checks passed after reproduced corrections. No additional
implementation review is pending. The exhausted legacy G01′/G02′ confirmation
route remains unresolved; this implementation does not retroactively certify it.

Primary result: [machine-readable qualification](current-operation-result-provenance-qualification-20261005-cd9b9473.json).
All build, test, source and review evidence is retained under
`/home/gama/sgx535-offline/phase8-provenance-implementation-20261005T005304Z/`.
The exact prepared candidate is recorded in `one-shot-package.json` there.

## Minimum implementation and provenance relation

The new observer never launches a client or issues an SGX command. Generic,
already-available Linux syscall tracepoints delimit the actual public ioctl call
and retain the actual task lifetime. At the permitted backend admission point it
checks the approved executable's exact hash, size and protected file properties.
One private, boot-lifetime capsule binds that call to one fresh owner, its embedded
session and the actual hardware backend. There is no reset, rearm or rebind API.
The observer pins its module after the first eligible command so unloading cannot
reset a possibly consumed attempt.

The lifecycle is UNUSED → CALL → ACTIVE → TERMINAL → CLOSED. A second call or
admission invalidates evidence rather than creating another capsule. This does
not abort or authorize the existing ioctl: **any possible ioctl consumes the
attempt**, regardless of the capsule's issued flag.

Permitted backend/service taps bind each accepted event update to the preceding
actual sampler observation and checked session sequence. The sampler ticket is
transient and erased; no raw STATUS history is retained. The existing qualified
acceptance implementation supplies the ledger. The observer does not invent a
second completion interpretation.

The actual service return supplies terminal result and phase. The existing
retirement-gated readback supplies all 4096 color bytes from that same owner's
color storage before unmapping/release. Independent snapshot storage survives
owner release. Terminal, color and closed payloads cannot be rewritten; a monotonic
invalidity envelope may record later interference and prevents a success claim.
Matched partial issuance, terminal and retired-color facts remain preservable
even after invalidity.

The narrow supplemental interface is root-readable, mode 0400,
`/proc/sgx535_current_operation`. Each open returns a coherent 4152-byte record:
56-byte little-endian `SGXCAPB1` header plus the full 4096-byte color array.
It exposes lifecycle, validity, issuance, terminal result, phase, accepted ledger,
color-presence and syscall result. It adds no UUID, PID, wall-clock timestamp,
exported sequence or raw register history.

The fixed triangle request/response UAPI and approved response-export client are
unchanged. **A new supplemental read-only evidence ABI exists**; this is distinct
from changing the fixed 4268-byte response ABI. The offline reader validates actual
procfs origin and file properties before a bounded read. It was not used on hardware.
Protected preservation uses existing exclusive-write, sync, close and readback
primitives. An independent companion seal binds the capsule to actual archive
files without modifying the original response, image or legacy archive manifest.

The checker compares response ledger, terminal fields, image summaries and all
4096 image bytes with the capsule and original image. Agreement of supplied bytes
alone remains **UNKNOWN hardware attribution**. A future claim additionally needs
verified loaded producer, fresh current boot/context, unused instance, prior-event
quiescence and continuous competing-producer isolation. No physical completion,
GPU-to-CPU visibility or triangle has been observed by this qualification.

## Final candidate identities

| Artifact | Bytes | SHA-256 | Build ID |
| --- | ---: | --- | --- |
| `qualified-build-01/gma500_gfx.ko` | 246972 | `400b16b14a7fd648fe219842e3b0d5994ee8eec910e5a7894f31c373098e8b54` | `cd9b947371f18c2d19af05bfda69fbf9462c2e62` |
| `qualified-build-01/sgx535_provenance.ko` | 15524 | `6842b21b4ab0905e198ca95e07bee7ff52f0d225c8992e9ef18921055482a11d` | `143b1284fc643ea9a7ddd4aeeefa05cdc682718b` |
| `image-build-01/initrd.img-sgx535-provenance-cd9b947371f18c2d19af05bfda69fbf9462c2e62` | 50811745 | `5001a64ff5ef78751aea45f8762eeea8abbfc8ef3c3774c12a34fa214858288d` | — |

Paths in this table are relative to the evidence directory above. Both modules
are ELF32/i386 with matching `5.10.240-antix.1-486-smp` vermagic and
`module_layout 0xb84efb99`. Driver: 241 covered versioned imports; observer: 33.
Both have zero missing imports, CRC mismatches or unversioned undefined symbols.
Final independent builds are byte-identical. The driver now depends on
`sgx535_provenance`; the image loads that observer before the driver.

Two final image builds are byte-identical. Independent archive comparison confirms
that the original 15,006,720-byte early prefix and all other raw members are
unchanged. Only the driver and passive first-load hook were replaced and the
observer module added. Hook shell syntax passed. No image was installed or booted.

Unchanged approved client: 775264 bytes,
`2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835`.
Unchanged client source:
`919c2611e048e4a660ecae3542264adf5a5367d9b0fbe7fbceff2f45f90adedf`.
Unchanged fixed UAPI:
`04dd2080deeb0056a366546fe27faddbdeff6c1657a2471c96e39749dc080420`.
Exact source and design-contract hashes are in the qualification record and
`source-identity.json`; module/image ABI details are in each build's `identity.json`.

## Tests, falsification and corrections

- 45 capsule checks PASS in native, UBSan and static i386/qemu modes.
- 7 real-service differential cases PASS in all three modes: unchanged result,
  phase, accepted ledger, backend ordering/call counts and sample counts.
- 13 new evidence-reader, preservation, comparison and independent-binding tests
  PASS, zero skips.
- The affected existing service regression passed. Unrelated passing suites,
  historical capture and client qualification were not repeated.
- Module ABI/import/CRC, reproducibility, image-member and syntax checks PASS.

Adversarial fixtures cover stale/cross-operation events, color and terminal state;
reuse/rebind; incomplete evidence; duplicate terminalization; late/post-close
writers; owner release; full-byte color retention; failed copy-out/client outcome;
possible issuance; preservation failure; response/capsule mismatch and no retry.
Synthetic success never establishes a hardware triangle.

Reproduced defects were corrected before final qualification: invalidity suppressed
matched issuance/terminal facts; incomplete accepted evidence suppressed actual
terminal result and same-owner retired color; independent archive binding omitted
the exact approved-client pin; the compiled executable pin contained a transcription
error. Failing regressions and superseded build iterations remain preserved.
Final review has no unresolved critical/important findings within its offline scope.

Exact evidence: `tests/partial-retention-red.stderr`,
`tests/terminal-retention-red.stderr`, `tests/color-retention-red.stderr`,
`tests/independent-client-red.stderr`, `tests/producer-pin-red.stderr`;
final `tests/core-final-{native,ubsan,i386}-run.stdout`,
`tests/integration-final-{native,ubsan,i386}-run.stdout`,
`tests/reader-final.json`, `tests/reader-final.stderr`,
`reproducibility.json`, `image-reproducibility.json`, `implementation-review.json`
and `qualification.json`. The final checker count is 13; earlier 12-method logs
are retained intermediate evidence.

## Qualification reuse and PRE07

| Evidence/component | Disposition |
| --- | --- |
| Frozen geometry, request bytes, approved client and fixed response layout | REUSABLE, unchanged |
| Acceptance and retirement/readback algorithms | REUSABLE; passive integration separately qualified |
| Kernel/toolchain/target symbol table | REUSABLE; new module imports/CRCs checked |
| Existing archive, validator, file primitives and guard predicates | REUSABLE; supplemental reader/binding newly qualified |
| Old module/image hashes | INVALIDATED BY CHANGED BYTES for this new candidate; old artifacts remain valid history |
| Old boot/70-of-70, staging paths, module notes and first-owner guard pins | REQUIRES REBINDING and actual new first-owner observations |
| Gate B and client authority | Original decisions unchanged; new exact build/boot context requires binding, not automatic inheritance |
| PDS, Attempt03/05 and broad historical audits | NOT RELEVANT; not repeated |

The prior boot `89fc7306-6dac-4ab9-af62-360b7152ef22`, Build ID
`314e2f3b37195dc56df7c57dd938d78377a5ea8e` and 70/70 capture remain valid
historical evidence. They do not establish ownership or freshness of this changed
module pair. Current boot continuity is unobserved; a new candidate boot ID does
not exist in the evidence. No hot replacement is permitted. The old first-owner
checker has build-specific constants and cannot qualify this new pair unchanged.

PRE07 was evaluated once after final qualification: its offline producer and
preservation components are qualified; live PRE07 is blocked by the unstaged,
unbooted changed candidate and absent fresh guards. Required future facts include
both loaded module identities/Live state and hook order, actual UNUSED capsule,
zero prior call, ownership/health/boot continuity, prior-event quiescence and
continuous producer isolation, protected executable and exclusive evidence
destinations, privilege and STOCK recovery readiness. None is manufactured from
offline consistency. Actual target evidence paths remain NOT ESTABLISHED.

The exact prepared action remains
`MINI12-SGX535-REV121-FROZEN-32x32-SEQ1`: 32×32 ARGB8888, stride 128,
4096 bytes, white vertices (8,8), (24,8), (8,24). Future preservation retains the
complete 4268-byte response, original 4096-byte image, stdout/stderr, process
outcome, actual capsule and current-context evidence; then seals independently and
validates a COPY. Accepted bits 1/2/4 support TA completion/end-render/3D-memory-free
only with attributable producer evidence. Neither ledger nor pixels alone prove
hardware execution. Failure, HOLD, incomplete evidence or possible issuance means
preserve / STOP / NO RETRY. RAM retention does not guarantee survival of power loss.

**The next action requiring maintainer authorization is new-candidate staging,
first-owner boot and passive preparation/capture, with no SGX.** Once those facts
and exact-context bindings exist, the separate execution-authorization decision
can be evidence-complete. This task does not authorize or perform that live step.
No further offline implementation review request was created.

## Files and state preservation

New implementation/tests:
`kernel/sgx535_frozen/gma500_capsule_observer.{c,h}`;
`tools/psb-dri-re/frozen_capsule.{c,h}`;
`tools/psb-dri-re/frozen_capsule_evidence.py`;
`tools/psb-dri-re/test_frozen_capsule.c`;
`tools/psb-dri-re/test_frozen_capsule_service.c`;
`tools/psb-dri-re/test_frozen_capsule_evidence.py`.
Modified permitted sources:
`kernel/sgx535_frozen/gma500_fixed_backend.c`,
`kernel/sgx535_frozen/gma500_bo_owner.c`,
`tools/psb-dri-re/frozen_fixed_service.{c,h}`.
New documentation: implementation approval, final qualification and this report.
The authoritative `CODEX-HANDOFF-20261002.md` receives only the prospective update.
Build/image/test/review evidence stays outside the repository in the evidence directory.

Existing dirty/untracked authority records, source changes and historical evidence
are preserved. The new files and previously untracked handoff remain untracked;
no index changes, staging or commits occurred. `git-preservation.json` records
final status and the proposed repository-history set, with hunk review required
for files that already contained unrelated changes. Excluded entry/reporting and
restricted wrapper material was neither inspected nor modified.

Final state: **sgx_execution_authorized=false; SGX invocations=0; Mini 12 hardware
interactions=0; triangle=NOT ATTEMPTED.** TA/end-render/3D-memory-free remain
unobserved, readback NONE, no new HOLD or recovery state. The smallest remaining
boundary is authorization for the new candidate's live preparation, not another
provenance architecture exercise.
