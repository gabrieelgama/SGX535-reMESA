# Evidence index

Read [RESULT.md](RESULT.md) and the [offline analysis](../../phase8/first-load-stock-boot-observation.md).
Raw target capture is immutable. `offline-analysis/` contains derived local
records, not target execution. `decoded-files/` reproduces ordinary captured
files with hash/size verification. `artifacts/` is an empty directory left by
the rejected local decoder attempt; it contains no target artifacts.

Local decoder initially rejected a valid `20_memtest86+` label; its failing
implementation/test output is preserved. The corrected parser permits `+`,
rejects traversal/duplicates, and records hash/size validity explicitly.
Changing file payloads are never silently qualified; all 55 actual files match.
A separate offline newc/gzip parser is checked against GNU cpio listings and
negative controls. These helpers do not execute captured code or construct an
experimental image. Absolute paths in derived logs record the analysis work
location; preserved inputs are available in this evidence directory.

An independent read-only reviewer verified transport/capture/file integrity and
key GRUB facts before its usage limit. Final conclusions were checked against
the raw records locally; no further target queries followed capture 02.
