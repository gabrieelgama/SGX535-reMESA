# CORE_ID / CORE_REVISION audit

The historical candidates are SGX-relative offsets `0x0010` (`CORE_ID`) and `0x0014` (`CORE_REVISION`), accessed as 32-bit values in the cited software. The sources establish names, offsets, width, and historical software use. They do not establish the physical Poulsbo register contract.

For both candidates, read-only semantics, absence of read side effects, valid power and clock state, reset requirements, revision coverage, locking, suspend/resume interaction, failure behavior, and recovery remain **UNKNOWN**. A `readl()` call or debug dump is evidence that software performed a read; it is not proof that the read is safe.

No source shows clear-on-read, acknowledge-on-read, FIFO, latch, or destructive behavior for these IDs. That absence is also not positive evidence of side-effect-free reads.

Conclusion: neither candidate is a SAFE-CANDIDATE and neither is authorized for Test Vector One.
