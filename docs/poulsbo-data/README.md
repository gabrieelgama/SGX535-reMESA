# Provenance of phase 3

The `.txt` snapshots are documentary references, not a new implementation. They preserve bytes, license warnings, and line numbering of the consulted files. Do not compile or execute this material as part of this phase.

- `sources.json`: repository, complete commit, original path, snapshot, number of lines, and SHA-256. The `../archaeology-data/` paths reuse phase 2 snapshots without duplication.
- `external-trees.json`: Git trees returned by the GitHub API, fixed on commits PSB `98b5307e5158a9ac401b29128ddd1184ae06b4d7` and EMGD `e6884ec2eaaf1afe88d5ff9dd44d70403525be5b`. There was no clone/fetch in the reference repositories.
- `extended-history-matches.json`: search without case distinction by `EMGD|US15W(?:P|PT)?|Atom.{0,12}Z5|Poulsbo|psb_powermgmt|sys_pvr_drm_export` in the blobs reachable by all local refs TI KM/UM. Objects were read via `git rev-list --objects --all` and `git cat-file --batch`; files with NUL in the first 8192 bytes were excluded from textual interpretation. This is not a search in binary content.
- `extended-history-contexts.tsv`: join these blobs with the complete tree inventory from phase 2, providing commit and file for the occurrences. The field `line` is in matches.json. No matching blob was left without context.
- `header-comparison.json`: extraction of `#define NAME decimal_or_hex_constant[UL]*`, numerically normalized, from the SGX535 TI and EMGD headers. Macros-functions and expressions are not compared; `null` represents absence in the extracted set, not physical hardware absence.
- `validation.json` and `validate.py.txt`: hash checks, local Git snapshots, ranges, links, matrix IDs, and repository status. Scope: phase 3 documents and updated unknowns. Previous reports retain their citations and phase 2 validation; the matrix does not automatically reclassify their historical claims.

The official Intel matrix is an external reference with a document identifier, page, and URL, without a Git commit. It has not been republished in full here. All other lines of the phase 3 matrix point to a file with a fixed commit.

Mirror sources are evidence of the content found, not a guarantee of Intel package authenticity or official hardware documentation. For licenses, consult the file analysis in [historical ABI](../poulsbo-historical-abi.md). No external binary was imported, loaded, or disassembled.
