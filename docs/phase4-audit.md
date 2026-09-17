# Auditoria adversarial da Fase 4

## Escopo e método

A auditoria releu todos os documentos em `docs/`, as 123 linhas da matriz de
evidências da Fase 3 e as fontes originais correspondentes. A unidade contada
como uma afirmação auditada é um identificador `P3-*` único, não cada linha da
matriz: uma afirmação pode ter várias fontes. Foram auditadas **60 afirmações
CONFIRMED** (`P3-001` a `P3-064`, com quatro números não usados). Cada uma foi
verificada contra repositório, commit, arquivo, linhas e plataforma.

O resultado não transforma uso histórico em contrato de hardware. Código que
lê um registrador comprova que aquela versão do driver efetuava a leitura; não
comprova ausência de efeitos colaterais, pré-condição de clock ou segurança em
outra revisão.

## Resultado quantitativo

| item | quantidade |
|---|---:|
| afirmações CONFIRMED auditadas | 60 |
| rebaixadas para INFERRED | 0 |
| rebaixadas para UNKNOWN | 0 |
| divergências/contradições abertas com impacto no bring-up | 5 |

O total zero de rebaixamentos significa que os enunciados `P3-*`, já limitados
à plataforma e à fonte, continuam sustentados. Três conclusões operacionais
foram **estreitadas**, sem mudar a confiança da evidência subjacente:

1. `CORE_ID`/`CORE_REVISION` continuam offsets confirmados e leituras
   historicamente observadas, mas sua segurança de leitura é **UNKNOWN**.
2. `power/runtime_status=active` descreve o estado runtime-PM do dispositivo
   PCI, não prova que clocks internos da SGX estejam habilitados.
3. “hardware state modified: NO” no Test Vector Zero descreve somente as ações
   do probe. O bind anterior do `gma500` modifica SGX, GTT, MMU e IRQ.

Documentação anterior também foi corrigida onde ainda dizia, no tempo presente,
que `sgx535defs.h` faltava: `registers.md` e `mmu-bif.md` agora distinguem o
checkout master sem o arquivo dos commits históricos que o contêm.

## Claims downgraded during Phase 4

Nenhuma afirmação `P3-*` foi rebaixada. As três restrições operacionais acima
corrigem interpretações possíveis, não os fatos source-scoped auditados.

## Evidências novas decisivas

- **CONFIRMED — P4-001:** o Linux atual associa exatamente `8086:8108` e
  `8086:8109` a Poulsbo/SGX535 e `psb_chip_ops`; não se usa o nome de `lspci`
  como prova (`linux`, commit `9b87fdc...`, `psb_drv.c:43-59`).
- **CONFIRMED — P4-006/P4-007:** durante o bind, antes de registrar o DRM, o
  `gma500` mapeia VDC/SGX, inicializa PM, GTT, GEM e MMU, reseta blocos SGX,
  programa contextos/BIF/PDS e instala IRQ (`psb_drv.c:250-385,450-479`).
- **CONFIRMED — P4-012:** o driver chama `pm_runtime_get()`
  incondicionalmente porque seu runtime PM é declarado quebrado; os callbacks
  Poulsbo `power_up`/`power_down` são stubs (`power.c:46-70`;
  `psb_device.c:186-194`).
- **CONFIRMED — P4-014:** PSB histórico lê `CORE_ID` e `CORE_REVISION`, mas não
  documenta semântica elétrica, clock ou efeito colateral
  (`PSB_psb_drv_c.txt:325-340`).

## Cinco divergências preservadas

1. **Apertura SGX:** Linux atual mapeia `0x8000`; DDK Poulsbo histórico declara
   `0x4000`. A origem é explícita em ambos, mas a revisão/configuração que
   explica a diferença permanece **UNKNOWN** (`P3-003`, `P3-005`).
2. **Contextos BIF:** Linux/PSB usam a expressão baseada em `BASE1 + context*4`;
   TI SGX535/EMGD usam arranjo distinto. Não há base para escolher uma fórmula
   genérica (`P3-026`, `P3-061`).
3. **Endereço GATT/MMU:** o Linux insere stolen em `gatt_start`, calcula espaço
   a partir de `mmu_gatt_start`, e no unload remove a sequência a partir de
   `mmu_gatt_start`. O comentário `mmu_gatt ??` confirma incerteza no próprio
   código (`psb_drv.c:134-160,189-193,352-359`).
4. **Power ownership:** o gma500 atual mantém uma referência runtime-PM e tem
   callbacks Poulsbo vazios, enquanto o DDK Poulsbo histórico chama uma camada
   OSPM externa cuja implementação não está no artefato. Não há sequência
   completa e reconciliada de power/clock da SGX (`P3-017`, `P3-045`).
5. **Cobertura dos headers SGX535:** os headers TI histórico e EMGD têm 597
   valores escalares comuns iguais, mas conjuntos exclusivos diferentes. Isso
   comprova parentesco dos artefatos, não equivalência da integração nem
   segurança dos registradores (`P3-063`).

## Extrapolações recusadas

- SGX540/544 não preenche campos SGX535 ausentes.
- OMAP/TI só descreve a integração TI.
- EMGD permanece artefato histórico de espelho comunitário.
- ABI PSB histórica não é proposta de ABI moderna.
- O display KMS do gma500 não prova aceleração 3D.
- Nomes de registrador e `#define` não provam read-safety.

## Proveniência e licenças

| material | owner/origem | licença/proveniência | uso permitido nesta fase |
|---|---|---|---|
| Linux/gma500 e documentação kernel | Linux contributors; commit `9b87fdc...` | SPDX por arquivo, predominantemente GPL-2.0 | evidência primária; código só conforme licença |
| TI KM/UM e DDK Poulsbo histórico | TI/IMG e respectivos autores; commits fixados na matriz | licenças por arquivo/árvore; não presumir uniformidade | documentação/evidência; reuso depende de auditoria do arquivo |
| PSB KMP histórico | snapshot público `gregkh/psb-kmp` commit `98b5307...` | código histórico GPL conforme cabeçalhos/árvore | evidência e possível referência GPL; não copiar para Mesa permissivo sem análise |
| EMGD mirror | Intel-origin material em espelho comunitário `e6884ec...` | licença do pacote e proveniência incompleta para autenticação | estudo histórico somente; não copiar para implementação nova |
| binários EMGD | espelho comunitário | binários históricos, termos do pacote | catalogar/hashes; nunca código reutilizável |
| `sgx535-probe` | código original SGX535-GFX, Fase 4 | MIT, arquivo `tools/sgx535-probe/LICENSE` | reutilizável sob MIT |

Nenhum trecho histórico foi copiado no probe. Constantes PCI usadas como fatos
de identificação vêm da tabela Linux e são registradas na matriz.

## Conclusão da auditoria

**CONFIRMED (P4-001–P4-006):** um inventário passivo por sysfs/procfs pode
identificar a função PCI e o binding sem tocar em BAR ou abrir DRM. **UNKNOWN:**
não há, nas fontes
auditadas, um registrador SGX535/Poulsbo com contrato simultaneamente explícito
de leitura sem efeito colateral, power/clock válido, stepping coberto e regra de
locking. Portanto a auditoria aprova o Test Vector Zero e bloqueia MMIO.
