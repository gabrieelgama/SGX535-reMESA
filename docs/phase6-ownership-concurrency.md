# Ownership and concurrency

The current gma500 source creates `dev_priv->sgx_reg` during driver load and destroys it during teardown. It owns the PCI function and this mapping ([P4-007](evidence-matrix.csv)). A separate PCI driver, userspace BAR mapping, or `/dev/mem` access would create an additional owner and remains prohibited.

The antiX source package's gma500 files were previously compared with upstream v5.10.240, but identity with the installed module is still `UNVERIFIED`. The observed driver binding is gma500, not proof that every local lifecycle detail is identical on the running kernel.

`irqmask_lock` protects the IRQ mask path. The source does not identify it as a general SGX-MMIO exclusion lock. Existing lifecycle code also has probe/remove, KMS, workqueue, runtime-PM, system suspend/resume, and IRQ interleavings. No gma500-owned execution context or locking protocol was found that proves exclusion against all of them for a standalone identification read.

| requirement | status |
|---|---|
| single mapping owner | PASS: gma500 |
| mapping lifetime described | PASS: gma500 driver lifetime |
| exclusion from removal | UNKNOWN |
| exclusion from suspend/resume | UNKNOWN |
| exclusion from PM state transitions | UNKNOWN |
| IRQ/KMS interaction protocol | UNKNOWN |
| correct in-driver observation context | UNKNOWN |

An observation must remain within existing gma500 ownership if a later phase authorizes one. Phase 6 does not propose an implementation or a new lock.
