# Identificação passiva do hardware

## Identidade aceita

O probe aceita somente estes pares:

| vendor | device | classificação | evidência |
|---|---|---|---|
| `0x8086` | `0x8108` | Poulsbo, SGX535 segundo a tabela do Linux | CONFIRMED `P4-001` |
| `0x8086` | `0x8109` | Poulsbo, SGX535 segundo a tabela do Linux | CONFIRMED `P4-001` |

A tabela fonte declara também GMA 500/Atom Z5xx. Ela não mapeia revision ou
subsystem ID para US15W, US15WP, US15WPT ou stepping SGX específico. Essa
subclassificação é **UNKNOWN**. Revision e subsystem são registrados, jamais
usados para promovê-la.

## Superfícies passivas escolhidas

**CONFIRMED — P4-002:** a documentação PCI do Linux marca `vendor`, `device`,
`revision`, `subsystem_vendor`, `subsystem_device`, `class`, `irq` e `resource`
como atributos ASCII read-only. `resource` contém start/end/flags, a partir dos
quais o tamanho pode ser calculado inclusivamente
(`Documentation/PCI/sysfs-pci.rst:7-75`; `drivers/pci/pci-sysfs.c:40-76,163-195`).

São coletados:

- BDF, vendor/device, revision, subsystem, class e IRQ;
- entradas do arquivo textual `resource`, incluindo os índices 0–5 como BARs;
- alvo do symlink `driver`;
- nós `cardN`, `renderDN` ou `controlDN` em `/sys/class/drm` cujo symlink
  `device` resolve para a mesma função PCI;
- release/versão do kernel por procfs e arquitetura por `uname(2)`;
- campos DMI públicos de sistema/placa/BIOS em `/sys/class/dmi/id`, quando
  disponíveis; serial, UUID e asset tags não são coletados;
- `power_state`, `power/runtime_status` e contadores de tempo, quando presentes.

**CONFIRMED — P4-019:** o Linux exporta vendor/nome/versão de sistema e placa e
vendor/versão/data do BIOS com modo `0444`; serial e UUID usam modo `0400`
(`drivers/firmware/dmi-id.c:22-62,188-224`). O probe seleciona apenas os campos
públicos sem identificadores únicos.

**CONFIRMED — P4-003:** `config`, `enable` e `resourceN` não pertencem ao vetor:
`config` é espaço de configuração binário RW, `enable` é RW e `resourceN` pode
ser mmap de programação do dispositivo. A ROM normalmente exige write para ser
habilitada (`sysfs-pci.rst:36-87`).

**CONFIRMED — P4-004:** `runtime_status` pode retornar `active`, `suspended`,
`suspending`, `resuming`, `error` ou `unsupported`; sua implementação apenas
formata o estado mantido pelo PM core. O probe não lê nem escreve
`power/control`, cujo write para `on` pode acordar o dispositivo
(`sysfs-devices-power:35-52,264-306`; `drivers/base/power/sysfs.c:123-179`).

**CONFIRMED — P4-022:** o atributo PCI read-only `power_state` formata
`pci_dev.current_state` (`drivers/pci/pci-sysfs.c:154-161`). Também não descreve
os clocks internos da SGX.

## DRM e memória

**CONFIRMED — P4-005:** o gma500 atual anuncia `DRIVER_MODESET | DRIVER_GEM`,
possui tabela própria de ioctls vazia e não anuncia `DRIVER_RENDER`
(`psb_drv.c:91-95,493-518`). O DRM core cria render node apenas com
`DRIVER_RENDER` (`drm_drv.c:771-785`). Assim, `cardN` é esperado; ausência de
`renderDN` não é falha do probe.

O arquivo PCI `resource` expõe recursos, mas não os valores derivados
`gtt_phys_start`, `mmu_gatt_start`, `gatt_start` ou o tamanho stolen calculado
pelo driver. O gma500 deriva GTT/GATT internamente e lê BSM para calcular
stolen (`gtt.c:185-253`; `gem.c:331-361`, CONFIRMED `P4-011`). Nenhum atributo
gma500 estável para esses derivados foi localizado; o probe imprime
“not exposed by selected passive interface”. Essa ausência de interface é um
resultado do inventário, não uma afirmação de que outro kernel nunca a exponha.

Nenhum arquivo debugfs específico do gma500 foi localizado no diretório do
driver atual. O probe não usa o debugfs genérico do DRM: ele não acrescenta um
campo necessário ao Test Vector Zero, não é uma ABI estável e ampliaria a
superfície de observação sem benefício demonstrado.

## Falha fechada

- nenhum alvo exato: exit `2`;
- BDF solicitado com ID diferente: exit `2` antes de conclusão SGX;
- múltiplos alvos: exit `2` até seleção explícita;
- atributo obrigatório ilegível/malformado: exit `2`;
- nenhuma tentativa de “adivinhar” por classe PCI, nome ou CPU.

## Limite da expressão “passivo”

**CONFIRMED — P4-006:** o `drm_dev_register()` ocorre depois de toda a sequência
de `psb_driver_load`; portanto observar um `cardN` gma500 também implica que o
driver já teve oportunidade de modificar hardware (`psb_drv.c:450-479`). O
relatório garante somente `state_modified_by_probe: false`.
