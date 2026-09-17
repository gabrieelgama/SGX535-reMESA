# Auditoria de segurança de leituras de registradores

## Critério

`SAFE-CANDIDATE` requer, ao mesmo tempo: offset Poulsbo/SGX535 confirmado,
semântica de leitura explícita, ausência conhecida de clear/ack/latch/FIFO,
power/clock conhecido e ownership/locking resolvido. Nenhum registrador atende
a todos os requisitos. A tabela não é uma whitelist executável.

| offset | name | purpose | platform | source | read-semantics | known-side-effects | power-requirement | ownership | confidence | phase4-status |
|---:|---|---|---|---|---|---|---|---|---|---|
| `0x0010` | `CORE_ID` | identificação/configuração | Poulsbo | Linux `psb_reg.h:31-35`; PSB histórico `PSB_psb_drv_c.txt:325-339`; DDK `INIT.txt:1320-1352` | lido historicamente | contrato ausente | SGX powered; como provar UNKNOWN | gma500 SGX mapping | offset CONFIRMED; safety UNKNOWN | UNKNOWN |
| `0x0014` | `CORE_REVISION` | designer/major/minor/maintenance | Poulsbo | mesmos artefatos; EMGD `sysconfig.c:547-575`; DDK `INIT.txt:1320-1352` | lido historicamente | contrato ausente | SGX powered; como provar UNKNOWN | gma500 SGX mapping | offset CONFIRMED; safety UNKNOWN | UNKNOWN |
| `0x0018` | `DESIGNER_REV_FIELD1` | revisão designer | Poulsbo | Linux `psb_reg.h:47` | não localizada | UNKNOWN | UNKNOWN | gma500 | offset CONFIRMED | UNKNOWN |
| `0x001c` | `DESIGNER_REV_FIELD2` | revisão designer | Poulsbo | Linux `psb_reg.h:58` | não localizada | UNKNOWN | UNKNOWN | gma500 | offset CONFIRMED | UNKNOWN |
| `0x0080` | `SOFT_RESET` | controla resets SGX | Poulsbo | Linux `psb_reg.h:49-56`; `psb_drv.c:103-124` | leitura usada como posted read | escrita altera vários blocos | UNKNOWN | gma500 init | action CONFIRMED | UNSAFE |
| `0x0110` | `EVENT_HOST_ENABLE2` | máscara IRQ SGX2 | Poulsbo | Linux `psb_reg.h:60-65`; `psb_irq.c:250-305` | readback post-write | corrida com IRQ/máscara | SGX IRQ active | `irqmask_lock` | use CONFIRMED | UNSAFE |
| `0x0114` | `EVENT_HOST_CLEAR2` | acknowledge IRQ SGX2 | Poulsbo | Linux `psb_reg.h:64-65`; `psb_irq.c:192-195` | somente readback post-write observado | write-to-clear; read effect UNKNOWN | SGX IRQ active | IRQ handler | use CONFIRMED | UNSAFE |
| `0x0118` | `EVENT_STATUS2` | status IRQ SGX2 | Poulsbo | Linux `psb_reg.h:62`; `psb_irq.c:198-239` | lido no handler | concorrência/latch UNKNOWN | SGX IRQ active | `irqmask_lock` | use CONFIRMED | UNKNOWN |
| `0x012c` | `EVENT_STATUS` | status IRQ SGX | Poulsbo | Linux `psb_reg.h:67`; `psb_irq.c:198-239` | lido no handler | concorrência/latch UNKNOWN | SGX IRQ active | `irqmask_lock` | use CONFIRMED | UNKNOWN |
| `0x0130` | `EVENT_HOST_ENABLE` | máscara IRQ SGX | Poulsbo | Linux `psb_reg.h:69`; `psb_irq.c:250-305` | readback post-write | concorrência com IRQ | SGX IRQ active | `irqmask_lock` | use CONFIRMED | UNSAFE |
| `0x0134` | `EVENT_HOST_CLEAR` | acknowledge IRQ SGX | Poulsbo | Linux `psb_reg.h:71-84`; `psb_irq.c:192-195` | read não documentado | write-to-clear | SGX IRQ active | IRQ handler | use CONFIRMED | UNSAFE |
| `0x0ab8` | `PDS_EXEC_BASE` | base PDS | Poulsbo | Linux `psb_reg.h:116-118`; `psb_drv.c:361` | não localizada | controle de execução/endereço | UNKNOWN | gma500 init | offset/use CONFIRMED | UNSAFE |
| `0x0ac4` | `EVENT_KICKER` | endereço de kick | Poulsbo | Linux `psb_reg.h:120-121` | não localizada | associado à submissão | UNKNOWN | ABI histórica | offset CONFIRMED | UNSAFE |
| `0x0ac8` | `EVENT_KICK` | dispara evento | Poulsbo | Linux `psb_reg.h:123-124` | não localizada | write inicia ação | UNKNOWN | firmware/command path | offset CONFIRMED | UNSAFE |
| `0x0c00` | `BIF_CTRL` | fault clear/cache/TLB controls | Poulsbo | Linux `psb_reg.h:128-130`; `mmu.c:69-120` | RMW/readback usado | flush/invalidate/clear fault em write; read side UNKNOWN | UNKNOWN | `mmu->sem` | use CONFIRMED | UNSAFE |
| `0x0c38` | `BIF_DIR_LIST_BASE1` | base/contexto MMU | Poulsbo | Linux `psb_reg.h:126`; `mmu.c:123-135` | não localizada isoladamente | write muda address space | UNKNOWN | `mmu->sem` | offset CONFIRMED | UNSAFE |
| `0x0c04` | `BIF_INT_STAT` | tipo/requester da falha | Poulsbo histórico/Linux | Linux `psb_reg.h:133-147`; `psb_irq.c:159-188` | lido em IRQ de fault | clear/latch UNKNOWN | SGX IRQ active | IRQ handler | use CONFIRMED | UNKNOWN |
| `0x0c08` | `BIF_FAULT` | endereço de falha | Poulsbo histórico/Linux | Linux `psb_reg.h:135-147`; `psb_irq.c:159-188`; DDK `INIT.txt:1353-1364` | lido em fault/debug | clear/latch UNKNOWN | SGX powered UNKNOWN | IRQ/debug owner | use CONFIRMED | UNKNOWN |
| `0x0000` | `CLKGATECTL` | clock gate | Poulsbo | Linux `psb_reg.h:13-29`; `psb_device.c:85-94` | RMW/readback | write altera gating; read effect UNKNOWN | é o próprio controle de clocks | gma500 PM | use CONFIRMED | UNSAFE |
| `0x0e04` | `2D_BLIT_STATUS` | status 2D | Poulsbo | Linux `psb_reg.h:160-163`; `psb_irq.c:156-158` | lido ao completar 2D | clear/latch UNKNOWN | clock 2D | IRQ handler | use CONFIRMED | UNKNOWN |

**CONFIRMED — P4-014/P4-015:** três famílias históricas leem ID/revision: PSB,
DDK Poulsbo e EMGD. **INFERRED:** ID/revision são os melhores alvos de pesquisa
documental. **UNKNOWN:** se são seguros para o primeiro acesso real. Repetição
de uma prática histórica não substitui datasheet/errata ou contrato do IP.

Não foi encontrada evidência explícita de clear-on-read para os dois IDs; também
não foi encontrada evidência explícita de ausência desse comportamento. Por
isso não há `SAFE-CANDIDATE`.
