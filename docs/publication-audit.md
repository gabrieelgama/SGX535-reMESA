# Public commit preparation audit

2026-10-01. PUBLICATION STATUS: BLOCKED — REVIEW REQUIRED.

This was a local working-tree audit. It did not contact the Mini 12, stage a
hardware experiment or change research results. Gate B remains BLOCKED,
whitelist `[]`, and no unauthorized SGX execution is permitted. No commit or
push was made.

## Handoffs and public technical records

Files moved: NONE. Every identified legacy handoff has independent technical
value or recorded evidence dependencies. Moving them would change recorded
paths used by inventories, links, scripts or scoped diffs. They remain unchanged.
This is a limit of the cleanup, not a claim that all legacy session instructions
have been removed.

Future AI-only prompts and session-continuation notes belong in
`internal/ai-handoffs/`, which Git now ignores. Technical facts must remain in
public research records. The [internal note policy](../internal/README.md)
explains the distinction.

| Retained file | Reason |
| --- | --- |
| `docs/phase7/psb-dri-re/CODEX-HANDOFF.md` | Technical Phase 7 closure checkpoint, linked by research and hardware records; present in eleven historical hash/provenance inventories. |
| `docs/phase7/psb-dri-re/CODEX-HANDOFF-chronology.md` | Archived research and experiment chronology; linked by the closure checkpoint and present in eleven historical hash/provenance inventories. |
| `docs/phase8/CHATGPT-HANDOFF-POST-ATTEMPT03.md` | Mixed technical checkpoint and session instructions; referenced by preserved update/integrity scripts, inventories, audits and scoped diffs. |
| `docs/phase8/artifacts/candidate-01-build-20260930/documentation-before/CHATGPT-HANDOFF-POST-ATTEMPT03.md` | Manifest-bound before snapshot for the candidate build record. |
| `docs/phase8/artifacts/candidate-01-build-20260930/scoped-diff-CHATGPT-HANDOFF-POST-ATTEMPT03.md.txt` | Manifest-bound before/after diff; its recorded source paths are historical evidence. |
| `docs/phase8/artifacts/lifecycle-gate-review-20261001/before/docs/phase8/CHATGPT-HANDOFF-POST-ATTEMPT03.md` | Manifest-bound before snapshot for the lifecycle review. |
| `docs/phase8/artifacts/lifecycle-gate-review-20261001/scoped-diff-CHATGPT-HANDOFF-POST-ATTEMPT03.md.txt` | Manifest-bound lifecycle review diff; preserved with its original paths. |
| `docs/phase8/artifacts/first-load-operational-review-20261001/hook-log-handoff-evidence.json` | Runtime initramfs-to-real-root log-delivery evidence. “Handoff” here does not mean an AI session. |

The [dependency inventory](publication-audit/handoff-dependencies.json) records
reference locations without copying source text. No historical snapshot, script,
manifest, audit or hash was rewritten.

The previously ignored Phase 6 implementation plan at
`docs/superpowers/plans/2026-09-22-phase6-access-contract.md` is now eligible for
Git. Its primary purpose is a technical plan and completed-work record, despite
its agent-oriented header. Its contents were not changed. The broad
`docs/superpowers/` ignore rule was replaced by the narrowly scoped private-note
rule.

## Generated files and large artifacts

`__pycache__/`, `*.pyc` and `*.pyo` are now ignored. Forty existing bytecode files
are excluded from the proposed Git tree. They were left on disk; historical
listings were not edited. No evidence or artifact extension was broadly ignored.

The proposed tree has three files over 10 MiB and none over 100 MiB:

| File | Bytes | Why retained |
| --- | ---: | --- |
| `docs/hardware-evidence/MINI12-20261001T030754Z-BOOT-PROVENANCE-READONLY-02/stdout.txt` | 70,717,268 | Raw capture, including framed base64 artifacts. |
| `docs/hardware-evidence/MINI12-20261001T030754Z-BOOT-PROVENANCE-READONLY-02/decoded-files/release_initrd` | 50,863,580 | Captured stock initramfs used for boot and first-load analysis. |
| `docs/phase8/artifacts/experimental-first-load-01-20261001/build-03/initrd.img-sgx535-firstload-01` | 50,804,481 | Qualified experimental image, not installed or executed by this audit. |

The sixteen recorded kernel modules and the compiler-command records are
intentional evidence, not stray build output. The existing `references/` ignore
rule still excludes the local third-party checkouts.

## Credential and privacy review

No actual password, access token, private key, Wi-Fi PSK, bearer credential or
credential-bearing URL was found by this best-effort inspection. This is not a
guarantee that none exists.

The initial scan inspected all 2,812 eligible working-tree files, including the
forty subsequently excluded bytecode files. It also decoded 66 framed base64
payloads, both initramfs images and their 4,279 members. The local tar archive,
PDF text/metadata, newly public technical plan and cleanup files were inspected
separately. No archive decoding failure was reported. Binary contents were
searched for recognizable strings; encrypted, opaque or concealed data cannot
be ruled out. Ignored reference checkouts and Git history were outside the
proposed working-tree commit scan. No scanner was installed, and nothing was
uploaded to a third-party service.

Nine credential-pattern leads were reviewed: one figurative README label and
eight matches in the unchanged stock cryptsetup/watchdog utilities. The binary
matches crossed string terminators or contained a formatting placeholder. They
are not captured authentication values. Attempt 01's authentication spill already
uses an explicit redaction marker; it was left unchanged. Curated build
environment records contain tool/build settings, not a full inherited environment.
See the [redacted lead review](publication-audit/credential-lead-review.json).

Privacy review is still required. Raw captures include:

- Private/link-local addresses and SSH endpoint/account information.
- WLAN MAC addresses in stock kernel logs and repeated capture records.
- Local home paths, host/user labels and device-identifier leads. USB string
  indices, PCI addresses and kernel configuration symbols are not unique serial
  values; remaining identifier leads need human review.
- A collaboration contact intentionally listed in the README. Upstream author
  and licence contacts also occur in source and utility metadata. These were not
  treated as leaked authentication data or removed automatically.

Network addresses, MACs and unrelated device identifiers are not generally
needed to explain SGX535 behavior. Some are embedded in hash-bound evidence and
command provenance. Do not edit those originals to hide them. The owner must
approve disclosure or authorize a separate redacted public evidence set that
preserves the private originals and clearly identifies any new hashes.
No precise street address or physical location was identified by this inspection;
that is not a forensic absence claim.

The [privacy review](publication-audit/privacy-review.json) gives file/line
locations with values redacted. No sensitive values are repeated in this report.

## References and proposed Git tree

No handoff was moved, so this cleanup introduces no handoff-path breakage.
The internal policy links resolve. The existing documentation has 377 link
findings: 252 links into excluded local reference checkouts and 125 missing
relative targets in historical evidence/snapshot documents. There are no missing
local targets in maintained documentation. Historical snapshot links were not
rewritten to fit their archive location. See the
[existing link findings](publication-audit/existing-link-findings.json).
These limitations must not be reported as a clean public GitHub link check.

A temporary Git index was used to evaluate `git add -A`. The real index was not
changed. The proposed tree contains 2,779 files: research and hardware evidence,
Phase 7/8 reports and audits, kernel/tool sources and tests, existing experiments,
the technical plan and these publication records. The proposed path set contains
no Python bytecode, private AI-note files or `references/` checkout files.

Preexisting changes outside the Phase 7/8 document directories also appear in
that proposed commit: README, the evidence matrix, unknowns, TVZ-001 notes,
tool documentation/target lists and the identification probe sources. They are
research-related but still need the owner's commit-scope review. They were not
removed or rewritten by this audit. No unexplained temporary file or accidental
large build output was identified.

Publication remains BLOCKED — REVIEW REQUIRED for privacy, public-link
limitations and final commit-scope approval. No automated scan authorizes
publication, deployment or SGX execution.

## Verification after cleanup

The offline suite passed 304 tests with zero skips, plus 14 boot-analysis tests
and three UBSan harnesses. The generator check passed. Candidate #1 and the
lifecycle derivative still have complete target-table CRC coverage. Both frozen
dry runs retained SHA-256
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` still refuses with `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.
`git diff --check` passed.

All 2,812 files in the initial audit manifest still exist. Only `.gitignore`
changed among those files; every other initial file has its original SHA-256.
The cleanup adds the public internal-note policy, this report and four redacted
audit inventories. The previously ignored technical plan is unchanged. The
private directory README is ignored. The temporary-index proposal has 25
modified and 2,336 added paths relative to HEAD, mostly preexisting research
work. Nothing is staged in the real index.
