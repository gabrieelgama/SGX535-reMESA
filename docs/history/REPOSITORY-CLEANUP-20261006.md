# Repository cleanup and retention decisions

This pass changed navigation and public explanation, not rendering behavior.
It preserved the research record instead of treating deletion count as progress.

## Inventory

The initial inventory contained 2,779 tracked paths and 4,587 non-cache working
files, approximately 448 MB. Most working files (4,401) were under `docs/`,
including archives that had not yet been added to Git. The source tree was already
dirty. No pre-existing source edits were reset, staged or discarded.

| Category | Locations and decision |
| --- | --- |
| Current source | `kernel/`, `tools/`: preserved; new offline verifier added separately |
| Current documentation | Root README, documentation index, overview and reproduction entry points rewritten/added |
| Reproduction material | Phase 8 manifests, build inputs and actual historical procedures retained at their paths |
| Evidence | Hardware captures, FIRE #2/#3, first observed display and square bundles retained byte-identical |
| Historical research | Early top-level phase reports and Phase 7 notes retained; indexed from the history summary |
| Superseded navigation | Previous Phase 8 index preserved in `docs/history/PHASE8-INDEX-BEFORE-CLEANUP-20261006.md`; relative links adjusted for its new location |
| Generated artifacts | Qualified modules/images are evidence, not accidental scratch output; retained |
| Temporary files | Python caches already ignored; root build/dist, local reproduction output, coverage and editor scratch now ignored |
| Duplicate / unknown purpose | No evidence duplicate or unique technical file deleted without a demonstrated replacement |

No research file was removed. The previous Phase 8 navigation was archived as a
snapshot, and its current page replaced with a concise index. Early phase files
were not mass-moved: their paths participate in citations and historical manifests.
This gives readers a history route without breaking those relationships.

## Large files and publication

Six files exceed 10 MB: one raw framed capture and five boot images. Two image
pairs share much of their contents but identify different experiments. Do not
remove them as duplicates. Normal Git is a poor long-term home for repeated boot
images; a versioned release/evidence archive with a checked checksum manifest is
a better prospective distribution mechanism. No LFS migration or history rewrite
was performed, and guides fail clearly if required archive files are unavailable.

The [privacy audit](../PRIVACY-AUDIT-20261006.md) records the remaining raw-evidence
publication decision. This cleanup does not authorize adding all untracked files
or claim that every existing raw capture is privacy-safe.

## Public workflow

`tools/reproduction/verify.py` is a read-only local verifier for both recorded
results and their archive inventories. It does not automate historical live
controllers or weaken their ownership/provenance checks. The public guides state
where exact binaries can be reused, where source-only reproduction is incomplete,
and which fresh bindings a new hardware experiment requires.

The old README's useful project identity, hardware background, humor and credits
are retained in shorter form. Historical blocked gates and old execution cards
are no longer the introductory user interface. Technical comments and qualified
rendering source were left unchanged; no safe deletion was established there.

## Checks and limitations

Nine verifier regression tests cover exact footprints, wrong scenes/colors,
truncation, missing artifacts, path escape and hash mismatch. Original rendering
qualification was not repeated. The archive verifier checks 223 triangle entries
and 252 square/candidate entries. A separate preservation comparison checks all
pre-existing artifact/capture/source files against this pass's starting hashes.

Public navigation links and JSON syntax were checked offline. In eight non-sealed
research pages, 104 local-editor `file:line` citations were converted to portable
`file#Lline` links; the cited files and line numbers were retained. Historical source
snapshots have relative links whose original checkout context is no longer present;
local third-party reference links require separately obtained checkouts. Such links
are recorded as archive/context limitations, not silently advertised as clean
GitHub navigation. No hardware or SGX work occurred.

Final reference review inspected 3,078 local Markdown targets. All 298 missing
file targets were inside historical artifact/capture contexts; no missing file
target remained in non-archive documentation. There were also 255 external-checkout
references and 111 machine-absolute links. Those are context dependencies, not
portable public navigation. The [link ledger](../publication-audit/link-review-20261006.json)
records them without changing originals. New reader-facing navigation passed.
All 1,170 JSON documents present at the initial validation parsed successfully;
new audit/status JSON was checked again after creation. `git diff --check` passed.
The [preservation receipt](../publication-audit/preservation-check-20261006.json)
records the 68-file FIRE #2 baseline, triangle, square and display seals.
