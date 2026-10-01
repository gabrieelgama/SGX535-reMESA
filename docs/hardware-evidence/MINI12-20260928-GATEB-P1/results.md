# P1 results: bounded passive Gate B target observations

**Decision: Gate B remains BLOCKED; `CORE_ID` and `CORE_REVISION` SAFE-CANDIDATE NO; whitelist `[]`.** This pass adds OS-visible target observations. It does not observe SGX register contents or establish safe SGX register access. Exact captures and hashes are in [the manifest](manifest.json); the [row reassessment](gate-b-passive-reassessment.md) preserves each obligation.

## Target re-verification and timing

The preflight ran through the retained pinned SSH host key as unprivileged `gama` and matched H0's Dell Inc. Inspiron 1210, board `0X605H`, BIOS A02, i686 Atom Z520, PCI `0000:00:02.0` `8086:8108`, PCI revision `0x06`, subsystem `1028:02b1`. This re-verifies endpoint continuity at the documented DMI/CPU/PCI level. The PCI revision is **not** the physical SGX core revision.

The host recorded identity execution `2026-09-28T02:53:55.232036+00:00` to `02:53:56.534578+00:00`; the target emitted its own UTC-formatted `00:44:20.428521+00:00` to `00:44:20.447595+00:00`. The snapshot and follow-up have their own host and target start/end values in `raw/*.meta.json` and stdout. The apparent host-target offset remains about 2 h 9 min 35 s, consistent with H0's earlier offset; this does not identify which clock, if either, is accurate. Capture order and raw hashes are the reliable chronology.

## New OS-visible observations compared with H0

| Gate relevance | P1 observation | Comparison and limit |
| --- | --- | --- |
| PCI owner and software state | `0000:00:02.0` remains `0x8086:0x8108`, class `0x030000`, enabled `1`, IRQ `16`, bound to `gma500` and its `gma500_gfx` module. The BAR text is byte-for-byte equal to H0's PCI resource text. | Refreshes scoped rows 01, 02 and 11. Reading text `resource` did not open any `resourceN` mapping. |
| Loaded module | `gma500_gfx` reports `live`, refcount `2`, no module holders, and loaded Build-ID note SHA-256 `484f90964d0c36b50256e42b1bc1cd8fb905fad8476b5a1bf5e549dc178792d7`, the same 36-byte note H0 captured. `/proc/modules` also lists `drm_kms_helper` and `drm`. | Supports continuity of the installed/loaded software identity; refcount `2` is a snapshot, not an observer lifetime guard. Module `taint` text is `E`; no behavioral inference is drawn. |
| PCI and PM | `power/control=on`, `power/runtime_status=active`, `power/runtime_active_time=15371718`, `power/runtime_suspended_time=0`, `d3cold_allowed=1`. H0 had `active` and suspended time `0`; P1 adds `control=on`. `power_state` and `power/runtime_usage` are absent; `power/autosuspend_delay_ms` returned EIO. | Rows 06 and 14 gain current OS PM text, not proof of internal SGX power/clock/reset state, present PCI D-state, or a race-free hold across a future read. `/sys/power/state` lists `freeze mem disk`, and `mem_sleep` shows `s2idle [deep]`; these are options, not an observed suspend. |
| Display and concurrency | `card0` and the same connector names remain present. `fb0` reports `gma500drmfb`, `1280,800`, 32 bpp, stride `8192`, as in H0. Xorg is present, PID `1772`, state `S (sleeping)`. IRQ 16's text line names `gma500, eth0` and has zero counts at capture. | Row 13 gains a current process and shared-IRQ snapshot, not quiescence or exclusion. The unprivileged DRM-fd scan inspected 147 process directories but could not list 126 fd directories; it saw no accessible DRM fd owner and is **not** an authoritative client inventory. Xorg's sleeping state does not prove it cannot issue work. |
| Recovery visibility | `sysrq=438`, `dmesg_restrict=1`, `panic=0`, `panic_on_oops=0`. `/sys/fs/pstore` is empty and not mounted; `/sys/class/watchdog` is empty. | Row 16 remains UNKNOWN. This matches H0's relevant SysRq/pstore/watchdog limits; it does not validate recovery. No kernel log or privileged access was attempted. |

The raw snapshot also retains absent/failed attribute reads instead of treating them as zeros. `fb0/blank` yielded empty text, so it is not used as a display-blanking claim. No connector-status or debugfs path was opened.

## Interpretation boundary

`HW-OBSERVED` here means text, symlink, directory name, or process state returned by ordinary OS interfaces on this identified Mini 12 in P1. A driver binding and `runtime_status=active` do not establish the SGX internal power/clock/reset prerequisites, a side-effect-free 32-bit identification read, a bounded CPU-MMIO failure mode, or physical core revision/errata applicability. No `HW-OBSERVED` SGX MMIO value exists. The [row table](gate-b-passive-reassessment.md) states the exact remaining blockers.

The selected unprivileged passive interfaces have reached their useful limit for this first-read gate: repeating these status snapshots would not supply the missing architectural access contracts, a whole-operation exclusion protocol, CPU-MMIO failure bound, or validated recovery. This does not assert that all possible passive sources are exhausted; privileged logs, vendor documentation or an independently reviewed observer design would require their own scope and evidence. No broader offline search or further target action is initiated by this pass.
