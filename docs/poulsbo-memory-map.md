# Poulsbo: memory, GTT and BIF/MMU

Phase 3 — static analysis, 2026-09-17. `CONFIRMED` means the source declares/implements; `INFERRED` is interpretation; `UNKNOWN` is gap. P3 IDs refer to [matrix](evidence-matrix.csv), with repository, commit, file, and lines. The snapshots preserve the original numbering; see [provenance](poulsbo-data/sources.json). No procedure below was executed on the GPU.

**CONFIRMED [P3-020]** — Linux obtains gtt_phys_start from PGETBL_CTL masked by PAGE_MASK; it gets GTT size from resource 3 and GATT from resource 2; ioremap uses gtt_phys_start. mmu_gatt_start is fixed at 0xe0000000, distinct from gatt_start. The GATT fallback 0x40000000 is commented as a CDV case. Sources: [GTT:185-280](poulsbo-data/GTT.txt).

**CONFIRMED [P3-021]** — Linux reads BSM from the PCI configuration 0x5c, calculates stolen_size = gtt_phys_start − stolen_base − PAGE_SIZE, maps WC, and populates the GTT with these pages. Sources: [LDRVH:55-61](poulsbo-data/../archaeology-data/LDRVH.txt); [GEM:309-364](poulsbo-data/GEM.txt).

**CONFIRMED [P3-022]** — When pinning a non-stolen GEM, Linux obtains pages, applies WC, inserts into the GTT and the SGX PD at the address gatt_start+offset. When unpinning, it removes both mappings and restores WB; allocation is limited to DMA32. Sources: [GEM:29-102](poulsbo-data/GEM.txt); [GEM:170-179](poulsbo-data/GEM.txt).

**CONFIRMED [P3-023]** — The DDK header uses 4 KiB pages, 10-bit PD/PT indexes, PTE valid=1, write-only=2, read-only=4, cache-consistent=8, and EDM-protect=16. Linux uses PDE22/PTE12 shifts and the first four flags; the SGX535 setting selects VA32 and 16 dirlists. Sources: [MMU:48-93](poulsbo-data/../archaeology-data/MMU.txt); [LDRVH:75-84](poulsbo-data/../archaeology-data/LDRVH.txt); [FEATURE:76-88](poulsbo-data/../archaeology-data/FEATURE.txt).

**CONFIRMED [P3-024]** — Linux zeros BANK0/1, removes bypass, inserts stolen into PD at the gatt_start address, and installs default PD with argument 0 and pf_pd with argument 1. Programs PDS_EXEC_BASE=0x20000000 and BIF_3D_REQ_BASE=0x30000000. Sources: [LDRVC:127-163](poulsbo-data/../archaeology-data/LDRVC.txt); [LDRVC:337-362](poulsbo-data/../archaeology-data/LDRVC.txt).

**CONFIRMED [P3-025]** — The CPU physical / system physical / device physical conversions of the Poulsbo integration in the DDK preserve the numerical value of the address. Sources: [PSYSC:1340-1445](poulsbo-data/../archaeology-data/PSYSC.txt).

**CONFIRMED [P3-026]** — Linux calculates BASE0 for context 0 and BASE1 + hw_context*4 for the others. The historical PSB already contains the same formula. DDK 1.14 and EMGD use BASE1 + 4*(ui32DirList−1) for lists 1..15. Sources: [LMMU:123-135](poulsbo-data/../archaeology-data/LMMU.txt); [PSB_psb_mmu_c:187-201](poulsbo-data/PSB_psb_mmu_c.txt); [RESET:125-135](poulsbo-data/../archaeology-data/RESET.txt); [EMGD_drm_pvr_services4_srvkm_devices_sgx_sgxreset_c:356-369](poulsbo-data/EMGD_drm_pvr_services4_srvkm_devices_sgx_sgxreset_c.txt).

## Translations that must not be merged

| Domain | Origin / use | Evidence |
|---|---|---|
| Virtual CPU | kernel/user mapping of RAM or resources | P3-021/022 |
| Physical / bus CPU | Real BARs and PFNs; numerical identity only in the conversions of the mentioned DDK | P3-006/025 |
| GATT offset | allocation of objects relative to the GATT resource | P3-020/022 |
| GTT table physical | PGETBL_CTL, not GATT base | P3-020 |
| SGX device VA | gatt_start+offset no caminho GEM atual | P3-022 |
| mmu_gatt_start | constante 0xe0000000 usada no planejamento interno | P3-020/024 |
| PDS / 3D request base | values written by Linux, not RAM reserved proven at these addresses | P3-024 |
| Host-port VA DDK | 0xd0000000, not universal CPU address | P3-008 |

**INFERRED:** the software maintains parallel page mappings; there is no basis to assert SGX BIF → GTT → RAM cascading translation. **UNKNOWN:** coherence of all requestors, need to maintain cache at each transition, bus mastering behavior on the actual board, and domain equivalence under another platform configuration.

## Directory list divergence: keep open

Verifiable arithmetic of P3-018/026: Linux argument `1` writes `0xc38+4 = 0xc3c`, called **BASE2** in the SGX535 header. The DDK index `1` writes `0xc38`, called **BASE1**. This does not authorize fixing Linux or swapping the index in an experiment.

| Hypothesis | Existing evidence | Status |
|---|---|---|
| Different SGX Revision | TI build points to 121; no physical identification or explanatory erratum | UNKNOWN |
| SoC Integration | both historical and current PSB have a distinct DDK formula | UNKNOWN: correlation is not explanation |
| Number of address spaces | SGX535 TI and EMGD branches declare 16 (P3-003/062) | does not explain the offset by itself |
| Register layout | headers TI and EMGD name BASE1=c38, BASE2=c3c, BASE0=c84 (P3-018/062) | evidence against assuming a linear layout from BASE0 |
| Build configuration | SGX_FEATURE_MULTIPLE_MEM_CONTEXTS stores DDK loop | UNKNOWN: missing connection between enumeration and Linux selection |
| Software convention / legacy defect | formula already exists in the old PSB | UNKNOWN: no comment of intention/erratum found |
| Specific behavior Poulsbo | missing manual or legitimately obtained trace, with known review | UNKNOWN |

Another gap: the Linux comment attributes a bug in the **video MMU** to `0xd0000000`; this does not prove that it is the same restriction as the host-port SGX nor does it resolve the use of `gatt_start` versus `mmu_gatt_start` (P3-020/024). Distinct domains remain until further evidence.


**CONFIRMED [P3-062]** — The SGX535 EMGD branch declares VA32 and 16 dirlists; its header lists BASE1=c38, BASE2=c3c, BASE15=c70, and BASE0=c84. Sources: [EMGD_drm_pvr_services4_srvkm_hwdefs_sgxfeaturedefs_h:53-64](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgxfeaturedefs_h.txt); [EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h:466-541](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt).
