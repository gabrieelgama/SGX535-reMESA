# Power and reset contract

The available sources describe historical sequencing, not a complete hardware contract for an identification read.

The Poulsbo DDK's `SUPPORT_DRI_DRM_EXT` path acquires display use before graphics use and releases graphics before display ([P3-036](evidence-matrix.csv)). The current gma500 Poulsbo callbacks `psb_power_up()` and `psb_power_down()` are stubs, while generic gma500 runtime-PM support is explicitly limited ([P4-012](evidence-matrix.csv)). On suspend, gma500 disables PCI and selects D3hot; resume restores PCI, GTT/GEM/display state, and IRQ handling ([P4-013](evidence-matrix.csv)).

These facts establish driver order and lifecycle behavior. They do not establish that the SGX island is powered, that its register interface is clocked, or that it is out of reset while gma500 is bound, while display is active, or while runtime status is `active`.

The DDK explicitly warns against a general SGX register dump when SGX is not powered ([P4-020](evidence-matrix.csv)). This is relevant negative evidence: it shows that at least historical software recognized a power-sensitive read boundary. It does not identify the minimum state, a safe check, `CORE_ID`/`CORE_REVISION` exceptions, or the result of a read outside that state.

## Contract status

| prerequisite | status | missing evidence |
|---|---|---|
| PCI state | CONFIRMED as a Linux-visible state | relationship to internal SGX accessibility |
| runtime PM `active` | CONFIRMED measurement in TVZ-001 | relationship to SGX power or clocks |
| SGX power | UNKNOWN | minimum state and passive verification method |
| SGX/register-interface clock | UNKNOWN | required domain, gate behavior, and proof method |
| reset asserted/deasserted | UNKNOWN | accessibility and ordering requirements |
| firmware, BIF, MMU, or microkernel prerequisite | UNKNOWN | hardware requirement, if any, rather than historical driver order |

No public evidence found in Phase 6 proves an identification register can be read before GTT, SGX MMU, firmware, microkernel, PDS, or USSE initialization. It also does not prove that any of those stages are prerequisites. Both directions remain `UNKNOWN`.
