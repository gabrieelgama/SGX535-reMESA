# USSE / USE / PDS

> Atualização da fase 2: o histórico local contém `sgx535defs.h` e uma integração Poulsbo explícita no DDK 1.14. As referências à ausência abaixo descrevem o checkout master da fase 1. Consulte [arqueologia](source-archaeology.md), [arquivos recuperados](sgx535-missing-files.md) e [comparação Poulsbo](poulsbo-evidence.md) para o estado ampliado.

## CONFIRMED

A configuração SGX535 declara duas USE pipes; SGX543/544 habilitam flags como `USE_NO_INSTRUCTION_PAIRING` e `USE_UNLIMITED_PHASES` que não aparecem no ramo SGX535. Isso impede importar automaticamente hipóteses de ISA dos cores mais novos. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h:63-118](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h#L63).

O Linux define 16 bases USE de `0x0a0c` a `0x0a48`, campo DM em bits 26:25 com nomes VERTEX=0, PIXEL=1, RESERVED=2, EDM=3, campo de base em bits 24:0 e alignshift 7. O comentário do endereço contém uma interrogação; o valor da macro é confirmado, mas a interpretação física precisa de confirmação adicional. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:87-114](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L87).

O registro PDS_EXEC_BASE está em `0x0ab8`, com shift/alinhamento declarado de 20 bits; o Linux escreve `0x20000000` durante init. Isso não documenta instruções PDS. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:116-118](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L116); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:361](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L361).

O comando kernel tem `ui32ServiceAddress` descrito como endereço do handler USE. O host-control usa nomes `PVRSRV_USSE_EDM_*`, e o DDK reserva heaps distintos para kernel code, pixel/vertex shaders e PDS code/data. [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:70-75](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L70); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:289-306](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L289); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h:224-248](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h#L224).

## INFERRED

O microkernel deste modelo utiliza a infraestrutura de execução USE/USSE/EDM, em vez de termos evidência aqui de uma CPU de firmware separada. A inferência se apoia no endereço de handler USE e nos campos USSE_EDM acima. Não especifica por si só como PDS dispara o programa, quais registradores o inicializam ou como instruções são codificadas.

## UNKNOWN

Não foi localizado fonte de assembler/disassembler USSE, descrição completa de opcodes, compilador de shaders ou fonte dos programas microkernel/PDS nas duas árvores TI. O inventário em [evidence.txt](evidence.txt) permite repetir essa busca. `libusc` e `libglslcompiler` são artefatos UM; seu nome não é documentação de ISA. O pacote se descreve como bibliotecas/binários para OMAP. [references/omap5-sgx-ddk-um-linux/README:20-25](../references/omap5-sgx-ddk-um-linux/README#L20).

Faltam: tamanho/codificação efetiva das instruções SGX535; pairing e hazards; bancos de registradores e operandos; branches/predicação; terminadores de tarefa; cargas/stores e endereçamento; sincronização de caches; descritores de textura e sampler; exportação de vertex/pixel e interface PBE. Não atribuímos valores a esses campos.

## Próxima investigação

Buscar publicação autorizada e específica SGX535 de USE/USSE e PDS, incluindo revisão. Examinar primeiro os contratos KM já disponíveis e a origem de `sgx535defs.h`; não usar SGX544 como especificação substituta. Os metadados UM inventariados em [firmware.md](firmware.md) servem para identificar componentes, não para importar payload. A decisão de escrever um assembler ou compilador deve esperar a especificação mínima de execução e uma licença adequada das novas fontes.
