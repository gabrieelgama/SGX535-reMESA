# Estado do Linux gma500 para Poulsbo

Escopo: checkout local Linux `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f`. Este documento não afirma que o kernel rodando no alvo seja esse. O conjunto compilado é dado pelo [references/linux/drivers/gpu/drm/gma500/Makefile:1-47](../references/linux/drivers/gpu/drm/gma500/Makefile#L1).

## CONFIRMED — o que existe

| Área | Implementação observada | Fonte |
| --- | --- | --- |
| Identificação | Intel 8086:8108 e 8109 → psb_chip_ops; comentário SGX535 | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:43-59](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L43) |
| PCI/MMIO | mapeamento de janela SGX; chip ops seleciona offset Poulsbo | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:261-264](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L261); [references/linux/drivers/gpu/drm/gma500/psb_device.c:272](../references/linux/drivers/gpu/drm/gma500/psb_device.c#L272) |
| KMS | init modeset, polling, vblank; Poulsbo inicializa LVDS/SDVO | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:366-405](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L366); [references/linux/drivers/gpu/drm/gma500/psb_device.c:18-23](../references/linux/drivers/gpu/drm/gma500/psb_device.c#L18) |
| GEM | dumb buffers, mmap/fault, pin/unpin, páginas e stolen | [references/linux/drivers/gpu/drm/gma500/gem.c:29-107](../references/linux/drivers/gpu/drm/gma500/gem.c#L29); [references/linux/drivers/gpu/drm/gma500/gem.c:140-200](../references/linux/drivers/gpu/drm/gma500/gem.c#L140); [references/linux/drivers/gpu/drm/gma500/gem.c:255-300](../references/linux/drivers/gpu/drm/gma500/gem.c#L255) |
| GTT | recursos, tabela e inserção/remoção/restauração | [references/linux/drivers/gpu/drm/gma500/gtt.c:78-157](../references/linux/drivers/gpu/drm/gma500/gtt.c#L78); [references/linux/drivers/gpu/drm/gma500/gtt.c:185-287](../references/linux/drivers/gpu/drm/gma500/gtt.c#L185) |
| MMU SGX | PD/PT, flags, inserção/remoção de páginas, invalidação | [references/linux/drivers/gpu/drm/gma500/mmu.c:44-210](../references/linux/drivers/gpu/drm/gma500/mmu.c#L44); [references/linux/drivers/gpu/drm/gma500/gem.c:56-59](../references/linux/drivers/gpu/drm/gma500/gem.c#L56); [references/linux/drivers/gpu/drm/gma500/gem.c:96-98](../references/linux/drivers/gpu/drm/gma500/gem.c#L96) |
| Reset básico | soft reset dos blocos, clear BIF fault e base 2D | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:103-125](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L103) |
| Init SGX parcial | zera bancos BIF, remove bypass, cria PDs, mapeia stolen, programa bases PDS/3D | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:127-163](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L127); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:340-362](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L340) |
| IRQ | vblank/page flip; IRQ SGX 2D e fault BIF; acknowledge | [references/linux/drivers/gpu/drm/gma500/psb_irq.c:115-129](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L115); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:151-230](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L151); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:286-289](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L286) |
| PM | save/restore display e PCI D3hot/D0, reconstrução GTT/GEM | [references/linux/drivers/gpu/drm/gma500/power.c:95-203](../references/linux/drivers/gpu/drm/gma500/power.c#L95) |

Não resumir isso como “Linux não toca no SGX”: ele toca. Também não chamar de driver 3D funcional por conter registradores e MMU.

## CONFIRMED — limites da interface e do fluxo

`psb_ioctls[]` está vazio; features registradas são `DRIVER_MODESET | DRIVER_GEM`, sem DRIVER_RENDER. Há `dumb_create`, mas não interface própria de submissão SGX. [references/linux/drivers/gpu/drm/gma500/psb_drv.c:91-95](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L91); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:493-510](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L493). O Kconfig descreve KMS framebuffer 2D experimental, não aceleração 3D. [references/linux/drivers/gpu/drm/gma500/Kconfig:16-19](../references/linux/drivers/gpu/drm/gma500/Kconfig#L16).

`gma_sgx_interrupt` lê status de blit e imprime faults; não há conclusão de jobs 3D nessa função. Habilitação normal só liga TWOD_COMPLETE e BIF_REQUESTER_FAULT. [references/linux/drivers/gpu/drm/gma500/psb_irq.c:151-196](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L151); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:286-289](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L286).

O inventário de usos de EVENT_KICK, USE_CODE_BASE e slave port 2D no diretório encontra definições, não um caminho de emissão; busca por `drm_sched`, `dma_fence` e `request_firmware` não encontra implementação local. Isso é evidência delimitada ao diretório e combinada com a interface/fluxo acima, não uma prova universal por palavras-chave. [evidence.txt](evidence.txt).

Presença de `dma_resv_lock` no pin GEM não equivale a scheduler e fences de renderização. [references/linux/drivers/gpu/drm/gma500/gem.c:39-65](../references/linux/drivers/gpu/drm/gma500/gem.c#L39). Defines de pacotes 2D e cookies de cena também não demonstram um caminho ativo de aceleração. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:165-190](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L165); [references/linux/drivers/gpu/drm/gma500/psb_reg.h:522-538](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L522).

## Power e recovery: limitações concretas

**CONFIRMED:** `psb_power_up/down` retornam zero sem operações. O framework comum gerencia PCI/display, e um comentário declara runtime PM quebrado, mantendo referência com `pm_runtime_get`. Não herdar capacidade de outros chip ops para Poulsbo. [references/linux/drivers/gpu/drm/gma500/psb_device.c:186-194](../references/linux/drivers/gpu/drm/gma500/psb_device.c#L186); [references/linux/drivers/gpu/drm/gma500/power.c:46-70](../references/linux/drivers/gpu/drm/gma500/power.c#L46).

**CONFIRMED:** o reset chamado na init não contém o handshake microkernel, o processamento de CCB ou o recovery de jobs do DDK TI. Compare [references/linux/drivers/gpu/drm/gma500/psb_drv.c:103-163](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L103) com [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:467-665](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L467) e [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:544-655](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L544). **UNKNOWN:** quanto do estado necessário a 3D já é deixado por firmware de plataforma.

## INFERRED — o que falta para aceleração SGX535 3D

A comparação das interfaces acima com o DDK permite identificar lacunas, sem escolher implementação agora:

- Boot SGX535 reproduzível: identificação/revisão, clocks, reset e scripts completos; firmware com origem e licença adequadas.
- Contextos e buffers executáveis/controláveis com validação, lifetime, coerência e isolamento; GEM scanout sozinho não fornece isso.
- Submissão de jobs e ABI kernel/userspace, scheduler, conclusão, dependências, timeout e recuperação.
- Formatos de comandos e estados TA/3D/PDS, parâmetros/tile buffers, render targets, texturas e PBE.
- ISA/assembler/compiler USSE e emissão de shaders/estados em userspace; só então integração com uma API gráfica/Mesa.

Fontes da comparação: [references/linux/drivers/gpu/drm/gma500/psb_drv.c:91-95](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L91); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:505-510](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L505); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:70-201](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L70); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h:85-114](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h#L85); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:2612-2629](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L2612). A necessidade de cada componente exato pode mudar se um desenho novo dispensar partes do protocolo histórico; a possibilidade dessa alternativa é **UNKNOWN**.
