# Proveniência da fase 3

Os snapshots `.txt` são referências documentais, não uma implementação nova. Preservam bytes, avisos de licença e numeração de linhas dos arquivos consultados. Não compilar ou executar esse material como parte desta fase.

- `sources.json`: repositório, commit completo, caminho original, snapshot, número de linhas e SHA-256. Os caminhos `../archaeology-data/` reutilizam snapshots da fase 2 sem duplicação.
- `external-trees.json`: árvores Git retornadas pela API GitHub, fixadas nos commits PSB `98b5307e5158a9ac401b29128ddd1184ae06b4d7` e EMGD `e6884ec2eaaf1afe88d5ff9dd44d70403525be5b`. Não houve clone/fetch nos repositórios de referência.
- `extended-history-matches.json`: busca sem distinção de caixa por `EMGD|US15W(?:P|PT)?|Atom.{0,12}Z5|Poulsbo|psb_powermgmt|sys_pvr_drm_export` nos blobs alcançáveis por todas as refs locais TI KM/UM. Objetos foram lidos via `git rev-list --objects --all` e `git cat-file --batch`; arquivos com NUL nos primeiros 8192 bytes foram excluídos da interpretação textual. Não é busca em conteúdo binário.
- `extended-history-contexts.tsv`: join desses blobs com o inventário completo de árvores da fase 2, fornecendo commit e arquivo para as ocorrências. O campo `line` está em matches.json. Nenhum blob com match ficou sem contexto.
- `header-comparison.json`: extração de `#define NOME constante_decimal_ou_hex[UL]*`, normalizada numericamente, dos headers SGX535 TI e EMGD. Macros-função e expressões não são comparadas; `null` representa ausência no conjunto extraído, não ausência física de hardware.
- `validation.json` e `validate.py.txt`: conferência de hashes, snapshots Git locais, intervalos, links, IDs da matriz e estado dos repositórios. Escopo: documentos da fase 3 e unknowns atualizado. Relatórios anteriores mantêm suas citações e validação da fase 2; a matriz não reclassifica automaticamente suas alegações históricas.

A matriz oficial Intel é uma referência externa com identificador documental, página e URL, sem commit Git. Ela não foi republicada integralmente aqui. Todas as outras linhas da matriz da fase 3 apontam a um arquivo com commit fixado.

Fontes de espelhos são evidência do conteúdo encontrado, não garantia de autenticidade de pacote Intel nem documentação oficial de hardware. Para licenças, consultar a análise por arquivo em [historical ABI](../poulsbo-historical-abi.md). Nenhum binário externo foi importado, carregado ou desassemblado.
