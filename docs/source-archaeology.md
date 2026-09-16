# Arqueologia de fontes — fase 2

## Resultado

**CONFIRMED:** `sgx535defs.h` existe no Git local TI KM: blob **`8039da4a73ef9ee3e929edb64244d2891bc9239e`**, 38.928 bytes, 739 linhas, encontrado em **255 dos 330 commits** disponíveis. O arquivo se identifica como SGX535 e declara licença dual MIT/GPLv2. [H535:1–43](archaeology-data/H535.txt#L1). A ausência documentada na fase 1 é verdadeira para o `master` examinado, não para o repositório inteiro.

**CONFIRMED:** existe também um alvo Poulsbo D0 com `SGXCORE := 535`, `SGX_CORE_REV := 121`, `PVR_SYSTEM := poulsbo` e `dc_poulsbo`. [PBUILD:42–52](archaeology-data/PBUILD.txt#L42). Isso permite estudar uma integração Intel explícita sem transportar a integração OMAP.

Convenção: **CONFIRMED** confirma conteúdo/histórico das fontes, não funcionamento no hardware; **INFERRED** identifica uma dedução; **UNKNOWN** é o que ainda não foi estabelecido. Identificadores de fonte como H535 e PBUILD resolvem para repositório, commit completo, arquivo e hash no [catálogo de fontes](source-archaeology.md#catálogo-de-fontes). Os snapshots documentais preservam as linhas originais e os avisos de licença; não são arquivos de implementação.

## Universo examinado e limitações

| Repositório | Commits físicos disponíveis / alcançáveis por refs | Refs locais, remotas e tags | Raso | Blobs únicos / binários omitidos da busca textual |
| --- | --- | --- | --- | --- |
| `omap5-sgx-ddk-linux` | 330 / 330 | 35 | false | 1082 / 0 |
| `omap5-sgx-ddk-um-linux` | 153 / 153 | 54 | false | 2021 / 1802 |
| `linux` | 1 / 1 | 3 | true | 95387 / 5 |

**CONFIRMED:** não havia commits adicionais não alcançáveis no banco local enumerado. Linux possui somente um commit: seu arquivo shallow interrompe a história; não há base local para concluir quando algo entrou ou saiu do gma500. “Não raso” nos dois repositórios TI não garante cobertura de branches remotas nunca clonadas, publicações privadas ou história anterior aos commits-raiz importados. Fontes: [summary.json](archaeology-data/summary.json), arquivos `*-refs.tsv` e `*-commits.tsv` neste diretório de evidências; para inventários Git, linhas de código são **N/A**.

O escopo de conteúdo é todo o projeto `/home/gama/sgx535-gfx`, particularmente os três repositórios de referência. A busca histórica inclui todos os caminhos de todas as árvores de commits disponíveis, não apenas `hwdefs` ou `gma500`. Documentos da fase 1 não contam como evidência independente. Uma busca auxiliar por nomes no home encontrou arquivos Mesa e Minecraft sem relação demonstrada; seu conteúdo e a história de projetos alheios não fazem parte deste resultado.

Nenhum fetch, checkout, switch, reset, rebase, alteração de refs, compilação, carregamento de binário ou acesso a hardware foi feito. Não foi consultada fonte externa. HEADs e estados limpos foram conferidos antes/depois. As consultas Git e os artefatos gerados ficaram restritos à leitura das referências e escrita em `docs/`.

## Método reproduzível

1. Enumerar refs e todos os objetos locais com `git cat-file --batch-all-objects --batch-check`; selecionar commits e comparar com `git rev-list --all`.
2. Enumerar cada árvore com `git ls-tree -r -z COMMIT`, incluindo renomes de diretório; relacionar commit, caminho e blob.
3. Ler cada blob único uma vez com `git cat-file --batch`; buscar os onze padrões sem distinguir maiúsculas/minúsculas. Arquivos com NUL foram classificados como binários e não submetidos à interpretação textual. Isso evita alegar que um blob fechado foi examinado como fonte.
4. Guardar linhas encontradas em `*-matches.tsv` e todas as associações históricas em `*-contexts.tsv`. O join pela coluna blob recupera **todos os commits/arquivos** de cada ocorrência. Guardar também matches em caminhos e mensagens de commits.
5. Complementar busca literal com inspeção semântica do build: `SGXCORE := 535` e `SUPPORT_SGX$(SGXCORE)` são exemplos que uma busca exclusiva por SGX535 perderia.
6. Confirmar história de arquivos por `git log --all --full-history --name-status` e snapshots de fronteira; registrar hashes e comparar offsets numericamente.

O [método da varredura](archaeology-data/scan-method.py.txt) foi preservado como texto documental. Não executa scripts de build nem binários das referências. [Registro de fontes e hashes](archaeology-data/source-registry.json).

## Resultado por padrão

Contagens de **linhas por blob único**, não quantidade de commits, ocorrências de palavra ou afirmações sobre hardware. Sobreposições são possíveis; a busca por substring é deliberadamente ampla. Em Linux, USSE/PSB podem ocorrer em palavras ou subsistemas alheios à GPU.

| Padrão | TI KM | TI UM texto | Linux texto |
| --- | --- | --- | --- |
| `sgx535defs.h` | 4 | 0 | 0 |
| `SGX535` | 35 | 0 | 3 |
| `SUPPORT_SGX535` | 0 | 0 | 0 |
| `SGX_CORE_REV` | 603 | 0 | 0 |
| `SGX_FEATURE_` | 2503 | 0 | 0 |
| `SGX_BIF` | 76 | 0 | 0 |
| `USSE` | 732 | 0 | 1801 |
| `EURASIA` | 431 | 0 | 0 |
| `Poulsbo` | 179 | 0 | 22 |
| `PSB` | 6568 | 0 | 4233 |
| `GMA500` | 4 | 0 | 28 |

**CONFIRMED:** o zero literal para `SUPPORT_SGX535` não é ausência da opção: o build emite `SUPPORT_SGX$(SGXCORE)`, e o alvo Poulsbo fixa 535. A expansão resultante é **INFERRED estaticamente**, sem executar Make. [CORE:488–494](archaeology-data/CORE.txt#L488); [PBUILD:47–50](archaeology-data/PBUILD.txt#L47).

**CONFIRMED:** as referências textuais ao nome exato do header são includes em variantes de `sgxdefs.h`; não foi encontrada uma lista de manifesto textual que acrescente um hash do arquivo. O Git, entretanto, dá hash e associação inequívoca a árvores. [DEFS:57–61](archaeology-data/DEFS.txt#L57); [matches KM](archaeology-data/omap5-sgx-ddk-linux-matches.tsv); [contexts KM](archaeology-data/omap5-sgx-ddk-linux-contexts.tsv). Os manifestos binários UM não foram convertidos integralmente; um resultado negativo sobre seu conteúdo permanece **UNKNOWN**.

## Descobertas históricas principais

| Descoberta | Commit/ref | Arquivo e linhas | Confidence |
| --- | --- | --- | --- |
| Importação inicial 1.9 já referencia 535, mas árvore não contém header | KM `1450ae2166ad952ef30197e79518b51577a629e6`; ancestral de master | `services4/srvkm/hwdefs/sgxdefs.h`:54–58; árvore em boundary-trees | CONFIRMED |
| Primeiro ingresso encontrado de H535, DDK 1.13 experimental | KM `322bcda5f3076037e2e20ef9209f4f4d575a7d5f`; ancestral de origin/dra7/experimental | [H535:1–43](archaeology-data/H535.txt#L1); [VER13:51–60](archaeology-data/VER13.txt#L51) | CONFIRMED |
| Renomeação com conteúdo igual para eurasia_km | KM `a24ae6b2573b7eb1dc94473aa9953b964079b5c6`; origin/dra7/experimental | paths de H535, R100; linhas 1–739 preservadas | CONFIRMED |
| Importação 1.14 inclui header e plataforma Poulsbo | KM `cb46ba4d0c900f89f7ec0284f9803d476bfa98de`; origin/img-sgx | [PBUILD:42–52](archaeology-data/PBUILD.txt#L42); [PSYS:46–111](archaeology-data/PSYS.txt#L46) | CONFIRMED |
| Outra linhagem reaplica importação 1.14 sem prefixo eurasia_km | KM `7c89d3433bd96d8b2755ca172e99198fa4b69c05`; ancestral de origin/1.17.4948957/mesa/k6.1 | `services4/srvkm/hwdefs/sgx535defs.h`:1–739, mesmo blob | CONFIRMED |
| Remoção do alvo e da integração Poulsbo, não de H535 | KM `3b6ca1d1f47a951c1f93ba5f9b693b25474d0798` e replay `636e957a340ebba5c10fb8a5c5b3f30b85078c66` | PBUILD e PSYSC, arquivo inteiro removido; H535 permanece | CONFIRMED |

Fontes das operações de árvore (linhas de código N/A): [header-history.txt](archaeology-data/header-history.txt), [poulsbo-history.txt](archaeology-data/poulsbo-history.txt), [boundary-trees.txt](archaeology-data/boundary-trees.txt). Datas de autor e committer foram preservadas: uma data de autor antiga em commit reaplicado não prova que aquela árvore foi publicada naquele ano.

## Catálogo de fontes

Cada ID abaixo é uma citação completa recuperável com `git -C references/REPO show COMMIT:ARQUIVO`. As referências `ID:linha` nos documentos usam **linhas originais do blob**, também preservadas no snapshot `.txt`. Confidence dos registros: **CONFIRMED** para identidade e conteúdo; inferências são marcadas no texto consumidor.

| ID | Repositório | Commit | Arquivo original | Blob / snapshot |
| --- | --- | --- | --- | --- |
| H535 | `omap5-sgx-ddk-linux` | `322bcda5f3076037e2e20ef9209f4f4d575a7d5f` | `services4/srvkm/hwdefs/sgx535defs.h` | `8039da4a73ef9ee3e929edb64244d2891bc9239e` / [H535.txt](archaeology-data/H535.txt) |
| PBUILD | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/eurasiacon/build/linux2/pc_i686_poulsbo_d0_linux/Makefile` | `86c0adfe78965f7d304e912f90a302dcfd4f119c` / [PBUILD.txt](archaeology-data/PBUILD.txt) |
| CORE | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/eurasiacon/build/linux2/config/core.mk` | `55c58fc853dbf4e0c3804cbac3283f1e15e3eabc` / [CORE.txt](archaeology-data/CORE.txt) |
| PSYS | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/system/poulsbo/sysconfig.h` | `5a47b6bad85388eac97da74ba8ca3cf3b19471b0` / [PSYS.txt](archaeology-data/PSYS.txt) |
| PSYSC | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/system/poulsbo/sysconfig.c` | `9c252ce8ae5d5a2de681294ad8bfd338cea95469` / [PSYSC.txt](archaeology-data/PSYSC.txt) |
| PINFO | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/system/poulsbo/sysinfo.h` | `a5539cc7554fb34b283cc27a940096b54eb4ab82` / [PINFO.txt](archaeology-data/PINFO.txt) |
| FEATURE | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h` | `de07cc890c0194d9bc02997176d6eb1eda048811` / [FEATURE.txt](archaeology-data/FEATURE.txt) |
| ERRATA | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/hwdefs/sgxerrata.h` | `e423d4fbaa566fe710c7b44e8cc25efe4a847a8c` / [ERRATA.txt](archaeology-data/ERRATA.txt) |
| DEFS | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/hwdefs/sgxdefs.h` | `7acba793124ebaa2c0c2478d321b030df5362cdc` / [DEFS.txt](archaeology-data/DEFS.txt) |
| MMU | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/hwdefs/sgxmmu.h` | `a6a907aecb69fecb2c8ba01fb213736943fa3588` / [MMU.txt](archaeology-data/MMU.txt) |
| RESET | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/devices/sgx/sgxreset.c` | `ecf0e6207ca8b104b648fc80924daaf7e184ae48` / [RESET.txt](archaeology-data/RESET.txt) |
| VER19 | `omap5-sgx-ddk-linux` | `1450ae2166ad952ef30197e79518b51577a629e6` | `include4/pvrversion.h` | `ca8bead1cb03245e15d97ed36e98e712d5fe7960` / [VER19.txt](archaeology-data/VER19.txt) |
| VER13 | `omap5-sgx-ddk-linux` | `322bcda5f3076037e2e20ef9209f4f4d575a7d5f` | `include4/pvrversion.h` | `f272e696dc577080de853df51ef16c524a896996` / [VER13.txt](archaeology-data/VER13.txt) |
| VER14 | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/include4/pvrversion.h` | `4fb45c1f330a33dca1be6b7b8785d6025864469d` / [VER14.txt](archaeology-data/VER14.txt) |
| VER17 | `omap5-sgx-ddk-linux` | `b630d462f5fbb86e5f98965ba1af35da1207822f` | `eurasia_km/include4/pvrversion.h` | `6f28525bf332bf7ea8929cf6ba46d3f555f76b49` / [VER17.txt](archaeology-data/VER17.txt) |
| LREG | `linux` | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` | `drivers/gpu/drm/gma500/psb_reg.h` | `2a229a0ef36c0d76d9eea29ad75107b5784f4a08` / [LREG.txt](archaeology-data/LREG.txt) |
| LDRVH | `linux` | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` | `drivers/gpu/drm/gma500/psb_drv.h` | `db197b865b90a020ac0975c4cc6a5bd00627a29a` / [LDRVH.txt](archaeology-data/LDRVH.txt) |
| LDRVC | `linux` | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` | `drivers/gpu/drm/gma500/psb_drv.c` | `d17cb5b4a4bf2d80e8fa0e83dd09cabff5b20fe1` / [LDRVC.txt](archaeology-data/LDRVC.txt) |
| LMMU | `linux` | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` | `drivers/gpu/drm/gma500/mmu.c` | `4fbc22a59ac7a9e76a373b65cd7fcf97aa1605ba` / [LMMU.txt](archaeology-data/LMMU.txt) |
| LIRQ | `linux` | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` | `drivers/gpu/drm/gma500/psb_irq.c` | `c224c7ff353ce35ba36b3c2b0a4cff4bff478578` / [LIRQ.txt](archaeology-data/LIRQ.txt) |
| UM19 | `omap5-sgx-ddk-um-linux` | `225230e5dae17836a8acf6f6e9fb6e8200af4dea` | `README` | `5284065ebe40bbbfb044e720d325ed7b72390032` / [UM19.txt](archaeology-data/UM19.txt) |
| UMROOT | `omap5-sgx-ddk-um-linux` | `348ea7e08d8b6e7f46f73065ad3ba75614edc12c` | `README` | `9e40cefcb46480543c2413014a4407196c2c9539` / [UMROOT.txt](archaeology-data/UMROOT.txt) |

Snapshots são cópias documentais com avisos originais, não código novo de driver. Licenças não se propagam automaticamente do KM ao UM ou do header a outros arquivos. Binários UM permanecem fora desses snapshots. A análise registra proveniência, sem declarar permissão geral de reutilização.
