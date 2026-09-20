# Registers: documented subset for Poulsbo

> Phase 2 update: local history contains `sgx535defs.h` and an explicit Poulsbo integration in DDK 1.14. The absence references below describe the Phase 1 master checkout. See [archaeology](source-archaeology.md), [recovered files](sgx535-missing-files.md), and [Poulsbo comparison](poulsbo-evidence.md) for the expanded state.

## Origin and trust

**CONFIRMED in the Linux code**, without testing on silicon. All rows in the table come from `psb_reg.h` of the commit fixed in [architecture.md](architecture.md). “Function” describes name/comment and, when indicated, observed use; it does not specify all effects, RO/RW permissions, reset values, reserved bits, or secure access.

The offsets below are relative to the SGX window. In Poulsbo, the driver maps **PCI resource 0 + 0x40000**, extension `0x8000`, selected by `psb_chip_ops`. These are not absolute physical addresses. [references/linux/drivers/gpu/drm/gma500/psb_drv.h:41-53](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L41); [references/linux/drivers/gpu/drm/gma500/psb_device.c:272](../references/linux/drivers/gpu/drm/gma500/psb_device.c#L272); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:261-264](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L261).

**CONFIRMED:** the IT master checkout does not contain `sgx535defs.h`; Phase 2 retrieved it in historical commits and Phase 3 compared the artifact with Poulsbo/EMGD. This does not make the headers proof of read-safety. See [recovered files](sgx535-missing-files.md), [Poulsbo comparison](poulsbo-evidence.md), and [read-safety audit](safe-register-reads.md). We did not replace missing fields with SGX530/540/544.

| offset | name | bits | function | source | confidence |
| --- | --- | --- | --- | --- | --- |
| `0x0000` | `PSB_CR_CLKGATECTL` | 24 auto/manual; USE 21:20; DPM 17:16; TA 13:12; TSP 9:8; ISP 5:4; 2D 1:0 | Clock fields; codes 0 enabled, 1 disabled, 2 auto | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:13-29](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L13) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0010` | `PSB_CR_CORE_ID` | 31:16 ID; 15:0 config | Declared identification; expected value SGX535 UNKNOWN | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:31-35](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L31) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0014` | `PSB_CR_CORE_REVISION` | 31:24 designer; 23:16 major; 15:8 minor; 7:0 maintenance | Revision fields; relation to DDK review UNKNOWN | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:37-45](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L37) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0018 / 0x001c` | `PSB_CR_DESIGNER_REV_FIELD1 / FIELD2` | UNKNOWN | Names and offsets; no decoding | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:47-58](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L47) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0080` | `PSB_CR_SOFT_RESET` | 6 TSP; 5 ISP; 4 USE; 3 TA; 2 DPM; 1 2D; 0 BIF | Block reset; used in psb_spank | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:49-56](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L49) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0110` | `PSB_CR_EVENT_HOST_ENABLE2` | 4 BIF_REQUESTER_FAULT | Enabling group 2; writing observed in the postinstall | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:60-65](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L60) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0114` | `PSB_CR_EVENT_HOST_CLEAR2` | 4 BIF_REQUESTER_FAULT | Acknowledge group 2 no handler | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:64-65](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L64) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0118` | `PSB_CR_EVENT_STATUS2` | 4 BIF_REQUESTER_FAULT | Group 2 status read by the handler | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:62-65](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L62) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x012c` | `PSB_CR_EVENT_STATUS` | see events below | Main group status | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:67-84](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L67) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0130` | `PSB_CR_EVENT_HOST_ENABLE` | see events below | Enable; Linux enables 2D_COMPLETE | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:69-84](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L69) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0134` | `PSB_CR_EVENT_HOST_CLEAR` | see events below | Acknowledge written by the handler | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:71-84](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L71) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0a0c + 4*i; i=0..15` | `PSB_CR_USE_CODE_BASE(i)` | 26:25 DM; 24:0 base; alignshift=7 | Bases USE: vertex/pixel/EDM; uncertain address comment | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:87-114](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L87) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0ab8` | `PSB_CR_PDS_EXEC_BASE` | ADDR_SHIFT=20; ALIGNSHIFT=20 | Base PDS; init Linux writes 0x20000000 | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:116-118](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L116) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0ac4` | `PSB_CR_EVENT_KICKER` | ADDRESS_SHIFT=4 | Kicker address according to macro; full format UNKNOWN | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:120-121](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L120) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0ac8` | `PSB_CR_EVENT_KICK` | 0 NOW | Kick defined; no emission in inspected gma500 | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:123-124](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L123) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0c00` | `PSB_CR_BIF_CTRL` | 4 CLEAR_FAULT; 3 INVALDC; 2 FLUSH | BIF Control; invalidation and fault clearing used | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:128-131](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L128) | CONFIRMED on Linux; UNKNOWN fields explicit |
| `0x0c04` | `PSB_CR_BIF_INT_STAT` | 14 PF_N_RW; 13:0 fault; requestors below | Fault classification read by the IRQ | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:133-147](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L133) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0c08` | `PSB_CR_BIF_FAULT` | UNKNOWN as bitfield; read as address | Address that failed according to the handler, not requestor flags | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:135-147](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L135) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0c38` | `PSB_CR_BIF_DIR_LIST_BASE1` | UNKNOWN | Define offset; Linux context 1 writes 0x0c3c | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:126-126](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L126) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0c78 / 0x0c7c` | `PSB_CR_BIF_BANK0 / BANK1` | UNKNOWN | Banks; init Linux writes zero | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:149-150](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L149) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0c84` | `PSB_CR_BIF_DIR_LIST_BASE0` | Linux writes PFN PD << PAGE_SHIFT | Context directory 0 | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:151-151](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L151) | CONFIRMED in Linux; explicit UNKNOWN fields |
| `0x0c88` | `PSB_CR_BIF_TWOD_REQ_BASE` | UNKNOWN as mask | Base requestor 2D; Linux writes gatt_start | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:152-152](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L152) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0cac` | `PSB_CR_BIF_3D_REQ_BASE` | UNKNOWN as mask | Base requestor 3D; Linux writes 0x30000000 | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:153-153](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L153) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0e04` | `PSB_CR_2D_BLIT_STATUS` | 24 BUSY; 23:0 COMPLETE | Status 2D read on IRQ | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:160-163](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L160) | CONFIRMED on Linux; explicit UNKNOWN fields |
| `0x0e18` | `PSB_CR_2D_SOCIF` | 7:0 FREESPACE; EMPTY=0x80 | Space/empty settings; no reconstructed submission | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:155-158](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L155) | CONFIRMED on Linux; explicit UNKNOWN fields |

## Events and faults

**CONFIRMED:** main event mask: bit 31 MASTER_INTERRUPT; 28 TA_DPM_FAULT; 27 TWOD_COMPLETE; 25 DPM_OUT_OF_MEMORY_ZLS; 24 DPM_TA_MEM_FREE; 18 PIXELBE_END_RENDER; 14 SW_EVENT; 13 TA_FINISHED; 12 TA_TERMINATE; 3 DPM_REACHED_MEM_THRESH; 2 DPM_OUT_OF_MEMORY_GBL; 1 DPM_OUT_OF_MEMORY_MT; 0 DPM_3D_MEM_FREE. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:72-84](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L72).

**CONFIRMED:** BIF_INT_STAT requesters defined: 1 CACHE, 2 TA, 3 VDM, 4 2D, 5 PBE, 6 TSP, 7 ISP, 8 USSEPDS, 9 HOST. Although the defines appear after `BIF_FAULT`, the use of the handler applies the masks to the **BIF_INT_STAT** value and treats BIF_FAULT as an address. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:133-147](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L133); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:159-188](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L159).

**CONFIRMED:** gma500 writes the status values to the clear registers and reads CLEAR2 for posting; it enables only the 2D events and BIF fault in the postinstall. This documents the pattern used by the driver, it does not guarantee that any writing of reserved bits is allowed. [references/linux/drivers/gpu/drm/gma500/psb_irq.c:192-195](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L192); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:286-289](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L286).

## Other spaces: do not mix

| Space | Value | Confirmed meaning | Source |
| --- | --- | --- | --- |
| SGX Window | `0x4000` | defines slave port 2D; not an opcode | [references/linux/drivers/gpu/drm/gma500/psb_drv.h:61](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L61) |
| PCI Configuration | `0x52`, `0x5c` | GMCH_CTRL, BSM | [references/linux/drivers/gpu/drm/gma500/psb_drv.h:55-58](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L55) |
| VDC MMIO | `0x2020` | PGETBL_CTL; GTT enable | [references/linux/drivers/gpu/drm/gma500/psb_drv.h:59-60](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L59); [references/linux/drivers/gpu/drm/gma500/gtt.c:128-144](../references/linux/drivers/gpu/drm/gma500/gtt.c#L128) |
| Virtual addresses written in init | `0x20000000`, `0x30000000` | values for PDS_EXEC_BASE and BIF_3D_REQ_BASE, not MMIO offsets | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:361-362](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L361) |
| ABI CCB | `SGXMKIF_CMD_*` | software enum, not registers | [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h:65-82](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h#L65) |

The 2D package defines from [references/linux/drivers/gpu/drm/gma500/psb_reg.h:169-190](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L169) are command data and should not be added as register offsets.

## UNKNOWN / review points

- RO/RW, clear-on-read, W1C formal, reset values and reservations are not inferred just from the names.
- CORE_ID/REVISION were research candidates. Phase 4 classifies `UNKNOWN` for actual reading: there is historical use, but no contract for read side effects, power, or locking. They are not part of an approved whitelist.
- The indexing of BASE1 differs between Linux and IT; details in [mmu-bif.md](mmu-bif.md).
- The basic reset uses the bits above, but boot and recovery firmware require additional state: [references/linux/drivers/gpu/drm/gma500/psb_drv.c:103-125](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L103) versus [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:467-665](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L467).
