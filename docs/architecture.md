# SGX535-GFX — arquitetura reconstruída

> Atualização da fase 2: o histórico local contém `sgx535defs.h` e uma integração Poulsbo explícita no DDK 1.14. As referências à ausência abaixo descrevem o checkout master da fase 1. Consulte [arqueologia](source-archaeology.md), [arquivos recuperados](sgx535-missing-files.md) e [comparação Poulsbo](poulsbo-evidence.md) para o estado ampliado.

## Escopo e método

Investigação estática em 2026-09-16. Nenhum driver novo, acesso MMIO, carregamento de módulo, firmware ou comando GPU foi implementado ou executado. “Linux atual” neste conjunto significa **o checkout local identificado abaixo**, não uma declaração sobre o HEAD remoto ou o kernel da máquina alvo.

- **CONFIRMED**: declaração ou comportamento encontrado no código/artefato indicado; não significa validação no silício.
- **INFERRED**: conclusão derivada de evidências citadas, com premissas explícitas.
- **UNKNOWN**: informação não estabelecida por estas fontes. Ausência de código aqui não prova ausência de capacidade do hardware.

As fontes solicitadas como `ti-sgx-km` e `ti-sgx-um` estão presentes com os nomes `omap5-sgx-ddk-linux` e `omap5-sgx-ddk-um-linux`. Não foram renomeadas. Commits e inventário de evidências estão em [evidence.txt](evidence.txt). As três árvores estavam sem modificações no início da análise.

| Árvore | Commit examinado |
| --- | --- |
| Linux | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` |
| TI KM | `430673f78b79eccdf308a6bbfb524209b485d2cc` |
| TI UM | `b6801bf89e00d69893c2957455bbeb3195c0ed52` |

## Resultado principal

**CONFIRMED:** existe uma seleção explícita SGX535, com espaço virtual declarado de 32 bits, múltiplos contextos, 16 directory lists BIF, hardware 2D, duas USE pipes e autoclockgating. É uma configuração do DDK, não uma medição de Poulsbo. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h:63-73](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h#L63).

**CONFIRMED:** o dispatcher inclui `sgx535defs.h`, mas esse arquivo não consta no inventário do checkout. Os headers específicos presentes são SGX530, SGX540 e SGX544; os Makefiles OMAP4430/5430 selecionam respectivamente 540 rev.120 e 544 rev.116. Portanto, não temos um DDK SGX535 completo nem evidência de uma integração OMAP-SGX535 neste pacote. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxdefs.h:48-80](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxdefs.h#L48); [references/omap5-sgx-ddk-linux/eurasia_km/eurasiacon/build/linux2/omap4430_linux/Makefile:109-110](../references/omap5-sgx-ddk-linux/eurasia_km/eurasiacon/build/linux2/omap4430_linux/Makefile#L109); [references/omap5-sgx-ddk-linux/eurasia_km/eurasiacon/build/linux2/omap5430_linux/Makefile:112-113](../references/omap5-sgx-ddk-linux/eurasia_km/eurasiacon/build/linux2/omap5430_linux/Makefile#L112); [inventário](evidence.txt).

**CONFIRMED:** o Linux associa Poulsbo/GMA500 a SGX535 e IDs PCI Intel `8086:8108`/`8086:8109`; sua interface própria de ioctls é vazia e as features registradas são MODESET e GEM. [references/linux/drivers/gpu/drm/gma500/psb_drv.c:43-59](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L43); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:91-95](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L91); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:505-510](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L505).

## Mapa com fronteiras de evidência

```mermaid
flowchart TD
    UM[UM histórico TI: bibliotecas e inicializador binários]
    BR[Bridge Services KM: handles e kick]
    TA[Client CCB: parte compartilhada TA e sincronização]
    KC[Kernel CCB: comando, handler USE, cache control]
    MK[Microkernel: contrato visível, implementação fonte ausente]
    USE[USE / PDS: código e eventos]
    ENG[TA / DPM / ISP / TSP / PBE / 2D]
    BIF[BIF / MMU: diretórios e tabelas]
    MEM[Memória do sistema]
    PSB[Linux Poulsbo: PCI, reset básico, MMU e IRQ]
    GTT[GTT / GEM / KMS / display]
    UM --> BR
    BR --> TA
    BR --> KC
    KC --> MK
    MK -. inferência de execução .-> USE
    MK -. programação de engines não reconstruída .-> ENG
    USE --> BIF
    ENG --> BIF
    BIF --> MEM
    PSB --> BIF
    PSB --> GTT
    GTT --> MEM
```

As setas do protocolo host são **CONFIRMED no DDK**: bridge chama `SGXDoKickKM`, este preenche a parte compartilhada e agenda o CCB; o comando contém endereço de handler USE. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/bridged/sgx/bridged_sgx_bridge.c:188-605](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/bridged/sgx/bridged_sgx_bridge.c#L188); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxkick.c:72-96](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxkick.c#L72); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxkick.c:732-803](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxkick.c#L732); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:70-96](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L70).

A execução do microkernel em USE é **INFERRED** da descrição do handler e dos nomes USSE_EDM; seu protocolo host é confirmado, mas o fluxo interno e o ISA não estão reconstruídos. Os requestors TA/VDM/2D/PBE/TSP/ISP/USSEPDS/host são **CONFIRMED como nomes e bits do diagnóstico Linux**, sem estabelecer ordem física do pipeline. [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:72-74](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L72); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:289-306](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L289); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:159-188](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L159).

A separação GTT/MMU é **CONFIRMED** por duas inserções distintas no pin GEM. Não se deve desenhar GTT e BIF como uma única tabela nem presumir que toda transação atravesse ambas em série. [references/linux/drivers/gpu/drm/gma500/gem.c:29-59](../references/linux/drivers/gpu/drm/gma500/gem.c#L29).

## Sequência reconstruída do DDK

**CONFIRMED, código compartilhado condicionado por features:** clocks → script de init parte 1 → reset → script parte 2 → limpar status → kick → esperar `INIT_COMPLETE`. Scripts e endereços dos handlers chegam pela estrutura de inicialização; as listas concretas SGX535 não estão nesta fonte. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:207-293](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L207); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:467-665](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L467).

**CONFIRMED:** recuperação chama reinicialização com hardware recovery; o ramo não-MP de reset usa um PD temporário e atende faults pendentes antes de restaurar contexto BIF. Isso é mais que simplesmente alternar SOFT_RESET. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:1582-1645](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L1582); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:544-655](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L544). **UNKNOWN:** quais dessas operações são necessárias e corretas para cada stepping Poulsbo.

## Licenças e limites

Este resultado contém documentação, números e referências, sem implementação copiada. O README KM declara MIT/GPLv2 e os headers consultados trazem aviso dual; o código Linux consultado usa SPDX GPL-2.0-only. Isso deve ser verificado por arquivo antes de qualquer futura reutilização. [references/omap5-sgx-ddk-linux/eurasia_km/README:17-25](../references/omap5-sgx-ddk-linux/eurasia_km/README#L17); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxmmu.h:1-40](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxmmu.h#L1); [references/linux/drivers/gpu/drm/gma500/mmu.c:1](../references/linux/drivers/gpu/drm/gma500/mmu.c#L1).

O manifesto UM identifica `targetfs` como binário sob TI TSPA e traz restrições de modificação e engenharia reversa. A extração textual parcial e sua localização estão no [registro de evidências](evidence.txt); ela não substitui revisão integral do documento original. Nenhum payload binário foi extraído para implementação, descompilado, desassemblado ou executado. A observação de símbolos ELF foi limitada a inventário de artefatos. Não aplicar a licença do KM ao UM.

## Leitura do conjunto

- [Registradores](registers.md): offsets Linux e limites de interpretação.
- [MMU/BIF](mmu-bif.md): tradução, caches e divergências.
- [Submissão](command-submission.md): CCB, sync e IRQ.
- [USSE](usse.md) e [firmware](firmware.md): contrato conhecido e partes ausentes.
- [TI versus Poulsbo](ti-vs-poulsbo.md): classes A/B/C.
- [Estado do gma500](gma500-current-state.md): capacidades e lacunas.
- [20 incógnitas e experimento mínimo](unknowns.md): bloqueios e próximos passos.
