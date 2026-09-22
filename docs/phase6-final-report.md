# Phase 6 final report

## Result

Phase 6 began with Phase 5 Gate B `BLOCKED` and ends with it still `BLOCKED`. `CORE_ID` and `CORE_REVISION` remain `SAFE-CANDIDATE NO`; the whitelist is `[]`. No active hardware operation occurred, and Test Vector One was neither designed nor executed.

## Evidence acquired

The source review made the `psb_get_core_freq()` question more precise. In the current target-source line it is `gma_get_core_freq()`: it writes a root-bridge PCI configuration selector, reads a value, then maps it to a frequency. The Poulsbo source consumes `dev_priv->core_freq` for backlight PWM calculation. This establishes a historical display-timing helper and a prohibited active PCI transaction, not the SGX execution clock or identification-register accessibility.

Intel document `364236` is confirmed only as the *Intel System Controller HUB External Design Specification* and as historically available through privileged Intel RDC access. No legitimate contents were recovered. The Imagination trail confirms a private support path for detailed SGX535 material, but not the contents of a manual. MeeGo/Moblin research found historically relevant packaging/source trails but no public Poulsbo SGX clock, power, reset, or read-access contract. Later Cedar Trail, Moorestown, and other platform material is not Poulsbo evidence.

## Access-contract answers

1. **Phase 5 starting Gate B:** `BLOCKED`.
2. **New primary evidence:** local GPL Linux source establishes the exact `gma_get_core_freq()` transaction and its backlight-PWM consumer; no new primary hardware manual was recovered.
3. **Intel `364236`:** title and restricted historical access route are established.
4. **Contents from `364236`:** no; only metadata and references.
5. **Imagination trail:** public posts show a support boundary, not a register contract.
6. **MeeGo:** historically relevant deployment and package/source traces; no public Poulsbo SGX access, clock, power, or reset contract recovered.
7. **Moblin:** historically relevant PSB/PowerVR package and source lineage; no public Poulsbo SGX access, clock, power, or reset contract recovered.
8. **Poulsbo SGX clock/power/reset from MeeGo/Moblin:** no.
9. **`psb_get_core_freq()`:** a host-bridge PCI selector/read helper whose target-source value feeds Poulsbo backlight PWM calculation.
10. **Proven SGX execution clock:** no.
11. **Required SGX clock-enable state:** UNKNOWN.
12. **Required SGX power state:** UNKNOWN.
13. **Required reset state:** UNKNOWN.
14. **Dell physical SGX revision:** UNKNOWN.
15. **PCI revision `0x06` to SGX revision mapping:** none found.
16. **Applicable BRNs:** UNKNOWN.
17. **`CORE_ID` read semantics:** UNKNOWN.
18. **`CORE_REVISION` read semantics:** UNKNOWN.
19. **Absence of read side effects:** not established for either register.
20. **Locking/concurrency:** ownership and mapping lifetime are known; a complete observation protocol is UNKNOWN.
21. **Suspend/resume interaction:** UNKNOWN.
22. **Failure behavior:** UNKNOWN.
23. **Recovery:** UNKNOWN.

## Gate and next phase

Requirements 04–10 and 13–16 in [the formal gate](phase6-gate.md) remain `UNKNOWN` for both registers. Therefore neither candidate is safe, the whitelist remains empty, and Gate B is `BLOCKED`.

The single most valuable missing artifact is an authorized SGX535/Poulsbo register and errata reference defining both IDs' access attributes and side effects, their required power/clock/reset state, revision/BRN coverage, unavailable-aperture behavior, and recovery. If Gate B remains blocked, Phase 7 should continue evidence acquisition around that artifact or an official equivalent. If a later phase obtains and verifies a complete contract, it may separately review a bounded Test Vector One design; it must not execute it without new authorization.

24. **Requirements still blocked or unknown:** gate rows 04–10 and 13–16 for both registers.
25. **`CORE_ID` SAFE-CANDIDATE:** NO.
26. **`CORE_REVISION` SAFE-CANDIDATE:** NO.
27. **Final whitelist:** `[]`.
28. **Final Gate B:** `BLOCKED`.
29. **Test Vector One design:** no; the blocked record is [phase6-test-vector-one-skipped.md](phase6-test-vector-one-skipped.md).
30. **Test Vector One execution:** NO.
31. **Hardware state modified:** NO.
32. **Greatest-confidence artifact:** the authorized SGX535/Poulsbo register and errata reference described above.
33. **Phase 7 if Gate B remains blocked:** continue evidence acquisition; do not perform an observation.
34. **Phase 7 if Gate B becomes GO:** separately review a high-level, bounded, read-only Test Vector One design before any implementation or execution.
