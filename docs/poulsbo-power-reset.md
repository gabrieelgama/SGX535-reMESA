# Poulsbo: clocks, energia e reset

Fase 3 — análise estática, 2026-09-17. `CONFIRMED` significa o que a fonte declara/implementa; `INFERRED` é interpretação; `UNKNOWN` é lacuna. IDs P3 remetem à [matriz](evidence-matrix.csv), com repositório, commit, arquivo e linhas. Os snapshots preservam a numeração original; ver [proveniência](poulsbo-data/sources.json). Nenhum procedimento abaixo foi executado na GPU.

**CONFIRMED [P3-033]** — O DDK Poulsbo usa clock nominal 200000000. Linux obtém core_freq por transação de configuração PCI no host bridge: escreve seletor 0xd0050300 em 0xd0, lê 0xd4 e decodifica clock&7 em 100/133/150/178/200/266. Fontes: [PINFO:46-51](poulsbo-data/../archaeology-data/PINFO.txt); [CLOCK:11-51](poulsbo-data/CLOCK.txt).

**CONFIRMED [P3-034]** — psb_spank do Linux aciona reset BIF/DPM/TA/USE/ISP/TSP/2D, espera 1 ms, libera reset, alterna CLEAR_FAULT e escreve a base 2D. psb_init_pm altera somente o campo de gating 2D para 1. Fontes: [LDRVC:103-125](poulsbo-data/../archaeology-data/LDRVC.txt); [DEVICE:85-95](poulsbo-data/DEVICE.txt).

**CONFIRMED [P3-035]** — Os callbacks Poulsbo psb_power_up/down retornam 0 sem sequência de island power. power.c mantém referência runtime-PM por suporte descrito como quebrado, e contém suspend/resume PCI D3hot/D0 com restauração de display/GTT/GEM. Fontes: [DEVICE:186-194](poulsbo-data/DEVICE.txt); [POWER:46-70](poulsbo-data/POWER.txt); [POWER:95-203](poulsbo-data/POWER.txt).

**CONFIRMED [P3-036]** — Sob SUPPORT_DRI_DRM_EXT, DDK Poulsbo adquire display antes de graphics por ospm_power_using_hw_begin, desfaz display se graphics falha e libera graphics antes de display. Inclui psb_powermgmt.h e sys_pvr_drm_export.h. Fontes: [PSYSC:65-69](poulsbo-data/../archaeology-data/PSYSC.txt); [PSYSC:1916-1979](poulsbo-data/../archaeology-data/PSYSC.txt).

**CONFIRMED [P3-037]** — O DDK associa revisão SGX535 121 a BRN22934/23944/23410 e 126 a BRN22934. O reset tem ramo BRN23944 que pausa BIF/limpa fault e uma drenagem de faults com PD temporária antes de restaurar o contexto. Fontes: [ERRATA:152-178](poulsbo-data/../archaeology-data/ERRATA.txt); [RESET:471-495](poulsbo-data/../archaeology-data/RESET.txt); [RESET:544-655](poulsbo-data/../archaeology-data/RESET.txt).

**CONFIRMED [P3-038]** — No espelho EMGD, pwr_set_plb não programa transições D0–D3, enquanto pwr_init_plb escreve CLKGATECTL=0x1111111 e o registrador em +8 com zero. Os hooks SysDevicePre/PostPowerState do common/sysconfig.c apenas registram mensagens. Fontes: [EMGD_drm_emgd_state_power_plb_pwr_plb_c:83-123](poulsbo-data/EMGD_drm_emgd_state_power_plb_pwr_plb_c.txt); [EMGD_drm_pvr_services4_system_common_sysconfig_c:1332-1365](poulsbo-data/EMGD_drm_pvr_services4_system_common_sysconfig_c.txt).

## O que falta reconstruir

**UNKNOWN:** sequência completa de clock/rail/reset a partir de cada estado de energia, hardware por trás das APIs OSPM, bits de ready e seus timeouts, alcance do reset sobre display/memória e errata do stepping instalado. Os stubs não demonstram que power gating é desnecessário; podem depender de outro componente/firmware de plataforma.

**INFERRED:** o caminho atual de probe já toca a SGX; carregar/descarregar gma500 não é experimento passivo. A operação que Linux chama de obter clock também escreve PCI (P3-033); não deve entrar em inventário somente leitura. A constante 200 MHz não substitui a detecção nem autoriza programação de PLL.

O reset do DDK é parte de uma inicialização com scripts e estado do microkernel (P3-051). Copiá-lo isoladamente não fornece procedimento de bring-up. A drenagem em loop deve ter saída limitada e diagnóstico em um projeto futuro, mas esta fase não altera o código nem propõe valores novos.

Para recuperação inicial, planejar reboot controlado e possibilidade de power cycle, sem depender de que um soft reset recupere o display. A recuperabilidade por reboot ainda precisa ser demonstrada na plataforma alvo. Não efetuar testes de suspend/resume ou gating antes dessa preparação.

