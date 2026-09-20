# Poulsbo versus IT integration

Phase 3 — static analysis, 2026-09-17. `CONFIRMED` means the source declares/implements; `INFERRED` is interpretation; `UNKNOWN` is gap. P3 IDs refer to [matrix](evidence-matrix.csv), with repository, commit, file, and lines. The snapshots preserve the original numbering; see [provenance](poulsbo-data/sources.json). No procedure below was executed on the GPU.

**CONFIRMED [P3-059]** — The available OMAP4 path uses pm_runtime_get_sync/put_sync and clock acquisition as gpu_fck/sgx_clk_mux_ck; it is TI integration code, not Poulsbo clock implementation. Sources: [OMAP:238-252](poulsbo-data/OMAP.txt); [OMAP:288-300](poulsbo-data/OMAP.txt); [OMAP:639-674](poulsbo-data/OMAP.txt).

## Comparison by element

`IDENTICAL / COMPATIBLE` is only used below for explicit equality of the indicated numerical or structural subset, never for complete hardware. `SIMILAR` describes a software relationship without ensuring interchange. The classifications are **INFERRED** from the named evidence.

| Element | Classification | Evidence and limit |
|---|---|---|
| BASE0/1/2 SGX535 TI vs header SGX535 EMGD | IDENTICAL / COMPATIBLE (values only) | P3-018/062: c84/c38/c3c; does not include protocol |
| BANK0/1 SGX535 layout | IDENTICAL / COMPATIBLE (numeric subset) | H535599–626 and EMGD466–541, P3-019/062 |
| VA32 / 16 declared dirlists SGX535 | IDENTICAL / COMPATIBLE (declared configuration) | P3-003/062 |
| DDK Poulsbo Aperture base vs Linux | IDENTICAL / COMPATIBLE (offset only) | P3-007: 0x40000; sizes differ |
| PTE valid/WO/RO/cache and page4K | SIMILAR | P3-023; EDM-protect extra and use of flags to clarify |
| Directory list index formula | UNKNOWN | P3-026; divergence preserved, not corrected by assumption |
| CCB/bridge IT Services vs EMGD | SIMILAR | PVR families, different P3-045–054 versions; Binary ABI UNKNOWN |
| PSB XHW vs Services | POULSBO-SPECIFIC (PSB interface examined) | P3-040–051; they are not actually protocol |
| PCI BARs, GTT/BSM/stolen, IRQ THALIA | POULSBO-SPECIFIC | P3-006/020/021/030 |
| OSPM display/graphics in DRM_EXT | POULSBO-SPECIFIC (examined integration code) | P3-036; API implementation missing |
| OMAP platform clocks/runtime PM | TI-SPECIFIC | P3-059; does not prove how SGX535 operates on any OMAP |
| Effective core clock and power islands | UNKNOWN | P3-033/035/036; nominal value is not a measurement |
| Reset and errata applicable to the board | UNKNOWN | P3-034/037; depends on actual revision |
| Cache, barriers, and DMA | SIMILAR in purpose, compatibility UNKNOWN | P3-022/023/050; x86 and ARM not interchangeable |
| Firmware/USSE/PDS payload | UNKNOWN | P3-048/051; no corresponding UM source verified |
| Display LVDS/SDVO and cursor ABI | POULSBO-SPECIFIC | P3-005/052; does not belong to ISA SGX |
| SGX540/544 compared to SGX535 | UNKNOWN outside specific evidence | do not use OMAP branch as a substitute for header535 |

## Additional header discrepancies

The static comparison [header-comparison.json](poulsbo-data/header-comparison.json) covers numeric scalar macros, normalizing suffixes U/UL; it excludes function macros, conditionals, and expressions. It found 597 identical common values, 20 macros only in the TI header, and 6 only in EMGD. The exclusives include triggers of banco2/FLUSH_COMPLETE in TI and PDS/MNE in EMGD (P3-061 and full sources in the catalog). **UNKNOWN:** whether the difference is due to header generation, partial exposure, revision, or integration; the absence of a macro does not prove the absence of hardware. The equality of the 597 values also does not prove the completeness of the headers.

There is no evidence in this comparison to treat clock/reset OMAP as Poulsbo, nor to consider all US15W/WP/WPT identical.


**CONFIRMED [P3-063]** — Comparison of numeric scalar macros: 597 common equal values, 20 macros only in the TI header and 6 only in the EMGD header; function macros/expressions are not included in the calculation. Sources: [H535:1-739](poulsbo-data/../archaeology-data/H535.txt); [EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h:1-633](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt).
