# Failure and recovery contract

No Poulsbo-specific public source recovered in Phase 6 defines CPU-visible behavior when the SGX aperture is accessed while the block is unpowered, clock-gated, reset, suspended, transitioning, or affected by an erratum. The available sources do not establish successful completion, a stale value, all-zero or all-one data, abort behavior, timeout, CPU stall, machine check, GPU hang, or system hang.

Generic PCI background cannot replace this missing platform contract. In particular, a PCI resource address and a normal PCI D-state do not prove the behavior of the internal SGX aperture.

Linux SysRq and pstore/ramoops documentation can support later diagnostic preparation ([P4-017](evidence-matrix.csv), [P4-018](evidence-matrix.csv)). They do not prove recovery from a failed CPU MMIO transaction. Neither `psb_spank()`, driver reload, unbind/rebind, suspend/resume, reboot, nor power cycling has been established as a recovery for the possible read-failure modes. None was attempted.

| question | status |
|---|---|
| CPU-visible failure behavior | UNKNOWN |
| timeout or hang boundary | UNKNOWN |
| documented recovery action | UNKNOWN |
| preconfigured diagnostic collection | INFERRED possible, platform-specific setup unverified |

Failure and recovery are separate gate requirements. A planned reboot is operational caution, not a safe-access guarantee.
