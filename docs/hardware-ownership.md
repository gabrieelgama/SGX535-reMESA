# Ownership do hardware

## Dono atual dos recursos

| recurso | owner no Linux atual | sincronização observada | resultado Phase 4 |
|---|---|---|---|
| função PCI | `psb_pci_driver`/DRM gma500 | lifetime PCI + devm DRM | owner exclusivo |
| VDC e SGX MMIO | `drm_psb_private.vdc_reg` e `.sgx_reg` | não há lock SGX global | não acessar externamente |
| GTT MMIO | `gtt_map` | `gtt_mutex` | não acessar externamente |
| page tables SGX | `psb_mmu_driver` | `sem` + page-table spinlock | não acessar externamente |
| GEM/GTT mappings | objetos GEM | `dma_resv` + `gtt_mutex`; `mmap_mutex` no fault | não acessar externamente |
| IRQ PCI | `gma_irq_handler`, `IRQF_SHARED` | `irqmask_lock` para máscara/handler | compartilhado com display/SGX |
| runtime PM | gma500 + PM core | contagem runtime-PM | usar API do próprio driver |

**CONFIRMED — P4-007:** o driver mapeia VDC e SGX a partir do recurso MMIO e
mantém os ponteiros em `drm_psb_private`; desmonta e desmapeia no unload
(`psb_drv.c:254-264,166-210`; `psb_drv.h:377-423`).

**CONFIRMED — P4-008:** inserção/remoção de GTT usa `gtt_mutex`; pin/unpin GEM
usa `dma_resv_lock`; faults de mmap usam `mmap_mutex`
(`gtt.c:70-125`; `gem.c:29-108,270-300`).

**CONFIRMED — P4-009:** a MMU mantém um `rw_semaphore` para estruturas
driver/PD e um spinlock para tabelas; flush e mudança de PD tomam o semaphore em
write (`mmu.h:11-24`; `mmu.c:97-135`).

**CONFIRMED — P4-010:** a IRQ agrega identidade VDC, display, hotplug e SGX. O
handler toma `irqmask_lock`, lê status SGX quando indicado e reconhece eventos
escrevendo `EVENT_HOST_CLEAR{,2}`; a instalação usa `IRQF_SHARED`
(`psb_irq.c:151-247,250-333`).

## Respostas diretas

1. **O gma500 pode acessar SGX simultaneamente a um experimento nosso?** Sim,
   seu IRQ, PM e caminhos MMU continuam ativos. Não há exclusão para código
   externo. Um acesso paralelo independente é **UNSAFE**.
2. **Quais acessos exigem locks?** GTT: `gtt_mutex`; MMU/PD: `mmu->sem` e, nas
   rotinas internas, page-table spinlock; IRQ/máscara: `irqmask_lock`; objetos:
   `dma_resv`; mmap fault: `mmap_mutex`. Não foi localizado um lock geral que
   autorize qualquer offset SGX.
3. **Quais exigem GPU powered?** A resposta por registrador é **UNKNOWN**. A API
   `gma_power_begin()` garante somente o que o código chama de display power
   island; não existe contrato auditado para todos os clocks SGX.
4. **É possível observar SGX sem interferir com KMS?** Pela identidade PCI e
   estado já exportado: sim. Por MMIO SGX: **UNKNOWN/BLOCKED**.
5. **Módulo separado seria seguro?** **NO-GO**. Ele não possuiria o
   `drm_psb_private`, os locks, a contagem PM, o IRQ ou o lifetime dos mappings.
6. **Instrumentar gma500 é mais seguro?** **INFERRED: sim**, para um futuro
   acesso limitado, porque esse local possui ownership e PM. Ainda exige prova
   de read-safety e uma whitelist por revisão.
7. **Existe risco para display?** **CONFIRMED:** SGX e display compartilham a
   função PCI, BAR0/VDC e o handler agregado. Suspend desinstala a IRQ e coloca
   toda a função em D3hot; portanto interferência é plausível e deve ser tratada
   como risco real (`power.c:178-203`; `psb_irq.c:198-247`).
8. **Recursos compartilhados?** Função PCI, recurso MMIO base, IRQ, runtime PM,
   GTT/GATT/stolen e lifetime DRM.

## Arquitetura futura preferida

Para qualquer Phase 5 que um dia seja desbloqueada, a ordem de segurança é:

1. instrumentação temporária compilada dentro do gma500;
2. ponto debug read-only específico no próprio driver;
3. userspace por ioctl específico e sem offsets somente se houver necessidade;
4. módulo separado — rejeitado nas condições atuais.

Nenhuma opção autoriza uma primitive genérica `read_mmio(offset)` ou qualquer
write. A preferência é inferência de ownership, não aprovação de MMIO.

