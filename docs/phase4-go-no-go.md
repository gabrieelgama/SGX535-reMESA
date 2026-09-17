# Matriz GO / NO-GO / BLOCKED da Fase 4

Os estados valem para as fontes e o harness atuais. `GO` não autoriza uma etapa
posterior implícita.

| item | estado | fundamento; falta exata para desbloquear |
|---|---|---|
| A. Passive hardware probe | GO | IDs exatos e atributos textuais RO documentados; implementação não abre dispositivo nem BAR |
| B. Controlled MMIO reads | BLOCKED | falta contrato de read-side-effects, power/clock por registrador, stepping e locking; fonte necessária: datasheet/register reference SGX535 Poulsbo + errata/power sequence Intel/IMG |
| C. Controlled SGX reset | BLOCKED | falta sequência oficial por revisão, estado pré/pós, impacto no display/IRQ/MMU e recuperação; fonte necessária: init/reset script SGX535 Poulsbo autenticado ou documentação Intel/IMG |
| D. Clock/power manipulation | NO-GO | gma500 atual tem runtime PM declarado quebrado, callbacks Poulsbo vazios e função PCI/display compartilhada; nenhuma manipulação externa é aceitável |
| E. BIF observation | BLOCKED | divergência de layout/contextos e semântica de status/fault; fonte necessária: SGX535 Poulsbo BIF register reference + errata de revisão |
| F. MMU initialization | BLOCKED | fórmula de contextos e endereços GATT/MMU diverge; falta formato/flush/fault oficial e limites; fonte necessária: DDK Poulsbo completo e documentação BIF/MMU da revisão |
| G. SGX address-space creation | BLOCKED | depende de F e do modelo GTT/stolen confirmado em hardware; falta ABI de memória moderna e regras de cache/coerência |
| H. Firmware/microkernel loading | NO-GO | firmware verificado, licença, hash, revisão alvo e protocolo de bootstrap estão ausentes |
| I. CCB allocation | BLOCKED | formato de CCB, memória/coerência, produtor/consumidor e firmware compatível não estão estabelecidos para Poulsbo alvo |
| J. EVENT_KICK | NO-GO | é ação de submissão; endereço/evento/firmware e recuperação não estão verificados |
| K. PDS execution | NO-GO | binário, ISA, bases, stepping e sandbox inexistentes |
| L. USSE execution | NO-GO | ISA/encoding e limites SGX535 não foram reconstruídos com fonte compatível |
| M. First GPU workload | BLOCKED | depende de B–L e de mecanismo de hang detection/recovery; fonte necessária: stack Poulsbo SGX535 completo e reproduzível |
| N. First rendering workload | BLOCKED | depende de M, formatos de command stream/render target e validação; fonte necessária: documentação/compilador/driver userspace com proveniência adequada |
| O. Mesa integration | BLOCKED | arquitetura moderna só pode ser definida após memória, submission, sync, firmware e workload mínimos confirmados |

## Decisão

Somente A está `GO`. D, H, J, K e L são `NO-GO` com as evidências atuais. Os
demais itens estão `BLOCKED`, nunca promovidos por semelhança com SGX540/544,
OMAP ou EMGD.

