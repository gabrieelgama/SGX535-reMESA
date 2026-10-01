# Kernel-health guard qualification

2026-10-01. [Evidence](artifacts/kernel-health-qualification-20261001/README.md). The [cycle01 failure](first-load-cycle-01-result.md) remains unchanged.

PASS OFFLINE: the corrected operational matcher accepts the retained stock
baseline, reports the bounded diagnostics, and rejects fault-bearing mutations.
278 tests passed with zero skips, along with 14 boot-analysis tests, three UBSan
harnesses, the generator and candidate CRC checks. The deterministic dry hash
was unchanged. The intentional PARTIAL refusal also passed its check.

The old case-insensitive matcher found BUG: inside BL bug:. The retained source
emits BL bug with dev_err when the computed backlight maximum is zero. It does
not invoke BUG(). The correction checks report/word boundaries and typed fields;
it does not use grep-v.

The four known PowerButton diagnostics must match exact stock forms and bounded
counts. They are not a general warning waiver. New, changed or repeated forms,
fatal warnings, BUG/oops/panic/traces/lockups and new graphics faults are
rejected. The complete baseline receipt is preserved. A baseline cannot make a
real fault acceptable.

At this checkpoint, the next step was to restart fresh preflight from the
beginning under the separately accepted non-SGX first-load cycle. Staging still
required every fresh check to pass. Experimental image/candidate identities
remained pinned, with no SGX permission. Gate B BLOCKED; whitelist[]. Live first
owner/recovery remained UNKNOWN.