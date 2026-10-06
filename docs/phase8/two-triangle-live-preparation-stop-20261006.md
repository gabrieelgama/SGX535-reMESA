# Two-indexed-triangle preparation STOP — 2026-10-06

The exact square candidate was staged and manually FIRSTLOAD booted on
`83ee4ff8-a7f1-4468-b348-d10627f38d17`. STOCK preparation80/80,
poststage82/82, FIRSTLOAD83/83, protected preparation56/56,
independent final guards204/204 and immediate passive precheck42/42 passed.
PRE07 LIVE PASS applies to the captured state, not indefinitely.

**No client launch, ioctl or SGX invocation occurred. Authorization unconsumed.**
The offline launch adapter rejected two copies of the same witness hash in the
new card: `source_witness` and the historical launch schema's `witness`.
This is a controller/schema integration error, not evidence of GPU failure.
Live execution stopped without retry. Originals are preserved in the
[95-file STOP seal](/home/gama/sgx535-offline/phase8-square-continuation-20261006T084340Z/pre07-stop-seal.json). The preceding18-file
STOCK STOP and53-file staging seals independently verify unchanged.

An offline-only correction makes `source_witness` an explicit alias to the one
unique `witness` object. Full boot/module/image/hash validation remains.
Seven CPU-only checks pass: valid alias, conflicting hash, wrong boot,
unexpected second object, malformed alias, missing witness and stale observer.
This is controller validation, not GPU qualification. No corrected controller
was dispatched. The exact candidate and rendering bytes remain unchanged.

Next action requires a renewed continuation across this STOP, passive continuity
of the same boot and qualified corrected launch binding before any call. Do not
reboot merely for this schema correction. No display publication is authorized.
`MULTI_TRIANGLE_ESTABLISHED` remains unestablished. The immutable FIRE3 triangle
and centered physical-display milestones remain established.

See [machine record](two-triangle-live-preparation-stop-20261006.json).
