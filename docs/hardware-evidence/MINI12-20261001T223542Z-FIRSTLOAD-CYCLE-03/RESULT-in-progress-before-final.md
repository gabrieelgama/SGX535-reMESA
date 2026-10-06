# First-load cycle03: experimental deadline STOP

Recovery is still in progress. Gate B remains BLOCKED; SGX whitelist `[]`.
FIRST TRIANGLE: NOT ATTEMPTED.

The fresh read-only STOCK preflight passed all 53 root guards. Both existing
experimental files matched the original creation receipts, including device and
inode. The five stock file records matched the retained recovery records.
No files were staged or overwritten in this cycle.

The operator reported EXPERIMENTAL userspace and a normal display at
approximately 120 seconds after kernel handoff. Local `sudo -v` had succeeded,
but the report did not establish any remaining time for capture. The reviewed
procedure requires capture completion within 120 seconds. No experimental
connection was made. This is NOT RUN, not an SSH or sudo failure.

The operator first said that a menu photo had been captured, then explicitly
corrected that statement: no photo was captured. Both replies are preserved.
Selection-photo evidence is missing. The experimental boot ID, loaded module
note, ordered hook trace, complete kernel log and PCI/DRM/IRQ capture are also
missing. Normal userspace/display are operator-reported observations. They do
not establish first ownership.

There is no experimental retry. The operator is using only the authorized
machine boundary and one manual STOCK selection. Stock capture has not run yet.
No hot removal, replacement, fixed ioctl or SGX submission was performed.

The pinned image and candidates remain unchanged. After the deadline STOP,
307 repository tests, 14 boot-analysis tests, three UBSan harnesses, generator,
procedure and candidate import/CRC guards passed. Both dry runs kept SHA-256
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` still exits 1 with
`PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.

`offline-display-boundary.json` records the current source boundary: the fixed
entry reads the color BO and releases its owner on permitted success; the
client writes a diagnostic file. The private SGX allocation has no retained
KMS framebuffer/GTT handoff. Stock display support in the derivative does not
make the frozen result scanout-ready. No CPU framebuffer substitute is proposed.

Raw evidence and exact operator statements are separate from this summary.
See `preflight/`, `preflight-decision.json`, `experimental-deadline-stop.json`
and `operator-photo-correction-and-recovery-boundary.json`.
