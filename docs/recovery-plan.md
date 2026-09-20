# Recovery plan for future experiments

This plan does not authorize active access. Items marked **RECOMMENDATION** are
operational procedure; `CONFIRMED` describes only documented behavior
do Linux ou do gma500.

## Preparation outside the target machine

1. **RECOMMENDATION:** maintain a boot entry with a known kernel as
functional and confirm, before the test, that it starts without intervention.
2. **RECOMMENDATION:** use SSH from another host and, if available, console
serial. Not dependent on the display that shares the PCI function.
3. **RECOMMENDATION:** copy code, initial report, hash of kernel/config and
collection commands before starting. Keep test data out of the only one
important writable filesystem.
4. **RECOMMENDATION:** sincronizar filesystems, encerrar workloads e evitar
non-essential recordings. If the installation allows, run from a system
disposable or root read-only with remote logs.
5. **RECOMMENDATION:** validar separadamente que reboot remoto, SysRq e a entrada
recovery kernel work. Not finding this out after a hang.

## Prior logging

Register, without doing MMIO:

```text
uname -a
cat /proc/cmdline
cat /proc/version
python3 tools/sgx535-probe/sgx535_probe.py --json
journalctl -k -b
dmesg --ctime
```

Redirect the capture to another host when possible. Test Vector Zero does not
requires root and does not read debugfs.

## Recovery channels

**CONFIRMED — P4-017:** with `CONFIG_MAGIC_SYSRQ`, `/proc/sys/kernel/sysrq`
controls keyboard operations. SysRq can dump tasks (`t`), tasks
bloqueadas (`w`), locks (`d`), sincronizar (`s`), remontar read-only (`u`) e
reboot immediately (`b`). The reboot `b` does not synchronize or unmount
filesystems (`Documentation/admin-guide/sysrq.rst:9-46,93-165`).

Recommended future sequence, only if the machine still responds:

1. coletar `d`, `w`, `t` e logs por SSH/console;
2. terminate the experiment if userspace still responds;
3. synchronize and remount read-only (`s`, `u`) before reboot;
4. use immediate reboot only as a last resort.

**CONFIRMED — P4-018:** when `ramoops/pstore` is already configured, records
persistent can be read as `dmesg-ramoops-N`; persistent ftrace as well
pode ajudar em hangs (`Documentation/admin-guide/ramoops.rst:145-167`). Sua
Configuration is a task prior to the experiment, not part of the SGX harness.

## Classification of failures

| class | operational signs | possible collection | expected recovery |
|---|---|---|---|
| GPU hang | kernel/SSH and display respond; SGX does not progress or faults | journal/dmesg, process state, pstore afterwards | end experiment; planned reboot |
| display hang | screen for; SSH/serial and kernel respond | remote logs, tasks/locks | do not try SGX reset; clean remote reboot |
| kernel hang | SSH and userspace param; SysRq/serial still respond | SysRq `d/w/t`, console, pstore | sync/remount/reboot if possible |
| machine hang | no display, SSH, serial or SysRq | only watchdog externo/pstore post-reboot | power-cycle; boot into known kernel |

These are screening criteria, not automatic diagnoses. A static display
does not prove that SGX caused the failure.

## Watchdogs e reset

The inventory did not find a watchdog/recovery SGX on the current gma500. This does not
evidence of the platform's watchdog absence. Its state is **UNKNOWN**. The reset
`psb_spank()` toca BIF, DPM, TA, USE, ISP, TSP e 2D
(`psb_drv.c:97-125`), therefore it is not an approved recovery mechanism. Never
to assume that 'reset SGX' recovers display, kernel, or machine.

## After reboot

Before starting a new test:

1. save `journalctl -k -b -1` when persistent journald exists;
2. copy `/sys/fs/pstore/*` without deleting it during the first collection;
3. record reason and method of reboot/power-cycle;
4. coletar Test Vector Zero novamente e comparar PCI/BAR/IRQ/driver/PM;
5. confirmar integridade de filesystem e boot no kernel esperado;
6. arquivar stdout/stderr, commit do harness, kernel config e logs remotos.

