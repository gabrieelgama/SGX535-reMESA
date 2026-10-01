# Gate B reassessment after passive target pass P1

The [H0 sixteen-row assessment](../MINI12-20260927-H0/gate-b-reassessment.md) is retained. This table adds only the [P1 OS-visible observations](results.md) and the earlier [offline row 12](../../phase7/7.0-installed-lifetime.md) and [row 14](../../phase7/7.0-installed-pm-exclusion.md) checks. **Gate B BLOCKED; whitelist `[]`; no SGX MMIO observed or authorized.** `PASS_SCOPED` never means the first identification read is safe.

| # | Requirement | Current result after P1 | Exact blocker before first SGX MMIO read |
| ---: | --- | --- | --- |
| 01 | Exact identity | **PASS_SCOPED:** H0 DMI/CPU/PCI identity reverified over pinned SSH. | No additional PCI identity fact for this row; physical core revision is row 09. |
| 02 | Exact SGX-relative offset | **PASS_SCOPED:** BAR text unchanged; installed module mapping evidence retained. | Revision-specific access semantics are rows 04–10. |
| 03 | Access width | Historical/software 32-bit calls remain supported; **target applicability UNKNOWN**. | Applicable 32-bit access contract for these exact registers and core revision. |
| 04 | Explicit read semantics | **UNKNOWN.** P1 contains no register access attributes. | Applicable `CORE_ID`/`CORE_REVISION` read definition. |
| 05 | No destructive read side effects | **UNKNOWN.** No read was tried. | Positive side-effect-free contract for each exact register. |
| 06 | Required power state | **UNKNOWN:** PCI `active`, `control=on`, `enable=1` are current OS text, not internal SGX power. | SGX power prerequisite and independently safe pre-read predicate. |
| 07 | Required clock state | **UNKNOWN.** No passive clock-domain state exposed in this bounded pass. | Required interface/register clock and safe pre-read predicate. |
| 08 | Required reset state | **UNKNOWN.** Module `live` and display presence do not prove SGX reset/init state. | Revision-applicable reset/access ordering and current-state proof. |
| 09 | Physical core revision applicability | **UNKNOWN:** PCI revision is again `0x06`, not SGX core revision. | External core-revision evidence or safety proof across every possible target revision. |
| 10 | Applicable errata | **UNKNOWN.** | Errata/BRN set tied to a qualified core revision and identification-read constraints. |
| 11 | Single owner of SGX mapping | **PASS_SCOPED:** current `gma500` binding and `gma500_gfx` module note match H0. | No new mapping owner fact; exclusion remains row 13. |
| 12 | Mapping lifetime | Installed normal map/unmap sequence is statically corroborated; module `live`/refcount `2` is a snapshot. **Observer hold UNKNOWN.** | Exact driver-owned observer placement and pin through completion, teardown and failure paths. |
| 13 | Locking/concurrency exclusion | **UNKNOWN:** Xorg present; IRQ 16 shared with eth0; unprivileged fd inventory incomplete. | Exclude IRQ, 2D, fbdev/KMS, clients, remove and other SGX accesses for the whole observation. |
| 14 | Suspend/resume exclusion | **UNKNOWN:** current PCI runtime `active`/`control=on`; offline source/ELF check found no common lock or approved observer PM hold. | Reviewed driver-owned PM reference and protocol against runtime/system suspend, resume and D-state change across the whole read. |
| 15 | Bounded failure behavior | **UNKNOWN.** Passive OS state says nothing about a gated or stalled CPU load. | Target-applicable CPU/chipset MMIO completion/fault/stall contract. |
| 16 | Adequate recovery | **UNKNOWN:** SysRq text unchanged, pstore unmounted/empty, watchdog class empty; no recovery test. | Separately validated recovery and evidence-preservation path for credible failure classes, including machine stall, with acceptable filesystem risk. |

**Narrow-read gate:** not yet closer to PASS in the sense of satisfying a blocking hardware access contract. P1 improves confidence that the same software owner and presently active PCI/display stack are being examined, and exposes current concurrency and recovery limits. It cannot promote OS-visible text to an SGX hardware read-safety fact. The first `CORE_ID`/`CORE_REVISION` read remains prohibited pending rows 03–10 and 12–16 in their exact target-qualified scope.
