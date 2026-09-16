# SGX535: arquivos recuperados e lacunas restantes

Convenção: **CONFIRMED** confirma conteúdo/histórico das fontes, não funcionamento no hardware; **INFERRED** identifica uma dedução; **UNKNOWN** é o que ainda não foi estabelecido. Identificadores de fonte como H535 e PBUILD resolvem para repositório, commit completo, arquivo e hash no [catálogo de fontes](source-archaeology.md#catálogo-de-fontes). Os snapshots documentais preservam as linhas originais e os avisos de licença; não são arquivos de implementação.

## sgx535defs.h: resposta às cinco perguntas

| Pergunta | Resposta | Evidência / confidence |
| --- | --- | --- |
| Existiu em commit? | Sim, em 255 commits disponíveis | H535, blob `8039da4a73ef9ee3e929edb64244d2891bc9239e`; [H535:1–43](archaeology-data/H535.txt#L1); CONFIRMED |
| Foi removido? | Nenhuma remoção do header encontrada; não constava na importação 1.9/master | header-history e boundary-trees; operação Git, linhas N/A; CONFIRMED neste universo |
| Está em branch/tag? | Presente no tip de 13 remote-tracking branches; ausente no tip de todas as 12 tags KM disponíveis | header-by-ref, linhas de inventário; CONFIRMED |
| É referenciado por build? | Dispatcher inclui o header; alvo Poulsbo define core 535/rev121; build gera SGX535 e SUPPORT_SGX535 | [DEFS:57–61](archaeology-data/DEFS.txt#L57); [PBUILD:44–50](archaeology-data/PBUILD.txt#L44); [CORE:488–494](archaeology-data/CORE.txt#L488); CONFIRMED no código, expansão INFERRED |
| Pode ser identificado por hash/lista? | Sim: árvore Git, blob único, SHA-256 e tamanho | source-registry.json, H535; CONFIRMED. Manifesto externo correspondente UNKNOWN |

Fontes de inventário: [por ref](archaeology-data/header-by-ref.tsv), [história](archaeology-data/header-history.txt), [summary](archaeology-data/summary.json). Nenhum hash foi inventado para uma versão ausente: este hash é do conteúdo realmente disponível.

## Branches com o header no tip

| Ref local | Commit completo | Caminho |
| --- | --- | --- |
| `refs/remotes/origin/1.17.4948957/mesa/k6.1` | `9ae0fa4998b1c624408945e062bf8fb0ea7efb9d` | `services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/devel-next` | `62f31de3ceed156bff32abb3dd03693a02df117e` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/dra7/experimental` | `a24ae6b2573b7eb1dc94473aa9953b964079b5c6` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/img-sgx` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.14.3699939/k4.1` | `fed0756f1b8b9d526da2821635c7d742989d47c3` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.14.3699939/k4.14` | `76da7d73976f0a5dc04fdc84a3af899d6c2b1fe2` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.14.3699939/k4.4` | `fd47e44b18944cf7ade480ac67a9c0172619ff7e` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.14.3699939/k4.9` | `0086977380d3320d70a3abc78b95fa0641427073` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.17.4948957/k4.14` | `c545bf1c937b6067d27b7a268093baa3b8091185` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.17.4948957/k4.19` | `2a777b8fb72a89d299b82845d42b63b2a2618daa` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.17.4948957/k5.10` | `eda7780bfd5277e16913c9bc0b0e6892b4e79063` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.17.4948957/k5.4` | `bfe83bbabb3849c24b03d5172cf678e7c5915e04` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.17.4948957/k6.1` | `bfd9edef3f976906fb7fececbfa29d16801154f5` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |

Todas as linhas acima têm confidence **CONFIRMED** por `ls-tree`; linhas de código: H535:1–739. `origin/*` são refs já presentes no clone, não consulta ao servidor. Não há motivo para trocar de branch para ler o arquivo: `git show` resolve.

## O que foi removido de fato

A série DDK 1.17 inicialmente ainda carregava arquivos Poulsbo herdados. O commit `3b6ca1d1f47a951c1f93ba5f9b693b25474d0798` removeu `eurasia_km/eurasiacon/build/linux2/pc_i686_poulsbo_d0_linux/Makefile` e a integração `eurasia_km/services4/system/poulsbo`; a linhagem sem prefixo contém a operação correspondente em `636e957a340ebba5c10fb8a5c5b3f30b85078c66`. **CONFIRMED:** os registros de remoção não se referem a `sgx535defs.h`, que continua nas árvores. [História de Poulsbo](archaeology-data/poulsbo-history.txt), [fronteiras](archaeology-data/boundary-trees.txt). Linhas removidas do Makefile: 1–86; de sysconfig.c: arquivo inteiro, consultável no pai.

**UNKNOWN:** se a retirada significou descontinuação de suporte validado, limpeza de código não utilizado ou outra decisão. O título “remove DDK 1.14 specific files” não documenta o estado real do hardware ou do produto.

## O que o header resolve e o que não resolve

**CONFIRMED:** adiciona definições específicas SGX535 para clock/status/override, eventos, timer, invalidação PDS, BIF, 16 directory lists, bancos e 2D. Por exemplo, lista 1 = `0x0c38`, lista 2 = `0x0c3c`; BANK0 e BANK1 possuem campos EDM/TA/HOST/3D/2D. [H535:45–88](archaeology-data/H535.txt#L45); [H535:375–458](archaeology-data/H535.txt#L375); [H535:539–638](archaeology-data/H535.txt#L539).

**UNKNOWN:** datasheet completo, campos reservados, todos os efeitos de leitura/escrita, todas as revisões do silício e validação em Poulsbo. Ter esse arquivo não autoriza um dump arbitrário de registradores. O conjunto de declarações do header não é uma especificação completa da ISA USSE ou dos streams de renderização.

| Artefato | Estado nesta fase | Fonte exata / confidence |
| --- | --- | --- |
| Header SGX535 | Recuperado documentalmente do Git | [H535:1–739](archaeology-data/H535.txt#L1); CONFIRMED |
| Build e wrappers Poulsbo | Localizados em DDK 1.14 | [PBUILD:42–86](archaeology-data/PBUILD.txt#L42); [PSYSC:145–217](archaeology-data/PSYSC.txt#L145); CONFIRMED |
| Header de interface KM microkernel | Já existe; não é implementação do firmware | fase 1 `sgx_mkif_km.h`, [firmware.md](firmware.md); CONFIRMED para o contrato |
| `psb_powermgmt.h` e `sys_pvr_drm_export.h` requeridos pelo ramo externo | Referenciados; não localizados como caminhos nos commits KM ou no snapshot Linux | [PSYSC:65–69](archaeology-data/PSYSC.txt#L65); logs de caminhos; CONFIRMED como dependência ausente neste conjunto |
| Inicializador UM Poulsbo compatível e scripts preenchidos | Não localizado como fonte | buscas globais e alvo [PBUILD:62–66](archaeology-data/PBUILD.txt#L62); UNKNOWN quanto a disponibilidade externa |
| Fonte do microkernel e programas PDS SGX535 | Não localizada | inventário global; UNKNOWN quanto a implementação e licença externa |
| ISA/assembler/compiler USSE SGX535 | Não localizado como fonte | busca global; compiladores UM encontrados são binários nomeados 530/544, não 535; CONFIRMED sobre os nomes, compatibilidade UNKNOWN |

Não confundir nomes `sgx_ukernel_status_codes.h`, flags USSE e constantes EURASIA com a presença de fonte de firmware ou compilador. Os compiladores publicados no commit UM `5830fc09a5f37ca89b718d7c60a838e0267a6371` são `glslcompiler_SGX544_116.exe` e `glslcompiler_SGX530_120.exe`, conforme [registro Git com paths e hashes](archaeology-data/um-offline-compilers.txt); linhas de código N/A, confidence CONFIRMED para os artefatos. Não foram executados nem desassemblados.
