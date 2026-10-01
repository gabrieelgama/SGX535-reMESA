# Recovery protocol audit for the Mini 12

**State: OPERATOR FALLBACK REPORTED / RECOVERY NOT VALIDATED. No recovery action was exercised.** This document refines the existing [project recovery plan](../../recovery-plan.md) for the freshly identified Dell. It is not permission to read SGX MMIO, reset the device, submit a command, reboot, or manipulate power. Gate B requirement 16 remains UNKNOWN.

## Passive facts and their limits

The [hashed capture](raw/baseline/recovery_capabilities.stdout) reports kernel config `CONFIG_MAGIC_SYSRQ=y`, `CONFIG_PSTORE=y`, `CONFIG_PSTORE_RAM=m`, and `CONFIG_WATCHDOG=y`; runtime `/proc/sys/kernel/sysrq` is `438`. `/sys/fs/pstore` is empty and not listed as mounted; `/sys/class/watchdog` is empty. These are **HW-OBSERVED text/interface facts**, not proof that SysRq reaches the machine during a stall, that pstore records survive, or that a watchdog will reset it. No SGX-specific recovery primitive is approved. SSH was available during a healthy-state capture; availability during display, kernel, or CPU-MMIO failure is UNKNOWN.

The operator reports physical access to the Mini 12's keyboard and power button, including forced power-off by holding the button, last-resort manual removal/reconnection of external power, and the ability to boot the current known-working Debian/antiX installation afterward. The battery is reported non-functional. The operator reports **no serial console, hardware watchdog, or remotely switched power**; SSH is available only while the machine/network remains responsive. This is **USER-REPORTED operational capability**, not an exercised or HW-OBSERVED recovery result. The operator explicitly prohibited a destructive recovery test in this pass. A manual power cycle is therefore a contingency with possible filesystem/data loss, not a verified guarantee. The target and capture-host clocks have an observed offset; neither has externally verified UTC. Use local capture order and raw logs rather than comparing absolute timestamps across hosts without correlation.

The [operator-provided kernel-log snapshot](raw/operator-dmesg-mini12.txt) reports `ACPI Error: Could not enable PowerButton event`, a warning for the fixed event, and failure of both `button` and `tiny-power-button` probes around boot seconds 19.4–19.5. This does not prove the physical long-hold cutoff fails, but it prevents treating a normal short press as a verified clean-shutdown channel. The log's generation command/time were not observed by this session; its raw bytes and hash are in the [manifest](baseline-manifest.json).

## Preconditions for any future active experiment

1. Gate B passes for the exact candidate and operation; no requirement may be assumed from healthy SSH or PCI runtime `active`.
2. The physical operator is present and prepared to use the keyboard/power button without relying on the display or SSH. This capability is reported but has not been validated against a hang; no serial or switched-power fallback exists.
3. Use the captured operator-provided kernel-log snapshot as a qualified baseline, then establish an external log sink or tested persistent capture for a future experiment. Direct unprivileged and exactly approved passwordless `dmesg` attempts both failed; pstore was not mounted.
4. Protect filesystem data and document the precise failure-class recovery action. A reboot, SysRq or cold power cycle must be separately authorized and tested outside the SGX experiment. `psb_spank()` and undocumented SGX reset sequences are prohibited.
5. The eventual experiment protocol must define its own finite duration, expected progress, abort trigger and one-attempt rule. A userspace timeout does **not** bound a stalled CPU MMIO transaction.

## Conditional response ladder — not exercised

| Observed failure class | Preserve evidence first | Conditional response | Current qualification |
| --- | --- | --- | --- |
| GPU task stops but OS/SSH responds | capture kernel log, task state, interrupt delta and approved status only | stop the task; consider a separately approved clean reboot | no task has run; kernel-log access pending |
| Display freezes but SSH responds | save remote logs and process state; do not infer SGX cause | clean remote reboot only under a separately tested/authorized path | SSH-after-display-failure untested |
| OS/SSH stalls but independent console responds | capture console/SysRq diagnostics if actually available | sync/remount/reboot only after that route is validated | console and SysRq delivery unverified; mask `438` alone is insufficient |
| Entire machine stops responding | operator observation and any external logs already captured | operator-reported hold-power-button force-off and boot current known-working installation, only under a later separately authorized protocol | no serial/watchdog/switched power; forced-off outcome and filesystem integrity unverified |

After any failure, **do not retry immediately**. Preserve logs and exact last operation, identify whether the failure was a refusal, timeout, driver fault or machine hang, then compare the next passive identity/baseline with this session. No recovery path is marked PASS merely because a facility appears in config or because a hypothetical power cycle exists.

## Gate decision

The reported physical fallback improves operational planning, but no recovery validation, independent failure observation channel or target-qualified CPU-MMIO completion bound has been established. Requirement 16 remains **UNKNOWN** and Gate B remains **BLOCKED**, whitelist `[]`. The [individual gate audit](gate-b-reassessment.md) lists the least invasive evidence needed for each other requirement. No destructive recovery test will be performed in this pass.
