# Poulsbo: memória, GTT e BIF/MMU

Fase 3 — análise estática, 2026-09-17. `CONFIRMED` significa o que a fonte declara/implementa; `INFERRED` é interpretação; `UNKNOWN` é lacuna. IDs P3 remetem à [matriz](evidence-matrix.csv), com repositório, commit, arquivo e linhas. Os snapshots preservam a numeração original; ver [proveniência](poulsbo-data/sources.json). Nenhum procedimento abaixo foi executado na GPU.

**CONFIRMED [P3-020]** — O Linux obtém gtt_phys_start de PGETBL_CTL mascarado por PAGE_MASK; obtém dimensão GTT do recurso 3 e GATT do recurso 2; ioremap usa gtt_phys_start. mmu_gatt_start é fixado em 0xe0000000, distinto de gatt_start. O fallback de GATT 0x40000000 é comentado como caso CDV. Fontes: [GTT:185-280](poulsbo-data/GTT.txt).

**CONFIRMED [P3-021]** — O Linux lê BSM da configuração PCI 0x5c, calcula stolen_size = gtt_phys_start − stolen_base − PAGE_SIZE, mapeia WC e popula a GTT com essas páginas. Fontes: [LDRVH:55-61](poulsbo-data/../archaeology-data/LDRVH.txt); [GEM:309-364](poulsbo-data/GEM.txt).

**CONFIRMED [P3-022]** — Ao fixar um GEM não-stolen, o Linux obtém páginas, aplica WC, insere na GTT e na PD SGX no endereço gatt_start+offset. Ao desafixar remove ambas as traduções e restaura WB; a alocação restringe-se a DMA32. Fontes: [GEM:29-102](poulsbo-data/GEM.txt); [GEM:170-179](poulsbo-data/GEM.txt).

**CONFIRMED [P3-023]** — O header DDK usa páginas 4 KiB, índices PD/PT de 10 bits, PTE valid=1, write-only=2, read-only=4, cache-consistent=8 e EDM-protect=16. Linux usa shifts PDE22/PTE12 e os primeiros quatro flags; a configuração SGX535 seleciona VA32 e 16 dirlists. Fontes: [MMU:48-93](poulsbo-data/../archaeology-data/MMU.txt); [LDRVH:75-84](poulsbo-data/../archaeology-data/LDRVH.txt); [FEATURE:76-88](poulsbo-data/../archaeology-data/FEATURE.txt).

**CONFIRMED [P3-024]** — O Linux zera BANK0/1, remove bypass, insere stolen na PD no endereço gatt_start e instala default PD com argumento 0 e pf_pd com argumento 1. Programa PDS_EXEC_BASE=0x20000000 e BIF_3D_REQ_BASE=0x30000000. Fontes: [LDRVC:127-163](poulsbo-data/../archaeology-data/LDRVC.txt); [LDRVC:337-362](poulsbo-data/../archaeology-data/LDRVC.txt).

**CONFIRMED [P3-025]** — As conversões CPU physical / system physical / device physical da integração Poulsbo no DDK preservam o valor numérico do endereço. Fontes: [PSYSC:1340-1445](poulsbo-data/../archaeology-data/PSYSC.txt).

**CONFIRMED [P3-026]** — Linux calcula BASE0 para contexto 0 e BASE1 + hw_context*4 nos demais. O PSB histórico já contém a mesma fórmula. DDK 1.14 e EMGD usam BASE1 + 4*(ui32DirList−1) para listas 1..15. Fontes: [LMMU:123-135](poulsbo-data/../archaeology-data/LMMU.txt); [PSB_psb_mmu_c:187-201](poulsbo-data/PSB_psb_mmu_c.txt); [RESET:125-135](poulsbo-data/../archaeology-data/RESET.txt); [EMGD_drm_pvr_services4_srvkm_devices_sgx_sgxreset_c:356-369](poulsbo-data/EMGD_drm_pvr_services4_srvkm_devices_sgx_sgxreset_c.txt).

## Traduções que não devem ser fundidas

| Domínio | Origem / uso | Evidência |
|---|---|---|
| CPU virtual | mapeamento kernel/user de RAM ou recursos | P3-021/022 |
| CPU physical / bus | BARs reais e PFNs; identidade numérica apenas nas conversões do DDK citado | P3-006/025 |
| GATT offset | alocação de objetos relativa ao recurso GATT | P3-020/022 |
| GTT table physical | PGETBL_CTL, não base do GATT | P3-020 |
| SGX device VA | gatt_start+offset no caminho GEM atual | P3-022 |
| mmu_gatt_start | constante 0xe0000000 usada no planejamento interno | P3-020/024 |
| PDS / 3D request base | valores escritos pelo Linux, não RAM reservada comprovada nesses endereços | P3-024 |
| Host-port VA DDK | 0xd0000000, não endereço CPU universal | P3-008 |

**INFERRED:** o software mantém mapeamentos paralelos de páginas; não há base para afirmar tradução em cascata SGX BIF → GTT → RAM. **UNKNOWN:** coerência de todos os requestors, necessidade de manutenção de cache em cada transição, comportamento de bus mastering na placa real e equivalência de domínios sob outra configuração de plataforma.

## Divergência de directory list: manter aberta

Aritmética verificável de P3-018/026: argumento Linux `1` escreve `0xc38+4 = 0xc3c`, chamado **BASE2** no header SGX535. O índice DDK `1` escreve `0xc38`, chamado **BASE1**. Isso não autoriza corrigir o Linux nem trocar o índice em um experimento.

| Hipótese | Evidência existente | Estado |
|---|---|---|
| Revisão diferente SGX | build TI aponta 121; sem identificação física nem erratum explicativo | UNKNOWN |
| Integração SoC | ambos PSB histórico e atual têm fórmula distinta do DDK | UNKNOWN: correlação não é explicação |
| Quantidade de address spaces | ramos SGX535 TI e EMGD declaram 16 (P3-003/062) | não explica sozinho o deslocamento |
| Register layout | headers TI e EMGD nomeiam BASE1=c38, BASE2=c3c, BASE0=c84 (P3-018/062) | evidência contra supor layout linear a partir de BASE0 |
| Configuração de build | SGX_FEATURE_MULTIPLE_MEM_CONTEXTS guarda loop DDK | UNKNOWN: falta conexão entre enumeração e seleção Linux |
| Convenção software / defeito legado | fórmula já existe no PSB antigo | UNKNOWN: nenhum comentário de intenção/erratum localizado |
| Comportamento específico Poulsbo | falta manual ou trace legitimamente obtido, com revisão conhecida | UNKNOWN |

Outra lacuna: o comentário do Linux atribui a `0xd0000000` um bug da **video MMU**; isso não prova que seja a mesma restrição do host-port SGX nem resolve o uso de `gatt_start` versus `mmu_gatt_start` (P3-020/024). Permanecem domínios distintos até evidência adicional.


**CONFIRMED [P3-062]** — O ramo SGX535 EMGD declara VA32 e 16 dirlists; seu header enumera BASE1=c38, BASE2=c3c, BASE15=c70 e BASE0=c84. Fontes: [EMGD_drm_pvr_services4_srvkm_hwdefs_sgxfeaturedefs_h:53-64](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgxfeaturedefs_h.txt); [EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h:466-541](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt).
