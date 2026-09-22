# Phase 6.2 baseline

Phase 6.2 starts from commit `16899a1` (Phase 6.1), not from a reconstructed memory of earlier phases.

| item | authoritative state |
|---|---|
| Gate A — passive probe | `GO` |
| Gate B — first controlled SGX MMIO read | `BLOCKED` |
| `CORE_ID` | `SAFE-CANDIDATE NO` |
| `CORE_REVISION` | `SAFE-CANDIDATE NO` |
| whitelist | `[]` |
| Test Vector One | not designed; not executed |
| hardware state modified | `NO` |

The real machine remains the Test Vector Zero target: Dell Inspiron 1210, `0000:00:02.0`, `8086:8108`, PCI revision `0x06`, Dell subsystem `1028:02b1`, running gma500. PCI revision `0x06` identifies the observed PCI function revision only. It does not identify an Intel stepping, an SGX core revision, or an applicable BRN.

The Phase 6.1 [formal gate reassessment](phase6-1-gate-reassessment.md) passed only identity, offsets, historical 32-bit access width, gma500 ownership, and mapping lifetime. It left read semantics, side effects, power, clocks, reset, revision/errata applicability, locking, PM exclusion, failure behaviour, and recovery unresolved.

Phase 6.2 is an evidence-acquisition and decision phase. It performs no hardware access and does not design Test Vector One.
