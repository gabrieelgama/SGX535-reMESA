# MMU e BIF

> Atualização da fase 2: o histórico local contém `sgx535defs.h` e uma integração Poulsbo explícita no DDK 1.14. As referências à ausência abaixo descrevem o checkout master da fase 1. Consulte [arqueologia](source-archaeology.md), [arquivos recuperados](sgx535-missing-files.md) e [comparação Poulsbo](poulsbo-evidence.md) para o estado ampliado.

## CONFIRMED — formato descrito pelas fontes

O DDK define páginas de 4 KiB (`PAGE_SHIFT=12`), índices PD/PT de 10 bits, máscaras `0xffc00000` e `0x003ff000`. O ramo sem `SGX_FEATURE_36BIT_MMU` usa endereço PDE/PTE `0xfffff000` sem deslocamento adicional. A seleção SGX535 declara VA de 32 bits e não habilita essa feature de 36 bits. Não confundir largura virtual com capacidade física de toda integração. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxmmu.h:48-93](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxmmu.h#L48); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h:63-73](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h#L63).

O Linux usa índice PD `va >> 22` e índice PT `(va >> 12) & 0x3ff`; compõe PTE com PFN deslocado e flags. [references/linux/drivers/gpu/drm/gma500/mmu.c:44-52](../references/linux/drivers/gpu/drm/gma500/mmu.c#L44); [references/linux/drivers/gpu/drm/gma500/mmu.c:146-158](../references/linux/drivers/gpu/drm/gma500/mmu.c#L146); [references/linux/drivers/gpu/drm/gma500/psb_drv.h:75-84](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L75).

| Campo | DDK | Linux Poulsbo | Confiança |
| --- | --- | --- | --- |
| valid | bit 0 | bit 0 | CONFIRMED nas duas fontes |
| write-only | bit 1 | bit 1 | CONFIRMED nas duas fontes |
| read-only | bit 2 | bit 2 | CONFIRMED nas duas fontes |
| cache | bit 3, CACHECONSISTENT | bit 3, comentário CPU cache coherent | CONFIRMED como definição; coerência completa UNKNOWN |
| EDM protect | bit 4 | sem equivalente nesses defines | CONFIRMED apenas no header genérico TI |

Fontes da tabela: [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxmmu.h:66-93](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxmmu.h#L66); [references/linux/drivers/gpu/drm/gma500/psb_drv.h:81-84](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L81). O header TI também enumera tamanhos PDE de 16 KiB a 4 MiB; **UNKNOWN** se são utilizáveis no SGX535/Poulsbo. A presença desses defines em arquivo comum não prova suporte do core.

**INFERRED:** com o formato de 4 KiB, 1024 PDEs × 1024 PTEs × 4096 bytes cobrem 4 GiB de VA, com cada PT cobrindo 4 MiB. É aritmética sobre os campos acima, não quantidade de RAM.

## Contextos e divergência importante

**CONFIRMED:** o DDK SGX535 declara 16 directory lists; `SGX_BIF_DIR_LIST_INDEX_EDM` seleciona a última, portanto índice 15 nessa configuração. O reset associa EDM e 2D ao contexto kernel; sob BRN23410, associa TA também. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h:67-69](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h#L67); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:357-361](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L357); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:164-203](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L164).

**CONFIRMED:** Linux usa contexto 0 para PD padrão e 1 para `pf_pd`; os PDs alocados com `trap_pagefaults=1` usam entradas inválidas zero. [references/linux/drivers/gpu/drm/gma500/psb_drv.c:340-359](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L340); [references/linux/drivers/gpu/drm/gma500/mmu.c:160-210](../references/linux/drivers/gpu/drm/gma500/mmu.c#L160).

As fórmulas não coincidem:

| Fonte | Seleção de registrador |
| --- | --- |
| TI | base0 para 0; base1 + `4*(índice-1)` para os demais |
| Linux | base0 para 0; base1 + `4*hw_context` para os demais |

[references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:121-135](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L121); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:183-203](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L183); [references/linux/drivers/gpu/drm/gma500/mmu.c:123-135](../references/linux/drivers/gpu/drm/gma500/mmu.c#L123).

No Linux, `BASE1=0x0c38`, de modo que contexto 1 escreve em **0x0c3c**; esta conta é **INFERRED diretamente da expressão**, e não correção sugerida. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:126](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L126). **UNKNOWN:** diferença de convenção, definição histórica ou defeito; não escolher a fórmula TI por semelhança. Falta o header SGX535 para uma comparação completa.

## Endereços e mapeamento

**CONFIRMED:** o Linux mantém GTT e MMU SGX separadas. Pin de GEM obtém páginas, marca WC, insere na GTT e no PD SGX em `gatt_start + offset`. Unpin remove de ambas e restaura WB. O armazenamento GEM é limitado a páginas DMA32. [references/linux/drivers/gpu/drm/gma500/gem.c:29-107](../references/linux/drivers/gpu/drm/gma500/gem.c#L29); [references/linux/drivers/gpu/drm/gma500/gem.c:175-178](../references/linux/drivers/gpu/drm/gma500/gem.c#L175).

**CONFIRMED:** a inicialização insere stolen memory no PD padrão, escreve `PDS_EXEC_BASE=0x20000000` e `BIF_3D_REQ_BASE=0x30000000`. `gtt.c` mantém ainda `mmu_gatt_start=0xe0000000`, separado de `gatt_start` obtido do recurso PCI. Não usar esses nomes como sinônimos. [references/linux/drivers/gpu/drm/gma500/psb_drv.c:352-362](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L352); [references/linux/drivers/gpu/drm/gma500/gtt.c:185-250](../references/linux/drivers/gpu/drm/gma500/gtt.c#L185).

**CONFIRMED:** a integração OMAP5 implementa conversões CPU físico ↔ sistema ↔ dispositivo como identidade, explicitando a suposição UMA. Isso não estabelece uma regra universal para SGX. [references/omap5-sgx-ddk-linux/eurasia_km/services4/system/omap5/sysconfig.c:827-935](../references/omap5-sgx-ddk-linux/eurasia_km/services4/system/omap5/sysconfig.c#L827).

O DDK separa heaps de dados, parâmetros 3D, TA, sync, código/dados PDS, código/dados kernel e shaders. No ramo de 32 bits sem BRN31620, exemplos são sync `0xef000000`, kernel code `0xf2000000`, kernel data `0xf4000000 + offset`, pixel shader `0xf9000000`, vertex shader `0xfe000000`. **CONFIRMED como política do DDK**, não endereços físicos fixos do hardware. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h:67-68](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h#L67); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h:195-248](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h#L195).

## Invalidação e sincronização

**CONFIRMED:** o MMU TI acumula flags de invalidação PD/PT em `ui32CacheControl`; o scheduler as transfere ao comando e limpa o acumulador. As flags são pedidos ao microkernel, não offsets MMIO. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/mmu.c:591-643](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/mmu.c#L591); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxutils.c:454-466](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxutils.c#L454); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:367-370](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L367).

**CONFIRMED:** Linux invalida por `BIF_CTRL`, alternando INVALDC ou FLUSH e fazendo barreira e leitura de retorno. Possui caminho CPU `clflush`; isso não prova coerência de todos os dados e caches de shader. [references/linux/drivers/gpu/drm/gma500/mmu.c:54-120](../references/linux/drivers/gpu/drm/gma500/mmu.c#L54).

## Faults e recovery

**CONFIRMED:** o handler Linux lê status BIF e endereço de fault, distingue page fault/proteção e imprime o requestor. Depois limpa os eventos. Não há ali paginação sob demanda ou replay de job. [references/linux/drivers/gpu/drm/gma500/psb_irq.c:151-196](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L151). A inserção de PTE em IRQ é apenas TODO em [references/linux/drivers/gpu/drm/gma500/mmu.c:35-42](../references/linux/drivers/gpu/drm/gma500/mmu.c#L35).

**CONFIRMED:** o reset TI não-MP drena requests através de PD/PT/página temporários e repete até não haver fault. O comentário sobre endereço relativo de 2 GiB e bus-master MSB está nesse contexto específico; **UNKNOWN** se descreve Poulsbo, logo não deve virar regra de PTE. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:544-618](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L544).

**UNKNOWN:** ordenação end-to-end, segurança de invalidação com engines ativas, isolamento entre contextos, política de EDM protect e limites DMA reais no alvo. Resolver antes de permitir ao SGX tocar memória nova.

## Camadas de mapeamento no DDK TI

**CONFIRMED:** o buffer manager reserva VA do dispositivo via `pfnMMUAlloc` e escolhe funções de mapeamento conforme origem da memória. No ramo contíguo passa endereço físico convertido e VA de destino para `pfnMMUMapPages`; há ramos separados sparse/shadow. Isso distingue alocação de VA GPU do mapeamento CPU. [services4/srvkm/common/buffer_manager.c:2294–2380](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/common/buffer_manager.c#L2294).

**CONFIRMED:** `OSMapPhysToLin` exige `PVRSRV_HAP_KERNEL_ONLY` e usa wrappers de ioremap/LinuxMemArea para produzir VA CPU. Não é a função que cria a tradução BIF. [services4/srvkm/env/linux/osfunc.c:1647–1687](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/env/linux/osfunc.c#L1647).

**CONFIRMED, integração OMAP:** `create_gem_wrapper`, no ramo `SUPPORT_DRI_DRM_EXTERNAL`, converte LinuxMemArea em páginas ou região física, traduz flags cached/WC/uncached em flags `OMAP_BO_*` e chama `omap_gem_new_ext`. É dependência concreta da integração TI dentro do código de ambiente Linux, não somente em `services4/system/omap5`. [services4/srvkm/env/linux/mmap.c:372–482](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/env/linux/mmap.c#L372). **UNKNOWN:** política equivalente para compartilhar BOs de renderização com scanout Poulsbo. Não portar esse wrapper para Intel só por utilizar GEM.
