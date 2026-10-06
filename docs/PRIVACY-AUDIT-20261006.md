# Privacy and publication audit — 6 October 2026

The reader-facing documentation has been cleaned up, but **the complete raw
research tree is not cleared for unrestricted publication**. Original captures
contain network and machine identifiers, including in existing Git history.
They were retained to preserve evidence. No history was rewritten or account
credentials rotated; no data was sent to an external scanning service.

## Scope and results

The pass inventoried 2,779 tracked files and 4,587 working-tree files outside Git,
third-party `references/` checkouts and Python caches. It scanned text in the current
tree and every reachable named Git blob, including earlier versions. Filename,
configuration, URL, key/token, authentication, network, account/path and
location-related patterns were reviewed. These numbers are the pre-cleanup scan,
not the number of files this pass changed.

| Category | Result |
| --- | --- |
| Public project identity | README contact and GitHub sponsor identity intentionally retained; upstream author/licence contacts retained |
| Local account/home paths | Found in 904 working-tree text files; many are historical command receipts or build provenance |
| Private IPv4 leads | 302 text files; endpoint details removed from three narrative documents |
| MAC-address leads | 99 text files; includes actual network logs and false positives from non-network hex fields |
| Public network addresses | Route destinations occur in raw captures; they are not proof of the maintainer's public address and are unnecessary to explain rendering |
| IPv6 / SSID / router / serial / machine identifiers | Source/configuration and capture leads reviewed; not every matching name is an actual identifier. Raw boot/filesystem/session identifiers remain where they establish provenance |
| Credentials | No recognizable actual private key, API token, credential-bearing URL or authorization-header value found by this scan |
| Location / phone / street address | No confirmed personal address, phone number or GPS location found; keyword hits include documentation and configuration symbols |
| Photos | Two original JPEGs have camera/timestamp/unique-image metadata; no GPS IFD or camera/lens serial tag found. Metadata-free derivatives provided |

The counts above are **matching-file counts, not counts of confirmed disclosures**.
Dotted software versions can resemble IPs; GPU addresses, registers, artifact
hashes, PCI identifiers, kernel versions and Build IDs were not redacted.

Five boot images were decompressed and their 10,679 CPIO members scanned for
recognizable key/token/header material and sensitive credential filenames. A
35-member build tar was inspected without extraction. No such credential lead
was found. Four derived PNGs had no EXIF/XMP marker. The prior
[publication audit](publication-audit.md) additionally reviewed framed base64,
PDF metadata and utility credential-pattern false positives. The current scan
cannot prove absence of encrypted, obfuscated or unrecognized credentials.

## Changes made

Private endpoint details were replaced with descriptive placeholders in:

- `phase7/psb-dri-re/CODEX-HANDOFF-chronology.md`
- `phase8/first-load-stock-boot-observation.md`
- `phase8/post-attempt03-checkpoint-audit.md`

Their technical conclusions are retained. Raw evidence and hash-bound snapshots
were not redacted. New public guides use repository-relative paths and explain
historical machine bindings rather than repeating personal SSH details.

[Metadata-free camera copies](publication-audit/sanitized-media/manifest.json)
retain the exact JPEG compressed pixel stream while removing metadata segments.
Original untracked photos remain on disk and are specifically ignored to prevent
accidental addition. Their old evidence references/hashes are preserved; a
sanitized copy has a new identity and must not be passed off as the original.

## Git history and remaining publication decision

[History findings](publication-audit/privacy-history-20261006.json) identify 170
paths with private-address or MAC pattern matches, and the most recent commit
that touched each path. The report does not repeat identifier values. These include
actual captures and incidental matches; severity is a **privacy/provenance review**,
not a demonstrated credential compromise. Many are already in commit
`f8565115977bda8a529401a7ac5cba822d1a0d36` or its ancestors.

Current-tree redaction does not remove those earlier blobs. The smallest next step
is to decide which network/account/device details may be disclosed, and create a
separately identified sanitized public evidence bundle for the rest, retaining
originals privately and recording the relationship between both sets of hashes.
Do not broadly ignore or delete successful evidence to make this review disappear.

If removing prior disclosures from hosted history is desired, plan it separately
with the maintainer, including affected forks/tags and provenance references.
This pass makes no destructive rewrite. If a genuine credential is later found,
removal alone will not invalidate it: the owner must arrange revocation/rotation
through the appropriate service. This pass performed no account actions.

No SGX invocation, target connection or physical-display modification occurred.
