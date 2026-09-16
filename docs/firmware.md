# Firmware / microkernel

> Atualização da fase 2: o histórico local contém `sgx535defs.h` e uma integração Poulsbo explícita no DDK 1.14. As referências à ausência abaixo descrevem o checkout master da fase 1. Consulte [arqueologia](source-archaeology.md), [arquivos recuperados](sgx535-missing-files.md) e [comparação Poulsbo](poulsbo-evidence.md) para o estado ampliado.

## CONFIRMED — o que o host sabe

O KM recebe handles para kernel CCB, controle, event-kicker, host-control e TA/3D-control, além de `aui32HostKickAddr`, scripts, build options e tamanhos de estruturas. [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h:85-114](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h#L85). `InitDevInfo` associa esses buffers ao estado KM e copia scripts e handlers. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:207-293](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L207).

Os scripts são listas de operações WRITE_HW_REG, READ_HW_REG e HALT, com operação adicional condicionada por PDUMP. Há duas listas init de 64 entradas e uma deinit de 16. O executor está no KM; as listas preenchidas não foram recuperadas como fonte SGX535. [references/omap5-sgx-ddk-linux/eurasia_km/include4/sgxscript.h:49-89](../references/omap5-sgx-ddk-linux/eurasia_km/include4/sgxscript.h#L49); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:327-370](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L327).

A inicialização roda script parte 1 antes do reset e parte 2 depois, zera `ui32InitStatus`, faz kick e espera bit INIT_COMPLETE com timeout. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:478-647](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L478). O bit é `1<<0`. [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:289](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L289). Portanto, ter `sgxreset.c` não equivale a ter uma sequência de boot completa.

O DDK checa build options entre cliente/KM, versão/build do microkernel, revisão do core e tamanhos das estruturas. Existem exceções explícitas e caso de revisão HEAD. Não declarar os binários UM compatíveis apenas porque todos usam a etiqueta “1.9”. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:2489-2649](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L2489).

## Artefatos UM observados, sem execução

**CONFIRMED como metadados ELF**, não semântica das instruções: `pvrsrvinit`, `libsrv_init.so.1.9.6.0` e `libusc.so.1.9.6.0` são ARM ELF32 little-endian e possuem símbolos/debug info. A saída reproduzível de `file`/`readelf -Ws`, SHA-256 e commit estão em [evidence.txt](evidence.txt).

Na tabela de símbolos de `libsrv_init.so.1.9.6.0`:

| Símbolo | Valor ELF | Tamanho | Interpretação limitada |
| --- | --- | --- | --- |
| `pbuKernelProgram` | `0x00001eb8` | 60208 | objeto local com nome de programa microkernel |
| `pbSlaveuKernelProgram` | `0x000109e8` | 11552 | objeto local com nome de programa slave |
| `g_pui32PDSUKERNEL_INIT_PRIM1` | `0x00013750` | 24 | objeto PDS nomeado |
| `g_pui32PDSUKERNEL_INIT_SEC` | `0x00013768` | 56 | objeto PDS nomeado |
| `g_pui32PDSUKERNEL_INIT_PRIM2` | `0x000137a0` | 24 | objeto PDS nomeado |
| `g_pui32PDSUKERNEL_EVENTS` | `0x000137b8` | 252 | objeto PDS nomeado |

Valores ELF não são offsets de arquivo nem endereços GPU prontos para uso. **INFERRED:** a biblioteca carrega conteúdo de programas microkernel/PDS embutidos; **UNKNOWN:** compatibilidade SGX535, formato, relocação e código de inicialização exato. O alvo OMAP5430 do KM seleciona SGX544, reforçando que esses objetos não devem ser tratados como firmware Poulsbo. [references/omap5-sgx-ddk-linux/eurasia_km/eurasiacon/build/linux2/omap5430_linux/Makefile:112-113](../references/omap5-sgx-ddk-linux/eurasia_km/eurasiacon/build/linux2/omap5430_linux/Makefile#L112).

## Energia e recuperação no contrato

**CONFIRMED:** host-control contém estados de power, cleanup, init e IRQ. Comandos POWER distinguem POWEROFF, IDLE e RESUME; pre-power envia comando, espera status e interrupções pendentes, verifica clock gating e pode rodar deinit. Post-power reinicializa ou envia RESUME conforme transição. [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:103-148](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L103); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:295-350](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L295); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxpower.c:295-418](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxpower.c#L295); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxpower.c:444-503](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxpower.c#L444).

**CONFIRMED:** o ramo não-MP força o caminho de recovery também na primeira inicialização (`bHardwareRecovery |= bFirstTime`). [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:499-503](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L499). **UNKNOWN:** se isso deve ser preservado para Poulsbo e qual estado anterior o firmware de plataforma deixa.

## Fonte e reutilização

O manifesto UM é um documento OLE, não fonte C. O inventário registra trechos localizados por offsets de extração ASCII sobre o arquivo original; a listagem não equivale a extração completa do Word. Ela identifica TI TSPA, distribuição binária sem modificação e restrição de engenharia reversa. Nenhum firmware foi copiado para uma nova implementação.

**UNKNOWN:** disponibilidade de microkernel SGX535 com fonte/licença compatível, possibilidade de um microkernel mínimo novo e conjunto de tarefas obrigatório. Uma simples escrita de EVENT_KICK sem programa e memória válidos não é experimento seguro. Próximas fontes: init UM publicado legitimamente, fontes do microkernel/PDS e especificação correspondente à revisão identificada, com auditoria de proveniência antes do uso.
