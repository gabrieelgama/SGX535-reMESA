# Registradores: subconjunto documentado para Poulsbo

> Atualização da fase 2: o histórico local contém `sgx535defs.h` e uma integração Poulsbo explícita no DDK 1.14. As referências à ausência abaixo descrevem o checkout master da fase 1. Consulte [arqueologia](source-archaeology.md), [arquivos recuperados](sgx535-missing-files.md) e [comparação Poulsbo](poulsbo-evidence.md) para o estado ampliado.

## Origem e confiança

**CONFIRMED no código Linux**, sem teste no silício. Todas as linhas da tabela vêm de `psb_reg.h` do commit fixado em [architecture.md](architecture.md). “Função” descreve nome/comentário e, quando indicado, uso observado; não especifica todos os efeitos, permissões RO/RW, valores reset, bits reservados ou acesso seguro.

Offsets abaixo são relativos à janela SGX. Em Poulsbo o driver mapeia **recurso PCI 0 + 0x40000**, extensão `0x8000`, selecionada por `psb_chip_ops`. Não são endereços físicos absolutos. [references/linux/drivers/gpu/drm/gma500/psb_drv.h:41-53](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L41); [references/linux/drivers/gpu/drm/gma500/psb_device.c:272](../references/linux/drivers/gpu/drm/gma500/psb_device.c#L272); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:261-264](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L261).

**UNKNOWN:** mapa SGX535 completo no TI, pois falta `sgx535defs.h`, embora seja incluído pelo dispatcher. Não substituímos por valores SGX530/540/544. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxdefs.h:54-58](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxdefs.h#L54); [inventário](evidence.txt).

| offset | name | bits | function | source | confidence |
| --- | --- | --- | --- | --- | --- |
| `0x0000` | `PSB_CR_CLKGATECTL` | 24 auto/manual; USE 21:20; DPM 17:16; TA 13:12; TSP 9:8; ISP 5:4; 2D 1:0 | Campos de clock; códigos 0 enabled, 1 disabled, 2 auto | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:13-29](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L13) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0010` | `PSB_CR_CORE_ID` | 31:16 ID; 15:0 config | Identificação declarada; valor esperado SGX535 UNKNOWN | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:31-35](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L31) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0014` | `PSB_CR_CORE_REVISION` | 31:24 designer; 23:16 major; 15:8 minor; 7:0 maintenance | Campos de revisão; relação com revisão DDK UNKNOWN | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:37-45](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L37) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0018 / 0x001c` | `PSB_CR_DESIGNER_REV_FIELD1 / FIELD2` | UNKNOWN | Nomes e offsets; sem decodificação | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:47-58](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L47) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0080` | `PSB_CR_SOFT_RESET` | 6 TSP; 5 ISP; 4 USE; 3 TA; 2 DPM; 1 2D; 0 BIF | Reset de blocos; utilizado em psb_spank | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:49-56](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L49) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0110` | `PSB_CR_EVENT_HOST_ENABLE2` | 4 BIF_REQUESTER_FAULT | Habilitação grupo 2; escrita observada no postinstall | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:60-65](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L60) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0114` | `PSB_CR_EVENT_HOST_CLEAR2` | 4 BIF_REQUESTER_FAULT | Acknowledge grupo 2 no handler | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:64-65](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L64) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0118` | `PSB_CR_EVENT_STATUS2` | 4 BIF_REQUESTER_FAULT | Status grupo 2 lido pelo handler | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:62-65](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L62) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x012c` | `PSB_CR_EVENT_STATUS` | ver eventos abaixo | Status grupo principal | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:67-84](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L67) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0130` | `PSB_CR_EVENT_HOST_ENABLE` | ver eventos abaixo | Enable; Linux habilita 2D_COMPLETE | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:69-84](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L69) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0134` | `PSB_CR_EVENT_HOST_CLEAR` | ver eventos abaixo | Acknowledge escrito pelo handler | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:71-84](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L71) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0a0c + 4*i; i=0..15` | `PSB_CR_USE_CODE_BASE(i)` | 26:25 DM; 24:0 base; alignshift=7 | Bases USE: vertex/pixel/EDM; comentário de endereço incerto | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:87-114](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L87) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0ab8` | `PSB_CR_PDS_EXEC_BASE` | ADDR_SHIFT=20; ALIGNSHIFT=20 | Base PDS; init Linux escreve 0x20000000 | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:116-118](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L116) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0ac4` | `PSB_CR_EVENT_KICKER` | ADDRESS_SHIFT=4 | Endereço do kicker conforme macro; formato completo UNKNOWN | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:120-121](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L120) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0ac8` | `PSB_CR_EVENT_KICK` | 0 NOW | Kick definido; ausência de emissão no gma500 inspecionado | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:123-124](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L123) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0c00` | `PSB_CR_BIF_CTRL` | 4 CLEAR_FAULT; 3 INVALDC; 2 FLUSH | Controle BIF; invalidação e clear fault usados | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:128-131](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L128) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0c04` | `PSB_CR_BIF_INT_STAT` | 14 PF_N_RW; 13:0 fault; requestors abaixo | Classificação de falha lida pelo IRQ | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:133-147](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L133) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0c08` | `PSB_CR_BIF_FAULT` | UNKNOWN como bitfield; lido como endereço | Endereço que falhou segundo o handler, não flags requestor | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:135-147](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L135) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0c38` | `PSB_CR_BIF_DIR_LIST_BASE1` | UNKNOWN | Offset do define; contexto 1 do Linux escreve 0x0c3c | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:126-126](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L126) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0c78 / 0x0c7c` | `PSB_CR_BIF_BANK0 / BANK1` | UNKNOWN | Bancos; init Linux escreve zero | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:149-150](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L149) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0c84` | `PSB_CR_BIF_DIR_LIST_BASE0` | Linux escreve PFN PD << PAGE_SHIFT | Diretório do contexto 0 | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:151-151](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L151) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0c88` | `PSB_CR_BIF_TWOD_REQ_BASE` | UNKNOWN como máscara | Base requestor 2D; Linux escreve gatt_start | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:152-152](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L152) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0cac` | `PSB_CR_BIF_3D_REQ_BASE` | UNKNOWN como máscara | Base requestor 3D; Linux escreve 0x30000000 | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:153-153](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L153) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0e04` | `PSB_CR_2D_BLIT_STATUS` | 24 BUSY; 23:0 COMPLETE | Status 2D lido no IRQ | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:160-163](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L160) | CONFIRMED no Linux; campos UNKNOWN explícitos |
| `0x0e18` | `PSB_CR_2D_SOCIF` | 7:0 FREESPACE; EMPTY=0x80 | Definições de espaço/empty; sem submissão reconstruída | [references/linux/drivers/gpu/drm/gma500/psb_reg.h:155-158](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L155) | CONFIRMED no Linux; campos UNKNOWN explícitos |

## Eventos e faults

**CONFIRMED:** máscara de eventos principal: bit 31 MASTER_INTERRUPT; 28 TA_DPM_FAULT; 27 TWOD_COMPLETE; 25 DPM_OUT_OF_MEMORY_ZLS; 24 DPM_TA_MEM_FREE; 18 PIXELBE_END_RENDER; 14 SW_EVENT; 13 TA_FINISHED; 12 TA_TERMINATE; 3 DPM_REACHED_MEM_THRESH; 2 DPM_OUT_OF_MEMORY_GBL; 1 DPM_OUT_OF_MEMORY_MT; 0 DPM_3D_MEM_FREE. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:72-84](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L72).

**CONFIRMED:** BIF_INT_STAT requestors definidos: 1 CACHE, 2 TA, 3 VDM, 4 2D, 5 PBE, 6 TSP, 7 ISP, 8 USSEPDS, 9 HOST. Embora os defines apareçam depois de `BIF_FAULT`, o uso do handler aplica as máscaras ao valor de **BIF_INT_STAT** e trata BIF_FAULT como endereço. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:133-147](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L133); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:159-188](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L159).

**CONFIRMED:** gma500 escreve os valores de status nos clear registers e lê CLEAR2 para postagem; habilita apenas os eventos 2D e fault BIF no postinstall. Isso documenta o padrão usado pelo driver, não garante que qualquer escrita de bits reservados seja permitida. [references/linux/drivers/gpu/drm/gma500/psb_irq.c:192-195](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L192); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:286-289](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L286).

## Outros espaços: não misturar

| Espaço | Valor | Significado confirmado | Fonte |
| --- | --- | --- | --- |
| Janela SGX | `0x4000` | define slave port 2D; não é opcode | [references/linux/drivers/gpu/drm/gma500/psb_drv.h:61](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L61) |
| Configuração PCI | `0x52`, `0x5c` | GMCH_CTRL, BSM | [references/linux/drivers/gpu/drm/gma500/psb_drv.h:55-58](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L55) |
| VDC MMIO | `0x2020` | PGETBL_CTL; habilitação GTT | [references/linux/drivers/gpu/drm/gma500/psb_drv.h:59-60](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L59); [references/linux/drivers/gpu/drm/gma500/gtt.c:128-144](../references/linux/drivers/gpu/drm/gma500/gtt.c#L128) |
| Endereços virtuais escritos na init | `0x20000000`, `0x30000000` | valores para PDS_EXEC_BASE e BIF_3D_REQ_BASE, não offsets MMIO | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:361-362](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L361) |
| ABI CCB | `SGXMKIF_CMD_*` | enum de software, não registradores | [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h:65-82](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h#L65) |

Os defines de pacotes 2D a partir de [references/linux/drivers/gpu/drm/gma500/psb_reg.h:169-190](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L169) são dados de comandos e não devem ser acrescentados como offsets de registradores.

## UNKNOWN / pontos de revisão

- RO/RW, clear-on-read, W1C formal, reset values e reservas não são inferidos só dos nomes.
- CORE_ID/REVISION são candidatos para futura identificação, não uma whitelist de acesso aprovada nesta etapa.
- A indexação de BASE1 diverge entre Linux e TI; detalhes em [mmu-bif.md](mmu-bif.md).
- O reset básico usa os bits acima, mas firmware boot e recovery exigem estado adicional: [references/linux/drivers/gpu/drm/gma500/psb_drv.c:103-125](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L103) versus [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:467-665](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L467).
