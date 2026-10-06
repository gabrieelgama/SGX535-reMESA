# Cycle04 STOCK preparation evidence

User authorization covers normal STOCK read-only inspection, timing measurements,
tooling qualification and preparation. It does not authorize a new experimental
boot, SGX, fixed ioctl, hot replacement, target restaging or destructive changes.
Gate B BLOCKED; SGX whitelist `[]`. No new boot, module insertion/removal or target
file/control writes were performed in this task.

`stock-preparation-capture/` contains the one live read-only connection: exact
argv, UTC and host monotonic duration, exit status, raw stdout/stderr, decoded
prefix/root/end records and verdict. All 54 root guards and existing creation
receipts passed. Source/scope pins were saved before contact. Original ownership,
stock files/default, staged bytes/device/inode, services and full kernel-health
receipt are fresh observations, not assumed recovery facts.

`stock-clock-analysis.json` separates automatic capture clocks, process-start
ticks and operator context. Normal HDD boot exceeding 120 seconds is OPERATOR-
REPORTED; process launches cannot establish usable userspace arrival. This new
stock capture cannot supply Cycle03's missing experimental ID/note/trace/photo.
The current stock boot is independently verified; full first-owner/recovery
qualification remains NOT ESTABLISHED.

`source-pins.json` identifies the version actually used for the live STOCK check.
`final-v2-inputs.json` pins the final successor tooling. Stricter clock/identity
receipt checks and the reusable exclusive-attempt controller were added offline
after that connection; actual raw STOCK receipts pass the stricter checks too.
No second target connection was made just to test them. Final code remains
unexecuted in an experimental boot. The module/image were not rebuilt.

`tests-*.stdout/stderr` preserve RED/GREEN actual-wrapper/controller/procedure
runs. `offline-verification-326/` contains the first full 326-test run and 14 boot
analysis tests, three UBSan harnesses, generator, procedure and candidate CRC
checks; after controller additions `suite-final.*` records 328 tests with zero
skips. `offline-verification-final-review/` records the final 329-test run, 14 boot
analysis tests and the other guards after the host/target interval regression.
The test first rejected the unfixed validator; the corrected validator rejects
a target duration longer than its enclosing host interval. Actual STOCK receipts
pass without a tolerance. `image-verification/` independently inspects the
unchanged pinned image.
Both dry runs retain 2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e.
`--complete` remains exit 1: PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE.

The local verification driver's bad cwd substitution is recorded separately;
its failing driver/output were preserved and corrected using a new output path.
It did not run a target operation. Historical Cycle03 files and v1 guards remain
unchanged. Supplied-record PASS is not authenticated live provenance or permission.

See [Cycle04 procedure](../../phase8/cycle04-preparation.md). The prospective
600-second boot watch, 1,200-second kernel-clock capture ceiling, 40-second host
and 35-second child bounds require explicit authorization for the future cycle.
Preparation is complete only to its documented offline/read-only level. No SGX
or experimental boot is authorized. No commit/push.
