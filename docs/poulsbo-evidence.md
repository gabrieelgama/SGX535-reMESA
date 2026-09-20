# Poulsbo: direct evidence and comparison with SGX535

Convention: **CONFIRMED** confirms the content/history of the sources, not hardware operation; **INFERRED** identifies a deduction; **UNKNOWN** is what has not yet been established. Source identifiers like H535 and PBUILD resolve to repository, full commit, file, and hash in [source catalog](source-archaeology.md#source-catalog). Document snapshots preserve the original lines and license notices; they are not implementation files.

The two anchors are the IT KM 1.14 `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` and the local Linux `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f`. H535 is cited by the first local version found (1.13); its blob is identical in anchor 1.14. We do not use SGX540/544 as substitutes.

## Intel Integration explicitly published in the KM

| Tema | DDK Poulsbo | Linux gma500 | Confidence / limite |
| --- | --- | --- | --- |
| Core/build review | Poulsbo D0 → 535/121, [PBUILD:44–50](archaeology-data/PBUILD.txt#L44) | SGX535 comment and IDs 8108/8109, [LDRVC:43–59](archaeology-data/LDRVC.txt#L43) | CONFIRMED; identity of each stepping UNKNOWN |
| MMIO SGX | offset 0x40000, size 0x4000, [PSYS:51–55](archaeology-data/PSYS.txt#L51) | offset 0x40000, size 0x8000, [LDRVH:41–53](archaeology-data/LDRVH.txt#L41) | CONFIRMED: same relative base, different extensions; reason UNKNOWN |
| PCI | vendor 8086, device 8108; MMADR index=4, [PSYS:64–71](archaeology-data/PSYS.txt#L64) | MMIO resource 0; IDs 8108/8109, [LDRVH:49–53](archaeology-data/LDRVH.txt#L49), [LDRVC:58–59](archaeology-data/LDRVC.txt#L58) | CONFIRMED; DDK index is converted before using resource |
| Integration IRQ | IER 20a0, IIR 20a4, IMR 20a8; THALIA bit18, [PSYS:81–94](archaeology-data/PSYS.txt#L81) | same positions and SGX bit18, [LDRVH:91–111](archaeology-data/LDRVH.txt#L91) | CONFIRMED: agreement of definitions and use |
| Host port | feature enabled, size 0x8000000, base VA 0xd0000000, [PSYS:48–49](archaeology-data/PSYS.txt#L48), [PSYS:108–112](archaeology-data/PSYS.txt#L108) | does not match slave port 2D 0x4000, [LDRVH:61](archaeology-data/LDRVH.txt#L61) | CONFIRMED as distinct declarations; need in the new design UNKNOWN |
| Nominal clock | constant 200000000, [PINFO:46–49](archaeology-data/PINFO.txt#L46) | does not show measurement of the target GPU | CONFIRMED as DDK parameter, real frequency UNKNOWN |
| Physical addresses | CPU/system/device converted by identity, [PSYSC:1340–1445](archaeology-data/PSYSC.txt#L1340) | MMU writes page PFN, [LMMU:123–157](archaeology-data/LMMU.txt#L123) | CONFIRMED in software, no proof for all DMA domains |
| Power | external DRM branch uses ospm_* for display/graphics islands, [PSYSC:1916–1979](archaeology-data/PSYSC.txt#L1916) | it is not the same ABI as the current phase 1 callbacks | CONFIRMED as a historical dependency, portability UNKNOWN |

**CONFIRMED:** on Linux, DDK defines `POULSBO_ADDR_RANGE_INDEX = MMADR_INDEX - 4`; the base used for SGX comes from this resource and adds SGX_REGS_OFFSET. Do not interpret MMADR_INDEX=4 as BAR4. [PSYSC:144–149](archaeology-data/PSYSC.txt#L144); [PSYSC:348–350](archaeology-data/PSYSC.txt#L348); [PSYSC:418–430](archaeology-data/PSYSC.txt#L418).

The branch `SUPPORT_DRI_DRM_EXT` depends on `psb_drv.h`, `psb_powermgmt.h`, and `sys_pvr_drm_export.h` and provides `SYSPVRServiceSGXInterrupt`, which calls the device ISR and schedules MISR. [PSYSC:65–69](archaeology-data/PSYSC.txt#L65); [PSYSC:1982–2005](archaeology-data/PSYSC.txt#L1982). **CONFIRMED:** it is a concrete integration point with the historical PSB driver, not a guarantee of fit in the current gma500.

## Offsets: systematic comparison

The direct comparison found **26 numeric offsets with matching names**, with no divergence in this subset. The other 16 Linux numeric defines are USE_CODE_BASE0..15: the DDK expresses these bases through indexed macros. The manual comparison of the formula confirms `0x0a0c + 4*i`, 16 entries, base fields `0x01ffffff` and DM `0x06000000` shift25. [LREG:89–114](archaeology-data/LREG.txt#L89); [H535:727–736](archaeology-data/H535.txt#L727). Formula equality is **CONFIRMED**; equality of effects across all steppings was not tested.

The automatic spreadsheet marks the 16 individual names as UNKNOWN due to the absence of a corresponding literal define; this is not a contradiction with the verified formula. [Full line comparison](archaeology-data/register-offset-comparison.tsv).

| Group | Offsets/valores compared | Sources | Confidence |
| --- | --- | --- | --- |
| Clock/identification/reset | 0000, 0010, 0014, 0018, 001c, 0080; reset bits 0–6 | [LREG:13–56](archaeology-data/LREG.txt#L13); [H535:45–128](archaeology-data/H535.txt#L45) | CONFIRMED as defined |
| Events | 0110, 0114, 0118, 012c, 0130, 0134; fault BIF bit4; SW_EVENT bit14 | [LREG:60–84](archaeology-data/LREG.txt#L60); [H535:129–374](archaeology-data/H535.txt#L129) | CONFIRMED as defined |
| Kick/PDS | 0ab8, 0ac4, 0ac8; NOW bit0 | [LREG:116–124](archaeology-data/LREG.txt#L116); [H535:375–386](archaeology-data/H535.txt#L375) | CONFIRMED how you define |
| BIF | 0c00, 0c04, 0c08; FLUSH bit2, INVALDC bit3, CLEAR_FAULT bit4 | [LREG:128–147](archaeology-data/LREG.txt#L128); [H535:417–458](archaeology-data/H535.txt#L417) | CONFIRMED how you define |
| Banks/directories/bases | 0c38, 0c78, 0c7c, 0c84, 0c88, 0cac | [LREG:126–153](archaeology-data/LREG.txt#L126); [H535:539–682](archaeology-data/H535.txt#L539) | CONFIRMED as you define |
| 2D | status 0e04: busy bit24, complete 23:0; SOCIF 0e18: freespace 7:0 | [LREG:155–163](archaeology-data/LREG.txt#L155); [H535:693–720](archaeology-data/H535.txt#L693) | CONFIRMED as you define |

## BIF: now more precise doubt

**CONFIRMED:** H535 names BASE1=0x0c38 and BASE2=0x0c3c, while Linux selects `BASE1 + hw_context*4` for non-zero context. With context 1 it therefore writes to the register called BASE2 by the SGX535 header. The TI reset uses `BASE1 + (index-1)*4`. [H535:539–546](archaeology-data/H535.txt#L539); [LMMU:123–135](archaeology-data/LMMU.txt#L123); [RESET:125–135](archaeology-data/RESET.txt#L125).

**INFERRED:** the divergence does not come from different BASE1 offsets between these sources; it comes from the indexing formula/convention. **UNKNOWN:** intention of context 1, driver convention, historical bug, or silicon effect. Without the prior history of the local Linux and an explanation of the contract, there is no basis to fix the code.

The header still confirms BANK0/BANK1 with fields EDM 3:0, TA 7:4, HOST 11:8, 3D 15:12, and 2D 19:16; the SGX535 branch declares 16 dirlists and one PT cache entry per line. [H535:603–626](archaeology-data/H535.txt#L603); [FEATURE:76–88](archaeology-data/FEATURE.txt#L76). The effective security/isolation policy remains **UNKNOWN**.

## Additional information that should not become a hardware assumption

**CONFIRMED:** H535 declares address mask BIF_FAULT `0xfffff000`, dirlist bases `0xfffff000`, request bases 2D/3D `0xfff00000`, PDS_EXEC_BASE `0xfff00000`, and EVENT_KICKER `0xfffffff0`. [H535:455–458](archaeology-data/H535.txt#L455); [H535:539–546](archaeology-data/H535.txt#L539); [H535:631–642](archaeology-data/H535.txt#L631); [H535:679–682](archaeology-data/H535.txt#L679); [H535:375–382](archaeology-data/H535.txt#L375). These fields reduce documentation gaps from phase 1, but do not establish side effects or a safe write sequence.

**CONFIRMED:** the SGX535 header and Linux agree on the basic bits PDE/PTE: valid=1, WO=2, RO=4, and cache=8 in the common MMU contract. [MMU:48–93](archaeology-data/MMU.txt#L48); [LDRVH:75–84](archaeology-data/LDRVH.txt#L75). **UNKNOWN:** full coherence and EDM permissions in each integration; using the common header with the 535 selection does not equate to testing.

## Priority external artifact

The most valuable artifact now is **a user-mode source package and microkernel of the IMG SGX DDK 1.14 build 3699939, specifically configured for `pc_i686_poulsbo_d0_linux`, SGX535 rev121, with verifiable license and provenance**. It should contain the SGX initializer that populates scripts/handlers, sources or reproducible build of the microkernel/PDS programs, and complete headers for the ABI corresponding to the KM.

This specification derives from the local anchors [PBUILD:44–52](archaeology-data/PBUILD.txt#L44) and [VER14:51–60](archaeology-data/VER14.txt#L51); the public existence of such a package is **UNKNOWN**. It is a priority because the header and KM integration have already been found, while boot and execution still depend on the missing side. If only one file can be obtained, prioritize the **`sgxinit.c` user-mode of this same entrega/alvo**, along with its includes and build identification; do not confuse it with the `sgxinit.c` kernel already present. This choice is an investigation recommendation (**INFERRED**), not a claim about where the artifact is available.
