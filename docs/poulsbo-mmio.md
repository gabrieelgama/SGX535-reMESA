# Poulsbo: PCI, MMIO, and registers

Phase 3 — static analysis, 2026-09-17. `CONFIRMED` means the source declares/implements; `INFERRED` is interpretation; `UNKNOWN` is gap. P3 IDs refer to [matrix](evidence-matrix.csv), with repository, commit, file, and lines. The snapshots preserve the original numbering; see [provenance](poulsbo-data/sources.json). No procedure below was executed on the GPU.

**CONFIRMED [P3-006]** — Linux defines resources MMIO=0, GATT=2, GTT=3. The DDK uses MMADR_INDEX=4 and GMADR_INDEX=6, converted into resource indices by subtracting 4; they are not BAR4 and BAR6. Sources: [LDRVH:41-61](poulsbo-data/../archaeology-data/LDRVH.txt); [PSYS:65-73](poulsbo-data/../archaeology-data/PSYS.txt); [PSYSC:144-149](poulsbo-data/../archaeology-data/PSYSC.txt).

**CONFIRMED [P3-007]** — Linux maps SGX at BAR0+0x40000, size 0x8000. The Poulsbo DDK defines the same offset, but size 0x4000. These sizes are lengths used by the drivers, not proof of the physical limits of the block. Sources: [LDRVH:41-53](poulsbo-data/../archaeology-data/LDRVH.txt); [LDRVC:258-266](poulsbo-data/../archaeology-data/LDRVC.txt); [PSYS:51-56](poulsbo-data/../archaeology-data/PSYS.txt).

**CONFIRMED [P3-008]** — The Poulsbo DDK enables SGX_FEATURE_HOST_PORT, uses the GMADR feature, and sets the size 0x08000000 and host-port VA 0xd0000000. The EMGD configuration only enables this feature under MAP_UNUSED_MAPPINGS. Sources: [PSYS:48-49](poulsbo-data/../archaeology-data/PSYS.txt); [PSYS:108-116](poulsbo-data/../archaeology-data/PSYS.txt); [PSYSC:418-438](poulsbo-data/../archaeology-data/PSYSC.txt); [EMGD_drm_pvr_services4_system_include_sysconfig_h:149-157](poulsbo-data/EMGD_drm_pvr_services4_system_include_sysconfig_h.txt).

## Relative ranges, not fixed physical addresses

| Base | Intervalo/tamanho used | Paper | Evidence |
|---|---|---|---|
| BAR0 | offset 0, size Linux 0x80000 | wide VDC mapping, overlaps the SGX window | P3-006/007 |
| BAR0 | offset 0, tamanho DDK 0x2100 | registros Intel, incluindo IRQ | P3-009 |
| BAR0 | 0x40000–0x47fff Linux; 0x40000–0x43fff DDK | acesso SGX | P3-007 |
| BAR0 | offset 0x50000 | MSVDX, bloco distinto | P3-009 |
| BAR0 | offset 0x70000, tamanho 0x2000 DDK | display | P3-009 |
| BAR2 | base/tamanho PCI, host-port DDK 128 MiB | GATT / GMADR | P3-006/008 |
| BAR3 | GTT resource; table mapped by PGETBL_CTL on Linux | do not confuse table and aperture | P3-020 |

**UNKNOWN:** physical size of the SGX aperture, effects of reads outside the documented subset, power requirements for each access. Do not indiscriminately dump BAR0.


**CONFIRMED [P3-009]** — The Poulsbo header separates Intel 0x00000/0x2100, MSVDX 0x50000, and display 0x70000/0x2000. Sources: [PSYS:51-56](poulsbo-data/../archaeology-data/PSYS.txt); [PSYS:83-107](poulsbo-data/../archaeology-data/PSYS.txt).

## Selected SGX Registers

The offsets below are relative to the SGX base, never directly relative to BAR0. Each line confirms the definition in the SGX535 header, not the access security.

| offset | name | bits | function | source | confidence |
|---|---|---|---|---|---|
| 0x0000 | CLKGATECTL | 2D[1:0], ISP[5:4], TSP[9:8], TA[13:12], DPM[17:16], USE[21:20], auto[24] | clock gating fields | [H535:45-61](archaeology-data/H535.txt), P3-010 | CONFIRMED |
| 0x0010 / 0x0014 | CORE_ID / CORE_REVISION | ID[31:16], config[15:0]; designer/major/minor/maintenance in bytes | declared identification | [H535:89-104](archaeology-data/H535.txt), P3-011 | CONFIRMED |
| 0x0018 / 0x001c | DESIGNER_REV_FIELD1/2 | 32 bits | integrator revision fields | [H535:105-112](archaeology-data/H535.txt), P3-012 | CONFIRMED |
| 0x0080 | SOFT_RESET | BIF0, 2D1, DPM2, TA3, USE4, ISP5, TSP6 | reset dos blocos | [H535:113-128](archaeology-data/H535.txt), P3-013 | CONFIRMED |
| 0x0110 / 0x0114 / 0x0118 | EVENT_HOST_ENABLE2 / CLEAR2 / STATUS2 | BIF_REQUESTER_FAULT bit4 | segundo banco de eventos | [H535:129-182](archaeology-data/H535.txt), P3-014 | CONFIRMED |
| 0x012c / 0x0130 / 0x0134 | EVENT_STATUS / HOST_ENABLE / HOST_CLEAR | masks per event; consult source | first event bank | [H535:183-374](archaeology-data/H535.txt), P3-015 | CONFIRMED |
| 0x0ab8 / 0x0ac4 / 0x0ac8 | PDS_EXEC_BASE / EVENT_KICKER / EVENT_KICK | base fff00000; address fffffff0; NOW bit0 | bootstrap/notification in DDK | [H535:375-386](archaeology-data/H535.txt), P3-016 | CONFIRMED |
| 0x0c00 / 0x0c04 / 0x0c08 | BIF_CTRL / INT_STAT / FAULT | pause1, flush2, invalidate3, clear4; fault-address fffff000 | BIF control and fault | [H535:417-458](archaeology-data/H535.txt), P3-017 | CONFIRMED |
| 0x0c38..0x0c70 / 0x0c84 | DIR_LIST_BASE1..15 / BASE0 | address fffff000 | directory lists; BASE0 out of sequence | [H535:539-634](archaeology-data/H535.txt), P3-018 | CONFIRMED |
| 0x0c74 / 0x0c78 / 0x0c7c | BANK_SET / BANK0 / BANK1 | SELECT3ff; EDM[3:0],TA[7:4],HOST[11:8],3D[15:12],2D[19:16] | context selection by requestor | [H535:599-626](archaeology-data/H535.txt), P3-019 | CONFIRMED |


**UNKNOWN:** version differences in headers are not automatically errata. The SGX535 header from the EMGD mirror also names PDS 0x0abc and MNE_CR_CTRL 0x0d00 / invalidate 0x0d20 (P3-061); do not add these offsets to the allowlist of an experiment just because they exist there. [Numeric comparison](poulsbo-data/header-comparison.json) separates missing macros from divergent values. Do not reproduce the constants as a programming sequence.

**CONFIRMED [P3-061]** — The SGX535 EMGD header contains EUR_CR_PDS=0x0abc, DOUT_TIMEOUT_DISABLE bit6, MNE_CR_CTRL=0x0d00, BYP_CC=0x8000 and MNE_CR_CTRL_INVAL=0x0d20. Sources: [EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h:324-327](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt); [EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h:628-630](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt).
