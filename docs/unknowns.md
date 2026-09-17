# 20 incógnitas prioritárias — avaliação da Fase 4

Fase 4 — auditoria estática, 2026-09-17. Nenhum MMIO, PCI config, reset,
firmware ou workload foi executado. IDs P3/P4 remetem à
[matriz](evidence-matrix.csv).

| # | Incógnita | Evidência atual / falta exata | Bloqueia |
|---|---|---|---|
| U01 | Qual é revision/subsystem/stepping da máquina real? | IDs alvo confirmados; ainda não há relatório do Inspiron | MMIO e posteriores |
| U02 | `CORE_ID`/`CORE_REVISION` são seguros para leitura no stepping real? | uso histórico confirmado, contrato de read-side-effect ausente | primeiro MMIO |
| U03 | Qual estado PM garante clocks SGX válidos? | runtime PM PCI não equivale a clock interno (P4-004/P4-012) | qualquer MMIO |
| U04 | Quais registradores são legíveis quando SGX está gated/off? | nenhuma fonte aplicável define isso | qualquer MMIO |
| U05 | Qual lock serializa leitura SGX genérica? | locks de IRQ/MMU/GTT existem; lock SGX global não | MMIO concorrente |
| U06 | Como evitar corrida com IRQ e KMS? | IRQ agrega display/SGX e usa `irqmask_lock` (P4-010) | status/IRQ reads |
| U07 | Por que a aperture é `0x8000` no Linux e `0x4000` no DDK? | ambas confirmadas, revisão/configuração desconhecida | range MMIO |
| U08 | Qual fórmula de directory-list vale para cada contexto? | Linux/PSB e TI/EMGD divergem (P3-026/P3-061) | MMU/contextos |
| U09 | Por que init/remove misturam `gatt_start` e `mmu_gatt_start`? | divergência interna confirmada | MMU/memória |
| U10 | Sequência completa de clock/power/reset por revisão? | gma500 tem PM incompleto; OSPM histórica ausente | reset/init |
| U11 | Quais errata/BRNs valem no silício real? | builds rev121/126 não medem a placa | reset/BIF/workload |
| U12 | Relação física e coerência entre GTT, GATT, stolen e BIF? | mecanismos parciais confirmados, contrato fim a fim ausente | address space |
| U13 | Semântica de fault/status: latch, clear, ordering e ack? | handler mostra prática, não contrato | observação BIF/IRQ |
| U14 | Há watchdog de plataforma e recovery SGX confiável? | nenhum mecanismo gma500 localizado; estado da plataforma desconhecido | operações ativas |
| U15 | Scripts init/deinit completos do DDK Poulsbo? | kernel consome interfaces; payload/UM correspondente ausente | bootstrap |
| U16 | Firmware/microkernel SGX535 verificável e licenciável? | payload compatível não disponível | firmware/CCB |
| U17 | Programa PDS e protocolo de bootstrap exatos? | bases/kick não definem programa | PDS/firmware |
| U18 | ABI CCB, sync, relocations e cache completa? | interfaces históricas parciais e distintas | submission |
| U19 | ISA/encoder USSE e PDS SGX535 com proveniência? | ausente; SGX540/544 não substitui | execução própria |
| U20 | Streams TA/3D, DPM, tiling, formatos/PBE e isolamento moderno? | insuficientes nas fontes atuais | workload/render/Mesa |

## Bloqueadores imediatos do primeiro MMIO

Os cinco maiores são U01, U02, U03/U04, U05/U06 e U07/U11. Em termos de
artefatos: medição passiva da placa; register reference/errata SGX535 Poulsbo;
power/clock sequence autenticada; código host completo com locking/OSPM; e
documentação que ligue a revisão PCI/SGX à aperture e aos BRNs.

## Mudanças desde a Fase 3

- O ownership do gma500 está agora localizado; isso demonstra que módulo
  separado não é seguro, mas não fornece lock universal.
- `CORE_ID`/`CORE_REVISION` deixaram de ser “candidato condicionado” e ficam
  formalmente **UNKNOWN** para read-safety.
- Test Vector Zero foi implementado e não depende de MMIO.
- GTT e stolen não são inventados quando a interface passiva selecionada não os
  expõe.
- Firmware/ISA continuam bloqueadores posteriores, não do inventário passivo.

## Artefato externo de maior valor

O item mais valioso para o **próximo bloqueador imediato** é um manual de
registradores + power/reset/errata autenticado para **SGX535 integrado ao
Poulsbo**, vinculando PCI revision/SGX core revision, aperture, comportamento de
leitura de `CORE_ID`/`CORE_REVISION`, clocks e requisitos de ownership. Para o
bootstrap posterior, o pacote UM/microkernel/inicializador correspondente ao
DDK Poulsbo 1.14 continua sendo o artefato de maior valor.
