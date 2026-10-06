# Corrected frozen one-shot: HOLD before raster completion

Authorization covered one `ONE whitelisted frozen ioctl; no retry, hot operation or reset` on corrected EXPERIMENTAL boot `203a5b9b-5a90-4fd3-8001-3c92cdefeddd`. This evidence is separate from Attempt04.

The fresh same-boot read-only preflight passed **25/25** after correcting a local guard that incorrectly expected an experimental token in `/proc/cmdline`. The actual entry uses the pinned stock kernel command line; boot ID, hook trace, module identity, and device ownership identify the corrected experimental boot. The initial failed guard output is preserved in `preflight/stdout.txt`.

The client was exclusively staged at `docs/hardware-evidence/MINI12-20261002T041334Z-SGX-ATTEMPT-05` on the target as `/root/sgx535-frozen-seq1-203a5b9b/frozen-triangle-one-shot-i386`: 771,320 bytes, SHA-256 `758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf`, root-owned mode 0700, single link. The corrected wrapper verified the boot ID, derivative note, PCI/DRM identity, client hash, and health, then invoked the client once.

The DRM ioctl returned successfully enough for the client to receive the UAPI result. The client reported:

```text
errno=-1 outcome=3 phase=11 events=0x00000000 color_observed=0 nonzero=0 fnv1a=0x00000000
row[0]=0
row[1]=0
row[2]=0
row[3]=0
row[4]=0
row[5]=0
row[6]=0
row[7]=0
row[8]=0
row[9]=0
row[10]=0
row[11]=0
row[12]=0
row[13]=0
row[14]=0
row[15]=0
row[16]=0
row[17]=0
row[18]=0
row[19]=0
row[20]=0
row[21]=0
row[22]=0
row[23]=0
row[24]=0
row[25]=0
row[26]=0
row[27]=0
row[28]=0
row[29]=0
row[30]=0
row[31]=0
```

The kernel returned operation status `-1`, outcome `3` (HELD), phase `11` (`SGX535_PHASE_HELD_AFTER_FAILURE`), no observed event bits, and `color_observed=0`. The frozen service sets `unsafe_possible` before device maintenance and sets the session phase to FIRE_POSSIBLE before the TA schedule/fire callbacks. Therefore this phase alone does not establish whether TA_FIRE reached the device. The zero event ledger proves no attributable TA completion was observed. TA submission is **UNKNOWN/POSSIBLE**; TA completion is **NOT OBSERVED**.

Raster submission is **NO** by the retained source path: raster fire is reachable only after the service records TA_FINISHED (the session requires `observed_events == 1` before `begin_raster`). The returned event ledger is zero, and no raster-stage completion is present. Raster completion is **NO/NOT OBSERVED**.

No color readback was captured. `color_observed=0`, the client returned nonzero and did not create `color.bin`; target retrieval also confirms `color.bin` is absent. No triangle is established. The before/after kernel logs were byte-identical, health remained within the reviewed baseline, the module remained Live, and the PCI driver binding remained `gma500`. These observations do not resolve the internal stage that returned `-1`.

The wrapper’s exact raw output, stderr, client stdout/stderr, `before.json`, `result.json`, and staging receipt were preserved and hashed. The read-only retrieval verified target file hashes. No second invocation, reset, reboot, module operation, or further SGX action occurred. HOLD; no retry.

## Result

- Ioctl accepted and response copied back: **YES**.
- TA submitted: **UNKNOWN; possible, not attributable from returned phase**.
- TA completed: **NO completion observed**.
- Raster submitted: **NO**.
- Raster completed: **NO**.
- Color readback changed as expected: **NO readback**.
- Triangle established: **NO**.
- Target remains on corrected EXPERIMENTAL boot; no recovery action was authorized or performed.
- Gate B readiness evidence remains as recorded; this authorized one-shot is spent and does not permit another attempt.
