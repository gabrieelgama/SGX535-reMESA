# Poulsbo versus integração TI

Fase 3 — análise estática, 2026-09-17. `CONFIRMED` significa o que a fonte declara/implementa; `INFERRED` é interpretação; `UNKNOWN` é lacuna. IDs P3 remetem à [matriz](evidence-matrix.csv), com repositório, commit, arquivo e linhas. Os snapshots preservam a numeração original; ver [proveniência](poulsbo-data/sources.json). Nenhum procedimento abaixo foi executado na GPU.

**CONFIRMED [P3-059]** — O caminho OMAP4 disponível usa pm_runtime_get_sync/put_sync e aquisição de clocks como gpu_fck/sgx_clk_mux_ck; é código de integração TI, não implementação de clock Poulsbo. Fontes: [OMAP:238-252](poulsbo-data/OMAP.txt); [OMAP:288-300](poulsbo-data/OMAP.txt); [OMAP:639-674](poulsbo-data/OMAP.txt).

## Comparação por elemento

`IDENTICAL / COMPATIBLE` só é usado abaixo para igualdade explícita do subconjunto numérico ou estrutural indicado, nunca para hardware completo. `SIMILAR` descreve relação de software sem assegurar intercâmbio. As classificações são **INFERRED** das evidências nomeadas.

| Elemento | Classificação | Evidência e limite |
|---|---|---|
| BASE0/1/2 SGX535 TI vs header SGX535 EMGD | IDENTICAL / COMPATIBLE (valores apenas) | P3-018/062: c84/c38/c3c; não inclui protocolo |
| BANK0/1 layout SGX535 | IDENTICAL / COMPATIBLE (subconjunto numérico) | H535599–626 e EMGD466–541, P3-019/062 |
| VA32 / 16 dirlists declarados SGX535 | IDENTICAL / COMPATIBLE (configuração declarada) | P3-003/062 |
| Aperture base DDK Poulsbo vs Linux | IDENTICAL / COMPATIBLE (offset apenas) | P3-007: 0x40000; tamanhos diferem |
| PTE valid/WO/RO/cache e página4K | SIMILAR | P3-023; EDM-protect extra e uso de flags a esclarecer |
| Fórmula de índice directory list | UNKNOWN | P3-026; divergência preservada, não corrigida por suposição |
| CCB/bridge Services TI vs EMGD | SIMILAR | famílias PVR, versões distintas P3-045–054; ABI binária UNKNOWN |
| PSB XHW vs Services | POULSBO-SPECIFIC (interface PSB examinada) | P3-040–051; não são mesmo protocolo |
| PCI BARs, GTT/BSM/stolen, IRQ THALIA | POULSBO-SPECIFIC | P3-006/020/021/030 |
| OSPM display/graphics em DRM_EXT | POULSBO-SPECIFIC (código da integração examinada) | P3-036; implementação da API ausente |
| OMAP platform clocks/runtime PM | TI-SPECIFIC | P3-059; não prova como SGX535 opera em qualquer OMAP |
| Core clock efetivo e power islands | UNKNOWN | P3-033/035/036; valor nominal não é medição |
| Reset e errata aplicáveis à placa | UNKNOWN | P3-034/037; depende de revisão real |
| Cache, barreiras e DMA | SIMILAR no objetivo, compatibilidade UNKNOWN | P3-022/023/050; x86 e ARM não intercambiáveis |
| Firmware/USSE/PDS payload | UNKNOWN | P3-048/051; sem fonte UM correspondente verificada |
| Display LVDS/SDVO e ABI de cursor | POULSBO-SPECIFIC | P3-005/052; não pertence à ISA SGX |
| SGX540/544 comparados a SGX535 | UNKNOWN fora das evidências específicas | não usar ramo OMAP como substituto do header535 |

## Discrepâncias adicionais de headers

A comparação estática [header-comparison.json](poulsbo-data/header-comparison.json) cobre macros escalares numéricas, normalizando sufixos U/UL; exclui macros-função, condicionais e expressões. Ela encontrou 597 valores comuns iguais, 20 macros apenas no header TI e 6 apenas no EMGD. As exclusivas incluem triggers do banco2/FLUSH_COMPLETE no TI e PDS/MNE no EMGD (P3-061 e fontes completas no catálogo). **UNKNOWN:** se a diferença é geração do header, exposição parcial, revisão ou integração; ausência de macro não prova ausência de hardware. A igualdade de 597 valores também não comprova a completude dos headers.

Não há evidência nesta comparação para tratar o clock/reset OMAP como Poulsbo, nem para considerar todos os US15W/WP/WPT idênticos.


**CONFIRMED [P3-063]** — Comparação de macros escalares numéricas: 597 valores comuns iguais, 20 macros só no header TI e 6 só no header EMGD; macros-função/expressões não entram no cálculo. Fontes: [H535:1-739](poulsbo-data/../archaeology-data/H535.txt); [EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h:1-633](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt).
