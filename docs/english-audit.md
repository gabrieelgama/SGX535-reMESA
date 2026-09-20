# English-language audit

Date: 2026-09-20

I audited every tracked Markdown, CSV, text, JSON, HTML, README, and source-adjacent documentation file outside `references/`. The audit covered project prose, tables, metadata, experiment descriptions, generated indexes, and the evidence matrix. I then repeated the language and malformed-word searches after editing.

## Corrections

The previous translation pass left mixed Portuguese/English prose across the architecture, MMU, command-submission, gma500, Poulsbo, ownership, power, recovery, register, unknowns, and Test Vector Zero documents. I translated those remnants and normalized recurring terms such as *read side effects*, *power state*, *clock state*, *mapping lifetime*, *stolen memory*, and *SGX core revision*.

Two provenance READMEs had been damaged by mechanical replacements. I reconstructed [Phase 4.1 sources](phase4-1-data/README.md) and [Phase 4.2 sources](phase4-2-data/README.md) from their manifests and archived artifacts, restoring URLs, filenames, document numbers, versions, dates, commits, license qualifications, and provenance limits.

I corrected malformed words including `invalidateidation`, `invalidateidates`, `Phaif`, `Dattheheet`, `Htheh`, and related replacement fragments. The regenerated [consistency inventory](phase4-program-data/consistency-inventory.json) now quotes current English project text instead of obsolete Portuguese excerpts.

The register-read table still marked write-control registers `UNSAFE` after the Phase 4.4 adversarial report had explicitly downgraded their *read* semantics to `UNKNOWN`. I aligned the table with that existing conclusion. This does not approve any read: neither candidate is a `SAFE-CANDIDATE`, the whitelist remains empty, and Gate B remains `BLOCKED`.

## Files changed

- Core architecture and interface documentation: `architecture.md`, `command-submission.md`, `ddk-version-map.md`, `firmware.md`, `gma500-current-state.md`, `mmu-bif.md`, `registers.md`, and `ti-vs-poulsbo.md`.
- Poulsbo documentation: `poulsbo-bringup-requirements.md`, `poulsbo-evidence.md`, `poulsbo-historical-abi.md`, `poulsbo-interrupts.md`, `poulsbo-memory-map.md`, `poulsbo-mmio.md`, and `poulsbo-power-reset.md`.
- Phase 4 safety and provenance documentation: `hardware-identification.md`, `hardware-ownership.md`, `power-state-safety.md`, `recovery-plan.md`, `safe-register-reads.md`, `phase4-audit.md`, `phase4-go-no-go.md`, `phase4-1-data/README.md`, `phase4-2-data/README.md`, and `phase4-program-data/consistency-inventory.json`.
- Source archaeology and open questions: `sgx535-missing-files.md`, `source-archaeology.md`, and `unknowns.md`.
- Evidence records: `evidence.txt` and `evidence-matrix.csv`.
- Experiment documentation: `experiments/bringup0/TEST-VECTOR-ZERO.md`.

## Intentionally preserved non-English material

- `docs/hardware-evidence/TVZ-001/operator-report.txt` is the original Portuguese hardware report supplied for Test Vector Zero. Its SHA-256 digest is part of the evidence record, so it remains byte-for-byte unchanged.
- Files under `docs/archaeology-data/`, source snapshots under `docs/poulsbo-data/`, archived kernel/source material under `docs/phase4-1-data/` and `docs/phase4-2-data/`, `docs/phase4-2-data/rev116-mail.html`, and `docs/phase4-2-data/antix-package.diff` preserve upstream source, quotations, names, addresses, or archived material. Non-English text in those artifacts is source material rather than project prose.
- Register names, symbols, paths, commands, evidence IDs, document numbers, hashes, and quoted source text remain exact.

The audit changed no implementation and performed no hardware access. Evidence classifications, safety gates, source locators, and the empty MMIO whitelist remain unchanged.
