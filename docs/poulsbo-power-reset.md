# Poulsbo: clocks, energy, and reset

Phase 3 — static analysis, 2026-09-17. `CONFIRMED` means the source declares/implements; `INFERRED` is interpretation; `UNKNOWN` is gap. P3 IDs refer to [matrix](evidence-matrix.csv), with repository, commit, file, and lines. The snapshots preserve the original numbering; see [provenance](poulsbo-data/sources.json). No procedure below was executed on the GPU.

**CONFIRMED [P3-033]** — The Poulsbo DDK configures a nominal `SYS_SGX_CLOCK_SPEED` of 200000000. Separately, Linux obtains `core_freq` through a host-bridge PCI configuration transaction: it writes selector `0xd0050300` to `0xd0`, reads `0xd4`, and decodes `clock & 7` as 100/133/150/178/200/266. Sources: [PINFO:46-51](poulsbo-data/../archaeology-data/PINFO.txt); [CLOCK:11-51](poulsbo-data/CLOCK.txt). These are distinct software facts; neither establishes the physical SGX execution clock.

**CONFIRMED [P3-034]** — Linux psb_spank triggers reset BIF/DPM/TA/USE/ISP/TSP/2D, waits 1 ms, releases reset, toggles CLEAR_FAULT, and writes the 2D base. psb_init_pm only changes the 2D gating field to 1. Sources: [LDRVC:103-125](poulsbo-data/../archaeology-data/LDRVC.txt); [DEVICE:85-95](poulsbo-data/DEVICE.txt).

**CONFIRMED [P3-035]** — The Poulsbo psb_power_up/down callbacks return 0 without an island power sequence. power.c maintains a runtime-PM reference for support described as broken, and contains suspend/resume PCI D3hot/D0 with display/GTT/GEM restoration. Sources: [DEVICE:186-194](poulsbo-data/DEVICE.txt); [POWER:46-70](poulsbo-data/POWER.txt); [POWER:95-203](poulsbo-data/POWER.txt).

**CONFIRMED [P3-036]** — Under SUPPORT_DRI_DRM_EXT, DDK Poulsbo acquires display before graphics via ospm_power_using_hw_begin, undoes display if graphics fail, and releases graphics before display. Includes psb_powermgmt.h and sys_pvr_drm_export.h. Sources: [PSYSC:65-69](poulsbo-data/../archaeology-data/PSYSC.txt); [PSYSC:1916-1979](poulsbo-data/../archaeology-data/PSYSC.txt).

**CONFIRMED [P3-037]** — The DDK associates SGX535 revision 121 with BRN22934/23944/23410 and revision 126 with BRN22934. Its BRN23944 reset branch pauses BIF, clears the fault, and drains faults with a temporary PD before restoring the context. Sources: [ERRATA:152-178](poulsbo-data/../archaeology-data/ERRATA.txt); [RESET:471-495](poulsbo-data/../archaeology-data/RESET.txt); [RESET:544-655](poulsbo-data/../archaeology-data/RESET.txt).

**CONFIRMED [P3-038]** — In the EMGD mirror, pwr_set_plb does not program D0–D3 transitions, while pwr_init_plb writes CLKGATECTL=0x1111111 and the register at +8 with zero. The SysDevicePre/PostPowerState hooks of common/sysconfig.c only log messages. Sources: [EMGD_drm_emgd_state_power_plb_pwr_plb_c:83-123](poulsbo-data/EMGD_drm_emgd_state_power_plb_pwr_plb_c.txt); [EMGD_drm_pvr_services4_system_common_sysconfig_c:1332-1365](poulsbo-data/EMGD_drm_pvr_services4_system_common_sysconfig_c.txt).

## What remains to reconstruct

**UNKNOWN:** complete clock/rail/reset sequence from each power state, hardware behind the OSPM APIs, ready bits and their timeouts, reset reach over display/memory, and errata for the installed stepping. The stubs do not demonstrate that power gating is unnecessary; they may depend on another platform component or firmware.

**INFERRED:** the current probe path already touches the SGX; loading or unloading gma500 is not a passive experiment. The operation that Linux calls getting the clock also writes to PCI (P3-033); it should not enter a read-only inventory. The 200 MHz configuration constant does not replace detection or authorize PLL programming.

The DDK reset is part of an initialization with scripts and microkernel state (P3-051). Copying it in isolation does not provide a bring-up procedure. Loop drainage must have limited output and diagnostics in a future project, but this phase does not change the code nor propose new values.

For initial recovery, plan a controlled reboot and the possibility of a power cycle, without relying on a soft reset to recover the display. Recoverability through reboot still needs to be demonstrated on the target platform. Do not perform suspend/resume or gating tests before this preparation.
