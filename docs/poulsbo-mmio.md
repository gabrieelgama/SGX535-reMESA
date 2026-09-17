# Poulsbo: PCI, MMIO e registradores

Fase 3 — análise estática, 2026-09-17. `CONFIRMED` significa o que a fonte declara/implementa; `INFERRED` é interpretação; `UNKNOWN` é lacuna. IDs P3 remetem à [matriz](evidence-matrix.csv), com repositório, commit, arquivo e linhas. Os snapshots preservam a numeração original; ver [proveniência](poulsbo-data/sources.json). Nenhum procedimento abaixo foi executado na GPU.

**CONFIRMED [P3-006]** — Linux define recursos MMIO=0, GATT=2, GTT=3. O DDK usa MMADR_INDEX=4 e GMADR_INDEX=6, convertidos em índices de recurso por subtração de 4; não são BAR4 e BAR6. Fontes: [LDRVH:41-61](poulsbo-data/../archaeology-data/LDRVH.txt); [PSYS:65-73](poulsbo-data/../archaeology-data/PSYS.txt); [PSYSC:144-149](poulsbo-data/../archaeology-data/PSYSC.txt).

**CONFIRMED [P3-007]** — O Linux mapeia SGX em BAR0+0x40000, tamanho 0x8000. O DDK Poulsbo define o mesmo offset, mas tamanho 0x4000. Esses tamanhos são comprimentos usados pelos drivers, não prova dos limites físicos do bloco. Fontes: [LDRVH:41-53](poulsbo-data/../archaeology-data/LDRVH.txt); [LDRVC:258-266](poulsbo-data/../archaeology-data/LDRVC.txt); [PSYS:51-56](poulsbo-data/../archaeology-data/PSYS.txt).

**CONFIRMED [P3-008]** — O DDK Poulsbo habilita SGX_FEATURE_HOST_PORT, usa recurso GMADR e define tamanho 0x08000000 e VA de host-port 0xd0000000. A configuração EMGD só habilita esse feature sob MAP_UNUSED_MAPPINGS. Fontes: [PSYS:48-49](poulsbo-data/../archaeology-data/PSYS.txt); [PSYS:108-116](poulsbo-data/../archaeology-data/PSYS.txt); [PSYSC:418-438](poulsbo-data/../archaeology-data/PSYSC.txt); [EMGD_drm_pvr_services4_system_include_sysconfig_h:149-157](poulsbo-data/EMGD_drm_pvr_services4_system_include_sysconfig_h.txt).

## Faixas relativas, não endereços físicos fixos

| Base | Intervalo/tamanho usado | Papel | Evidência |
|---|---|---|---|
| BAR0 | offset 0, tamanho Linux 0x80000 | mapeamento VDC amplo, sobrepõe a janela SGX | P3-006/007 |
| BAR0 | offset 0, tamanho DDK 0x2100 | registros Intel, incluindo IRQ | P3-009 |
| BAR0 | 0x40000–0x47fff Linux; 0x40000–0x43fff DDK | acesso SGX | P3-007 |
| BAR0 | offset 0x50000 | MSVDX, bloco distinto | P3-009 |
| BAR0 | offset 0x70000, tamanho 0x2000 DDK | display | P3-009 |
| BAR2 | base/tamanho PCI, host-port DDK 128 MiB | GATT / GMADR | P3-006/008 |
| BAR3 | recurso GTT; tabela mapeada pelo PGETBL_CTL no Linux | não confundir tabela e aperture | P3-020 |

**UNKNOWN:** tamanho físico do aperture SGX, efeitos das leituras fora do subconjunto documentado, requisitos de energia para cada acesso. Não fazer dump indiscriminado de BAR0.


**CONFIRMED [P3-009]** — O header Poulsbo separa Intel 0x00000/0x2100, MSVDX 0x50000 e display 0x70000/0x2000. Fontes: [PSYS:51-56](poulsbo-data/../archaeology-data/PSYS.txt); [PSYS:83-107](poulsbo-data/../archaeology-data/PSYS.txt).

## Registradores SGX selecionados

Offsets abaixo são relativos à base SGX, nunca relativos ao BAR0 diretamente. Cada linha confirma a definição no header SGX535, não a segurança de acesso.

| offset | name | bits | function | source | confidence |
|---|---|---|---|---|---|
| 0x0000 | CLKGATECTL | 2D[1:0], ISP[5:4], TSP[9:8], TA[13:12], DPM[17:16], USE[21:20], auto[24] | campos de clock gating | [H535:45-61](archaeology-data/H535.txt), P3-010 | CONFIRMED |
| 0x0010 / 0x0014 | CORE_ID / CORE_REVISION | ID[31:16], config[15:0]; designer/major/minor/maintenance em bytes | identificação declarada | [H535:89-104](archaeology-data/H535.txt), P3-011 | CONFIRMED |
| 0x0018 / 0x001c | DESIGNER_REV_FIELD1/2 | 32 bits | campos de revisão do integrador | [H535:105-112](archaeology-data/H535.txt), P3-012 | CONFIRMED |
| 0x0080 | SOFT_RESET | BIF0, 2D1, DPM2, TA3, USE4, ISP5, TSP6 | reset dos blocos | [H535:113-128](archaeology-data/H535.txt), P3-013 | CONFIRMED |
| 0x0110 / 0x0114 / 0x0118 | EVENT_HOST_ENABLE2 / CLEAR2 / STATUS2 | BIF_REQUESTER_FAULT bit4 | segundo banco de eventos | [H535:129-182](archaeology-data/H535.txt), P3-014 | CONFIRMED |
| 0x012c / 0x0130 / 0x0134 | EVENT_STATUS / HOST_ENABLE / HOST_CLEAR | máscaras por evento; consultar fonte | primeiro banco de eventos | [H535:183-374](archaeology-data/H535.txt), P3-015 | CONFIRMED |
| 0x0ab8 / 0x0ac4 / 0x0ac8 | PDS_EXEC_BASE / EVENT_KICKER / EVENT_KICK | base fff00000; endereço fffffff0; NOW bit0 | bootstrap/notificação no DDK | [H535:375-386](archaeology-data/H535.txt), P3-016 | CONFIRMED |
| 0x0c00 / 0x0c04 / 0x0c08 | BIF_CTRL / INT_STAT / FAULT | pause1, flush2, inval3, clear4; fault-address fffff000 | controle e falha BIF | [H535:417-458](archaeology-data/H535.txt), P3-017 | CONFIRMED |
| 0x0c38..0x0c70 / 0x0c84 | DIR_LIST_BASE1..15 / BASE0 | endereço fffff000 | directory lists; BASE0 fora da sequência | [H535:539-634](archaeology-data/H535.txt), P3-018 | CONFIRMED |
| 0x0c74 / 0x0c78 / 0x0c7c | BANK_SET / BANK0 / BANK1 | SELECT3ff; EDM[3:0],TA[7:4],HOST[11:8],3D[15:12],2D[19:16] | seleção de contextos por requestor | [H535:599-626](archaeology-data/H535.txt), P3-019 | CONFIRMED |


**UNKNOWN:** diferenças de versão dos headers não são errata automaticamente. O header SGX535 do espelho EMGD também nomeia PDS 0x0abc e MNE_CR_CTRL 0x0d00 / inval 0x0d20 (P3-061); não adicionar estes offsets à allowlist de um experimento apenas por existirem ali. [Comparação numérica](poulsbo-data/header-comparison.json) separa macros ausentes de valores divergentes. Não reproduzir as constantes como sequência de programação.

**CONFIRMED [P3-061]** — O header SGX535 EMGD contém EUR_CR_PDS=0x0abc, DOUT_TIMEOUT_DISABLE bit6, MNE_CR_CTRL=0x0d00, BYP_CC=0x8000 e MNE_CR_CTRL_INVAL=0x0d20. Fontes: [EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h:324-327](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt); [EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h:628-630](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt).
