# Passive source guard — offline implementation, 2026-10-05

**Historical fail-closed build record.** The later autonomous offline implementation
supersedes its current continuation guidance with a retained startup-lifecycle provider:
[source-lifecycle qualification and proof](source-lifecycle-quiescence-20261005.md).
The old builds/results and the original body below remain preserved; statements about
UNPROVEN/no new image/SGXSOURCE1 apply to those earlier artifacts, not the new candidate.


[Qualification record](source-quiescence-guard-qualification-20261005.json). Evidence:
`/home/gama/sgx535-offline/phase8-source-quiescence-20261005T055127Z/`.
Authority is the current maintainer instruction permitting offline design,
implementation and focused qualification only. No target contact occurred.

## Actual semantics and unresolved hardware premise

The guard runs in `sgx535_gma500_fixed_backend_begin` after the powered/fresh
owner preconditions and before publishing an active backend/IRQ owner or admitting
the capsule. It reads STATUS1/2, 2D FIFO and 2D busy using existing MMIO accessors
and the IRQ lock, and incorporates software pending/fire state. It neither waits
for a clean sample nor submits, acknowledges, clears or resets anything.

Non-benign pending bits, busy state, unreadable state, competing activity, lost
coverage and reuse all prevent admission. A one-use source interval starts before
the reads, so already-active work and activity overlapping the sample cannot be
missed merely because it finishes before admission. Failure reasons are monotonic.
An unsuccessful boundary returns `-EAGAIN`; attempted guard reuse returns `-EBUSY`.
No source guard reset/rearm API exists. An entered ioctl remains a possibly consumed
attempt under existing no-retry policy, even when backend admission is refused.

**A clean raw snapshot remains UNKNOWN.** The available qualified facts do not
establish an idle/drain guarantee for delayed TA/3D work. A latched/masked or delayed
prior event is not excluded by a quiet trace, UNUSED capsule or 2D-idle state.
Both native adapters explicitly keep `delayed_excluded=0`; there is no parameter,
proc write or user-supplied proof channel. Thus **no native successful source
boundary is available in these builds**. Tests of an admissible core boundary use
an explicitly synthetic independent delayed-exclusion premise, not hardware facts.

## Continuous isolation connection

The small `antix-source-guard.patch` attaches the covered driver instance and adds
balanced direct observation taps around **existing** `psbfb_2d_submit` and
`psb_spank`. It does not add a submission/reset call, alter command bytes, suppress
normal work or claim that ordinary 2D completion is triangle completion. Direct
taps cover the inlined submission helper without a kprobe symbol assumption or
trace-buffer transport loss. Counter overflow/unmatched exit invalidates evidence.

The source and capsule adapters share a spinlock. An admissible boundary would be
bound to one backend; interference makes capsule evidence invalid, and interval
closure occurs at the observed public ioctl exit. A crashed/unclosed call supplies
no closed interval. This is covered-path continuity, not universal protection from
privileged raw hardware access or arbitrary kernel changes. Native timing and
physical behavior are not qualified by offline tests. No actual boundary has been
established, so these taps do not currently discharge PRE07 isolation.

## Retention and qualification

`/proc/sgx535_source_guard` is a new root0400 **read-only** bounded textual snapshot:
`SGXSOURCE1`, state, monotonic reason mask, active producer count, driver attachment,
and `delayed_exclusion=UNPROVEN`. Opening it only snapshots retained RAM; it does
not read hardware or initiate a guard/operation. States are UNUSED=0, OBSERVING=1,
BOUNDARY=2, CLOSED=3, BLOCKED=4. Reason masks are declared in
`frozen_source_guard.h`. UNUSED is not a quiescence certificate. No UUID, exported
sequence, raw STATUS journal, triangle-UAPI change or approved-client change exists.
Saved evidence still needs exact producer/boot binding and protected durable
preservation; reader/copy failure cannot create a valid witness.

- Native and UBSan policy checks: **58/58 PASS each**.
- Adapter/source integration checks: **5/5 PASS**, zero skips, including four
  cases compiling the actual backend boundary with fake read-only MMIO and no
  write/ack/reset/submission APIs. Pending, busy, missing mapping and clean-but-
  unproven sources all stop before backend publication/admission.
- Both changed modules build in the existing i386/kernel environment. ELF32/i386,
  vermagic and import/CRC coverage pass. No full historical test suite was rerun.

These are **fail-closed implementation checks**, not physical quiescence proof or
execution qualification. No new image was built while the missing premise remains.
The qualified old image and running boot remain unchanged. New build identities
are recorded in the qualification JSON; do not substitute them under old bindings.

## Next boundary

The remaining prerequisite is a permitted, hardware-backed passive source
idle/drain or causal guarantee that excludes delayed prior work and remains
connected to the isolation interval. It cannot be manufactured by submission,
completion clearing or reset. No such provider has been justified or implemented.
Qualify that exact premise/provider offline before producing a new execution
image; any later live preparation needs its own applicable authorization.

**PRE07 BLOCKED; READY FOR EXECUTION AUTHORIZATION: NO;
sgx_execution_authorized=false; SGX invocations=0; hardware interactions=0;
triangle=NOT ATTEMPTED.** The current running candidate is unchanged and lacks
the new guard; it must not be hot-replaced.
