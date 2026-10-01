# First SGX MMIO read experiment: pre-execution record

**Gate B: BLOCKED. Whitelist: `[]`. Experiment status: OPERATOR-AUTHORIZED despite unresolved Gate B requirements; NOT EXECUTED.** Authorization was for exactly one SGX identification-register read attempt, with no retry. This session is separate from H0 and P1. No MMIO attempt occurred.

The intended candidate is `CORE_ID`, because retained SGX535 register definitions label it as the core identifier. Authoritative source and installed-module mapping evidence is summarized in [preflight-decision](preflight-decision.md). A target preflight may check identity, current OS state, access permissions and tooling without opening MMIO. The actual read is conditional on verifying a mechanism that performs one 32-bit transaction and no write or other MMIO read. If that cannot be established, stop without executing.

The pinned-SSH preflight succeeded but showed no read access to BAR0 `resource0` or `/dev/mem` for the existing unprivileged SSH user. The experiment therefore stopped before opening either resource. See [results](results.md), [manifest](manifest.json) and exact preflight stdout/stderr/metadata under `raw/`.
