# Phase 4.4 — Adversarial audit

2026-09-19. I reopened the Phase 4.3 conclusions against the primary snapshots and source records. The audit covered five inherited documentary PASS requirements, ten gate cells across the two candidate registers, 71 gma500 files, and the recorded source rechecks in `docs/phase4-program-data/`.

All documentary PASS claims survived only within their stated scope: software identity, offsets, access width, gma500 ownership, and mapping lifetime. None became evidence of physical read safety. No `UNKNOWN` became `PASS`, no blocker closed, and no candidate entered the whitelist.

The red-team review rejected these shortcuts: driver use does not prove safe reads; D0 or runtime active does not prove SGX clocks; equal offsets do not prove equal semantics; build targets do not identify silicon; and reboot or reset is not a demonstrated recovery path for a stalled CPU MMIO transaction.

The safe-register table was corrected where write-side effects had previously been treated as proof that reads were unsafe. Those read semantics are now `UNKNOWN`; the documented write effects remain unchanged. The rev116/rev121 difference is documented as proposal-versus-build configuration, not as conflicting physical measurements.
