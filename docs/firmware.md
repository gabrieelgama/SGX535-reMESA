# Firmware / microkernel

> Phase 2 update: local history contains `sgx535defs.h` and an explicit Poulsbo integration in DDK 1.14. The absence references below describe the Phase 1 master checkout. See [archaeology](source-archaeology.md), [recovered files](sgx535-missing-files.md), and [Poulsbo comparison](poulsbo-evidence.md) for the expanded state.

## CONFIRMED — o que o host sabe

The KM receives handles for CCB kernel, control, event-kicker, host-control, and TA/3D-control, as well as `aui32HostKickAddr`, scripts, build options, and structure sizes. [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h:85-114](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h#L85). `InitDevInfo` associates these buffers with the KM state and copies scripts and handlers. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:207-293](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L207).

The scripts are lists of WRITE_HW_REG, READ_HW_REG, and HALT operations, with additional operation conditioned by PDUMP. There are two init lists of 64 entries and one deinit list of 16. The executor is in the KM; the filled lists were not retrieved as SGX535 source. [references/omap5-sgx-ddk-linux/eurasia_km/include4/sgxscript.h:49-89](../references/omap5-sgx-ddk-linux/eurasia_km/include4/sgxscript.h#L49); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:327-370](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L327).

The initialization runs script part 1 before the reset and part 2 after, zeros `ui32InitStatus`, performs a kick and waits for the INIT_COMPLETE bit with timeout. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:478-647](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L478). The bit is `1<<0`. [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:289](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L289). Therefore, having `sgxreset.c` does not equate to having a complete boot sequence.

The DDK checks build options between client/KM, microkernel version/build, core revision, and structure sizes. There are explicit exceptions and the HEAD revision case. Do not declare UM binaries as compatible just because everyone uses the “1.9” label. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:2489-2649](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L2489).

## UM Artifacts observed, without execution

**CONFIRMED as ELF metadata**, not instruction semantics: `pvrsrvinit`, `libsrv_init.so.1.9.6.0`, and `libusc.so.1.9.6.0` are ARM ELF32 little-endian and contain symbols/debug info. The reproducible output of `file`/`readelf -Ws`, SHA-256, and commit are in [evidence.txt](evidence.txt).

In the symbol table of `libsrv_init.so.1.9.6.0`:

| Symbol | ELF Value | Size | Limited Interpretation |
| --- | --- | --- | --- |
| `pbuKernelProgram` | `0x00001eb8` | 60208 | local object with microkernel program name |
| `pbSlaveuKernelProgram` | `0x000109e8` | 11552 | local object with the name of slave program |
| `g_pui32PDSUKERNEL_INIT_PRIM1` | `0x00013750` | 24 | objeto PDS nomeado |
| `g_pui32PDSUKERNEL_INIT_SEC` | `0x00013768` | 56 | objeto PDS nomeado |
| `g_pui32PDSUKERNEL_INIT_PRIM2` | `0x000137a0` | 24 | objeto PDS nomeado |
| `g_pui32PDSUKERNEL_EVENTS` | `0x000137b8` | 252 | objeto PDS nomeado |

ELF values are not file offsets or ready-to-use GPU addresses. **INFERRED:** the library loads content from embedded microkernel/PDS programs; **UNKNOWN:** SGX535 compatibility, format, relocation, and exact initialization code. The KM's OMAP5430 target selects SGX544, reinforcing that these objects should not be treated as Poulsbo firmware. [references/omap5-sgx-ddk-linux/eurasia_km/eurasiacon/build/linux2/omap5430_linux/Makefile:112-113](../references/omap5-sgx-ddk-linux/eurasia_km/eurasiacon/build/linux2/omap5430_linux/Makefile#L112).

## Energy and recovery in the contract

**CONFIRMED:** host-control contains power, cleanup, init, and IRQ states. POWER commands distinguish POWEROFF, IDLE, and RESUME; pre-power sends command, waits for status and pending interrupts, checks clock gating, and may run deinit. Post-power reinitializes or sends RESUME according to transition. [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:103-148](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L103); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:295-350](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L295); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxpower.c:295-418](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxpower.c#L295); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxpower.c:444-503](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxpower.c#L444).

**CONFIRMED:** the non-MP branch also forces the recovery path on the first startup (`bHardwareRecovery |= bFirstTime`). [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:499-503](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L499). **UNKNOWN:** whether this should be preserved for Poulsbo and what previous state the platform firmware leaves.

## Source and reuse

The UM manifesto is an OLE document, not a C source. The inventory records excerpts located by ASCII extraction offsets over the original file; the listing does not equate to a complete extraction from Word. It identifies TI TSPA, binary distribution without modification, and reverse engineering restriction. No firmware was copied to a new implementation.

**UNKNOWN:** availability of SGX535 microkernel with compatible source/license, possibility of a new minimal microkernel and mandatory task set. A simple writing of EVENT_KICK without valid program and memory is not a safe experiment. Next sources: legitimately published UM init, sources of microkernel/PDS, and specification corresponding to the identified revision, with provenance audit before use.
