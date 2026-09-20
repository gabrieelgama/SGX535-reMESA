# Phase 5 ownership, power, and recovery review

## Installed environment

TVZ-001 records `5.10.240-antix.1-486-smp`, i686, gma500, `card0`, IRQ 16, and runtime status `active`. The recovered antiX source package's 71 gma500 files match upstream Linux v5.10.240, but the installed module and kernel image have not been hashed. Exact equivalence to the running code remains `UNVERIFIED`.

## Ownership and concurrency

The available gma500 source owns the PCI function and `dev_priv->sgx_reg`; the mapping exists during the driver's bound lifetime. This supports the existing `PASS` results for ownership and mapping lifetime. It does not provide a universal SGX lock.

A future read would need a gma500-owned execution point with proven exclusion against remove and system/runtime suspend, plus a defined relationship to the shared IRQ and KMS paths. `irqmask_lock` only protects the paths in which it is used. No complete exclusion protocol was recovered, so locking and suspend/resume remain `UNKNOWN`.

A second PCI owner, userspace BAR mapping, gma500 unbind/rebind, and module unload/reload remain prohibited.

## Power, clocks, and reset

`runtime_status=active` describes PM-core state. It does not prove that the SGX power island is on, the register-interface clock is running, or SGX is out of reset. The historical DDK warning against dumping registers while SGX is unpowered makes that distinction operationally relevant.

No recovered source defines the minimum state for either ID register or a passive way to verify it. Power, clock, and reset requirements remain `UNKNOWN`.

## Failure and recovery

No applicable source defines what a CPU read does if the SGX block is unavailable: completion with a value, abort, fault, timeout, CPU stall, or fabric hang all remain outside the recovered contract. The repository has a host recovery plan, but reboot, SysRq, pstore, `psb_spank()`, or module cycling does not prove recoverability of an unknown MMIO transaction.

Failure behavior and recovery remain `UNKNOWN`. They cannot be used as safety arguments for Test Vector One.
