# First successful CORE_ID MMIO observation — operator report

The operator reports executing the installed, corrected fixed-register CORE_ID helper exactly once after its DMI-fix installation. The operator supplied this exact output line:

```text
CORE_ID=0x01130000
```

**HW-OBSERVED raw CORE_ID value: `0x01130000`**, with **operator-reported provenance**. Preserve these eight hex digits exactly; no byte swap, normalization or inferred revision is applied. The helper's installed v2 SHA-256 was verified immediately after installation as `dd3993db4835d1e51848ab2cc6d6a647bebd5e5d38c6cad6c3848584b307a20d` in [the preceding installation record](../MINI12-20260928-COREID-DMI-FIX-INSTALL-01/results.md). The operator's invocation timestamp, exit status, raw stderr, post-read target state and hash of the binary at invocation were not independently captured in this message.

The retained SGX535 header names `EUR_CR_CORE_ID` at SGX-relative offset `0x0010` and defines `ID`/`CONFIG` masks. Those field names and masks may be reported separately as source-defined layout; this single value does not establish SGX core revision, errata applicability, power/clock semantics, or general MMIO safety. Gate B remains **BLOCKED**, whitelist `[]`. CORE_REVISION remains untouched at this point. No agent-initiated MMIO occurred while recording this report.
