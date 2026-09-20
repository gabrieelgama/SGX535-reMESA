# Documentation Style Cleanup Implementation Plan

**Goal:** Convert project documentation to natural technical English and improve consistency without changing technical conclusions, provenance, safety gates, or uncertainty classifications.

**Architecture:** Inventory all tracked Markdown and CSV documentation, translate only Portuguese prose, then normalize repetitive audit language while preserving structured evidence and exact source material. Validate links, evidence references, hashes, and clean reference repositories afterward.

**Tech Stack:** Markdown, CSV, ripgrep, Python validation scripts, Git diff checks.

## Global Constraints

- Do not modify implementation behavior.
- Do not modify `references/`.
- Preserve source provenance, hashes, citations, gate states, and `CONFIRMED` / `INFERRED` / `UNKNOWN` classifications.
- Do not translate code, commands, identifiers, filenames, quoted source text, or exact terminology.
- Do not execute hardware operations.

## Review Focus

- Portuguese prose must be translated without altering technical meaning.
- Safety language must remain conservative and must not unlock any gate.
- Evidence references, line ranges, hashes, and CSV fields must remain valid.
- Historical platform distinctions must remain explicit.
- The documentation should read as first-person project notes where appropriate, not as an AI-generated audit.

### Tasks

- [ ] Inventory documentation and identify Portuguese prose, repeated boilerplate, and structured content that must remain exact.
- [ ] Translate and edit documentation in bounded file groups while preserving evidence-bearing fields and quoted material.
- [ ] Normalize cross-document terminology and first-person references.
- [ ] Run repository validators, link checks, diff checks, and reference-tree integrity checks.
- [ ] Review the final diff for technical and classification changes.
