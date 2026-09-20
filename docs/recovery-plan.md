# Recovery plan for future experiments

This plan does not authorize active access. **RECOMMENDATION** marks operational preparation; **CONFIRMED** describes only documented Linux or gma500 behavior.

## Preparation

1. **RECOMMENDATION:** keep a known-working kernel boot entry and verify before testing that it boots without intervention.
2. **RECOMMENDATION:** use SSH from another host and, if available, a serial console. Recovery must not depend on the display that shares the PCI function.
3. **RECOMMENDATION:** stage the exact code, baseline report, kernel/config hashes, and collection commands before starting. Keep test data away from the only important writable filesystem.
4. **RECOMMENDATION:** sync filesystems, stop workloads, and avoid unnecessary writes. Where practical, use a disposable system or read-only root with remote logging.
5. **RECOMMENDATION:** separately verify remote reboot, SysRq, and the known-working recovery-kernel entry before any active experiment.

## Baseline logs

Collect these without MMIO:

```text
uname -a
cat /proc/cmdline
cat /proc/version
python3 tools/sgx535-probe/sgx535_probe.py --json
journalctl -k -b
dmesg --ctime
```

Send logs to another host when possible. Test Vector Zero does not require root and does not read debugfs.

## Recovery channels

**CONFIRMED — P4-017:** with `CONFIG_MAGIC_SYSRQ`, `/proc/sys/kernel/sysrq` controls the enabled keyboard operations. SysRq can dump tasks (`t`), blocked tasks (`w`), and locks (`d`); sync (`s`); remount read-only (`u`); and reboot immediately (`b`). `b` does not sync or unmount filesystems (`Documentation/admin-guide/sysrq.rst:9-46,93-165`).

If the machine still responds, first collect `d`, `w`, `t`, and remote console logs; terminate the experiment if userspace works; then sync and remount read-only before reboot. Use an immediate reboot only as a last resort.

**CONFIRMED — P4-018:** when `ramoops/pstore` is already configured, persistent records appear as `dmesg-ramoops-N`; persistent ftrace can also help with hangs (`Documentation/admin-guide/ramoops.rst:145-167`). Configure and verify this before an experiment, outside the SGX harness.

## Failure classes

| class | operational signs | collection | recovery assumption |
|---|---|---|---|
| GPU hang | kernel, SSH, and display respond; SGX stops progressing or faults | journal/dmesg, process state, later pstore | stop the experiment; planned reboot |
| display hang | screen freezes; SSH/serial and kernel respond | remote logs, tasks, locks | do not try SGX reset; clean remote reboot |
| kernel hang | SSH and userspace fail; SysRq/serial still respond | SysRq `d/w/t`, console, pstore | sync/remount/reboot if possible |
| machine hang | display, SSH, serial, and SysRq all fail | external watchdog status and post-reboot pstore only | power-cycle; boot known kernel |

These are screening criteria, not diagnoses. A frozen display does not prove that SGX caused the failure.

## Watchdogs and reset

No SGX watchdog or recovery mechanism was found in the current gma500 inventory. This does not prove that the platform lacks a watchdog; its state is **UNKNOWN**. `psb_spank()` touches BIF, DPM, TA, USE, ISP, TSP, and 2D (`psb_drv.c:97-125`), so it is not an approved recovery operation. An SGX reset cannot be assumed to recover the display, kernel, or machine.

## After reboot

Before another test:

1. save `journalctl -k -b -1` when persistent journald is available;
2. copy `/sys/fs/pstore/*` without deleting the first collection;
3. record why and how the system rebooted or was power-cycled;
4. rerun Test Vector Zero and compare PCI, BAR, IRQ, driver, and PM data;
5. verify filesystem integrity and the expected kernel;
6. archive stdout/stderr, the harness commit, kernel config, and remote logs.
