# Mapa de versões DDK e evidência SGX535

Convenção: **CONFIRMED** confirma conteúdo/histórico das fontes, não funcionamento no hardware; **INFERRED** identifica uma dedução; **UNKNOWN** é o que ainda não foi estabelecido. Identificadores de fonte como H535 e PBUILD resolvem para repositório, commit completo, arquivo e hash no [catálogo de fontes](source-archaeology.md#catálogo-de-fontes). Os snapshots documentais preservam as linhas originais e os avisos de licença; não são arquivos de implementação.

## Versões identificadas por conteúdo, não só por nomes de branches

| Versão IMG / build | Commit representativo TI KM | Evidência | SGX535 / Poulsbo | Confidence |
| --- | --- | --- | --- | --- |
| 1.9, branch numérico 19, build 2253347 | `1450ae2166ad952ef30197e79518b51577a629e6` | [VER19:55–65](archaeology-data/VER19.txt#L55) | dispatcher/feature/errata 535 existem; header ausente nessa árvore; sem alvo Poulsbo localizado nessa importação | CONFIRMED sobre conteúdo; build funcional 535 UNKNOWN |
| 1.13 build 3444720, experimental | `322bcda5f3076037e2e20ef9209f4f4d575a7d5f` | [VER13:51–60](archaeology-data/VER13.txt#L51) | primeiro ingresso local encontrado do header 535; não contém alvo pc_i686_poulsbo_d0_linux | CONFIRMED sobre conteúdo; integração funcional UNKNOWN |
| 1.14 build 3699939 | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de`, origin/img-sgx | [VER14:51–60](archaeology-data/VER14.txt#L51) | alvo Poulsbo D0 seleciona SGX535 rev121; header e system/poulsbo presentes | CONFIRMED: alvo explicitamente configurado; execução não validada |
| 1.17 build 4948957, variante SGX_DDK_Linux_XOrg | `b630d462f5fbb86e5f98965ba1af35da1207822f` | [VER17:51–60](archaeology-data/VER17.txt#L51) | esse commit ainda tem alvo e wrappers herdados; removidos no filho de limpeza | CONFIRMED histórico; suporte validado em 1.17 UNKNOWN |
| 1.17 build 4948957, variante SGX_DDK | `01a9d880722e93d8f8321e4f9be64dfae19e85a3` | `eurasia_km/include4/pvrversion.h`:51–57, blob `86a733d2fc714eedd0dcea03d50dfce1947ab62e` | diferenças de identificação de produto não provam mudança de core | CONFIRMED sobre versão; compatibilidade UNKNOWN |

Foram encontrados **cinco blobs distintos de pvrversion.h**, representando quatro pares versão/build; os dois de 1.17 diferem na identificação do produto. Inventário com commit, caminho, hash e linhas: [version-headers.json](archaeology-data/version-headers.json). Não interpretar `PVRVERSION_BRANCH=19` como DDK 1.19: major/minor são 1/9 na mesma fonte.

**CONFIRMED:** o histórico KM disponível começa numa importação 1.9; não foi encontrada fonte de uma versão anterior a 1.9. Isso não prova que versões SGX535 mais antigas não existam externamente. **INFERRED:** 1.14.3699939 é a melhor âncora local para investigar integração Poulsbo, pois combina versão explícita, header específico e alvo Intel/revisão no mesmo commit. [VER14:51–60](archaeology-data/VER14.txt#L51); [PBUILD:44–52](archaeology-data/PBUILD.txt#L44).

## Tags e branches não são versões intercambiáveis

A tag `TI_LINUX_OMAP_SGX_DDK_1.9_2253347` aponta para `6668f4ea0bbf7e3df7f72dc2230ab8da1e748b2d`. `ti_imgddk_1.9.0.11` aponta para `7e084e0452bfaa495d2a9c06c7a420bac35d8d99`; `glsdk_7.01.00.03` e master apontam para `430673f78b79eccdf308a6bbfb524209b485d2cc`. **CONFIRMED como refs locais**; linhas de código N/A. Todas as 12 tags KM disponíveis carecem do header no tip. [Refs KM](archaeology-data/omap5-sgx-ddk-linux-refs.tsv); [presença por ref](archaeology-data/header-by-ref.tsv).

`origin/dra7/experimental` termina em `a24ae6b2573b7eb1dc94473aa9953b964079b5c6`, com 1.13 movido para `eurasia_km`. `origin/img-sgx` ancora 1.14. Branches `ti-img-sgx/1.17.4948957/*` mantêm o header, mas seus tips examinados não mantêm necessariamente o alvo Poulsbo. É preciso verificar arquivo e commit, não inferir por nome de branch. **CONFIRMED:** [header-by-ref.tsv](archaeology-data/header-by-ref.tsv).

## História UM

**CONFIRMED:** a raiz UM `225230e5dae17836a8acf6f6e9fb6e8200af4dea` declara TI DDK 1.9.11 sobre IMG 1.9 ED2253347 e pacote OMAP5/DRA7xx. [UM19:19–41](archaeology-data/UM19.txt#L19); [UM19:57–61](archaeology-data/UM19.txt#L57). Isso não estabelece ABI compatível com Poulsbo.

A outra raiz, `348ea7e08d8b6e7f46f73065ad3ba75614edc12c`, possui autor datado de 2013 mas committer de 2023 e README que declara 1.17. **CONFIRMED:** metadados em [commits UM](archaeology-data/omap5-sgx-ddk-um-linux-commits.tsv) e [UMROOT:33–36](archaeology-data/UMROOT.txt#L33). Portanto, ordenar só por data de autor produziria uma cronologia enganosa. “Raiz” aqui é raiz do grafo disponível, não nascimento do DDK.

Refs UM nomeiam séries 1.14.3699939 e 1.17.4948957; o conteúdo fonte pesquisável não oferece SGX535/Poulsbo. Há 1802 blobs classificados como binários, cujo conteúdo não foi usado para inferir suporte 535. Os compiladores offline SGX530/544 adicionados em 2018 demonstram distribuição desses artefatos, não fonte do compilador nem suporte SGX535. [Refs UM](archaeology-data/omap5-sgx-ddk-um-linux-refs.tsv); [inventário da importação](archaeology-data/um-offline-compilers.txt), arquivos binários: linhas N/A, confidence CONFIRMED sobre nomes e hashes.

## Limite da conclusão de compatibilidade

O alvo Poulsbo D0 **seleciona** rev121; não demonstra que toda unidade 8086:8108, 8109 ou todo stepping de GMA500 tenha essa revisão. O header de erratas trata 121, 126 e HEAD. [PBUILD:47–50](archaeology-data/PBUILD.txt#L47); [ERRATA:152–178](archaeology-data/ERRATA.txt#L152). Essa associação de build é um avanço factual, não identificação do hardware do usuário.

Não foi construído nem executado qualquer DDK. Compatibilidade de kernel, userspace, firmware, build options e estruturas permanece **UNKNOWN** até que o conjunto correspondente seja obtido e analisado. Nenhuma feature de SGX540/544 foi atribuída ao 535.
