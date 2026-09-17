# Segurança de power state

## O que o código atual realmente garante

**CONFIRMED — P4-012:** `gma_power_init()` declara runtime PM “broken”, toma
uma referência incondicional com `pm_runtime_get()` e só a devolve no uninit
(`power.c:46-87`). Para Poulsbo, `psb_power_up()` e `psb_power_down()` retornam
zero sem sequência adicional (`psb_device.c:186-194`).

**CONFIRMED — P4-013:** o suspend de sistema desinstala IRQ, salva display,
desabilita a função PCI e seleciona D3hot. O resume volta a D0, restaura estado
PCI/BSM/VBT, reabilita GTT/GEM/display e reinstala IRQ
(`power.c:95-203`). Estados SGX 3D preservados não são documentados.

**CONFIRMED — P4-016:** no bind Poulsbo, `psb_init_pm()` faz read-modify-write
de `PSB_CR_CLKGATECTL` para a porção 2D e posta a escrita com uma leitura
(`psb_device.c:85-94`). Isso prova comportamento do driver, não que qualquer
outra leitura SGX seja válida.

## Respostas

| pergunta | resposta |
|---|---|
| quando SGX está ligada? | **UNKNOWN** no nível de clocks/blocos internos |
| quando está power-gated? | **UNKNOWN** |
| quem controla? | função PCI/PM: gma500 + PM core; gate SGX completo: **UNKNOWN** |
| quais registradores dependem de clocks? | somente comportamento de 2D clock gate aparece; conjunto completo **UNKNOWN** |
| leitura é válida desligada? | **UNKNOWN**; nenhuma fonte auditada fornece esse contrato |
| o que pode acordar? | `gma_power_begin(force_on=true)` chama `pm_runtime_resume_and_get()`; o probe não o chama (P4-021) |
| o que pode desligar? | suspend PCI chama disable + D3hot; o probe não o chama |
| estado preservado? | display é salvo/restaurado; estado SGX/BIF/USSE/PDS **UNKNOWN** |
| estado perdido em reset? | reset toca BIF/DPM/TA/USE/ISP/TSP/2D; consequências completas **UNKNOWN** |

`power/runtime_status=active` é **CONFIRMED** como estado do runtime-PM core,
mas sua equivalência a “SGX pronta para MMIO” é **UNKNOWN**. Não se infere clock
de um status PCI.

## READ não implica SAFE

**CONFIRMED — P4-020:** o DDK histórico diz que o dump de registradores **não
deve** ser feito quando a SGX não está powered (`INIT.txt:1320-1334`). Isso
refuta qualquer premissa de que leitura seja segura com power off. O trecho não
define como provar o estado powered, quais clocks bastam ou quais registradores
têm efeitos colaterais. Nenhuma leitura MMIO é aprovada nesta fase.

## Operações do probe

Ler os atributos textuais escolhidos não chama `gma_power_begin()` e a função
`runtime_status_show()` apenas consulta o estado do PM core (CONFIRMED
`P4-004`). O probe não abre DRM. Portanto ele não contém um caminho intencional
de wake; não se promete ausência de efeitos por agentes externos concorrentes.
