# bringup0 — desenho do primeiro harness

O único experimento executável aprovado é o **Test Vector Zero**, implementado
em `../../tools/sgx535-probe/`. Ele lê atributos textuais fixos de sysfs/procfs
e `uname(2)`. Não existe componente kernel nesta fase.

## Comparação das arquiteturas futuras

| opção | ownership/locking/PM | decisão |
|---|---|---|
| A. instrumentação temporária no gma500 | acesso ao `drm_psb_private`, locks, IRQ e PM | preferida se MMIO for desbloqueado |
| B. debug instrumentation do driver | mesma ownership, mas requer interface fixa e revisão cuidadosa | aceitável somente com whitelist compilada |
| C. módulo separado | concorreria com PCI driver, mappings, IRQ e PM existentes | NO-GO |
| D. userspace por interface limitada | pode ser auditável, mas exige novo ABI e mediação do driver | não necessário para primeiro read; BLOCKED |

A preferência por A/B é **INFERRED** do ownership confirmado; não é autorização
para ler MMIO.

## Invariantes do harness futuro

- validar `8086:8108` ou `8086:8109` e revisão suportada;
- tabela compilada de operações com nome e offset, sem parâmetro de offset;
- registrar PCI identity, power precondition, lock, valor e resultado;
- recusar revisão, power state ou owner desconhecido;
- nenhum write, firmware, command submission, `EVENT_KICK`, PDS ou USSE;
- cleanup/lifetime dentro do gma500;
- nenhuma interface `read_mmio(offset)`/`write_mmio(offset,value)`.

Como nenhuma leitura tem `SAFE-CANDIDATE` em
`docs/safe-register-reads.md`, o harness MMIO fica somente como design.

## Proveniência

O probe é código original da Fase 4, licenciado sob MIT em
`tools/sgx535-probe/LICENSE`. Ele usa somente a biblioteca padrão Python e não
contém código copiado do DDK, PSB ou EMGD.

