# Operator checklist — one first-load boot, NO triangle

Documentation only. This checklist authorizes no live action. Follow
[the exact procedure](first-load-staging-boot-recovery-procedure.md) and its [pinned record contract](first-load-operational-procedure.json). Any mismatch means STOP/HOLD. Do
not use another route, manipulate the driver or retry the experiment.

## PRE-STAGE — STOP before writes if any item fails

- [ ] Obtain new authorization for staging, one non-SGX boot and stock recovery.
- [ ] The physically present operator confirms display/input/power controls and
  understands black screen/network loss, volatile logs and power-off data-loss risk.
- [ ] Fresh stock identity, original loaded note/hash, PCI/DRM/fb/IRQ/VT,
  slimski/Xorg/display and boot ID match the reviewed normal baseline.
- [ ] Check that all stock file hashes and the saved normal GRUB ID match; no next_entry.
- [ ] Image 50,804,481 bytes, SHA `4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`.
- [ ] Entry 974 bytes, SHA `181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612`.
- [ ] Destinations/incoming directory absent, including dangling symlinks;
  trusted parents, writable filesystem, /boot free space ≥128 MiB.
- [ ] Existing authenticated/noninteractive transport and normal exclusive
  copy/fsync/read-back facilities usable. No credentials in script input.

## STAGE — no graphics changes; STOP before reboot on failure

- [ ] Create the private incoming copy exclusively; check source hash, size and regular-file status.
- [ ] Create the root:root experimental image exclusively, with mode 0644, at
  `/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01`.
- [ ] Fsync the file and directory; reopen the destination and check exact hash/size.
- [ ] Create root:root `/boot/grub/custom.cfg` exclusively, with mode 0644 and only the exact pinned entry.
- [ ] Fsync the file and directory; reopen and check the exact entry and resolved initrd path.
- [ ] Stock kernel/initrd/module/grub.cfg/grubenv unchanged; original still
  active, same boot/service/binding state; stock entry remains selectable.
- [ ] No update-grub, initramfs rebuild, default change, blacklist or module action.

## PRE-BOOT / BOOT — ONE manual experimental selection

- [ ] Save work, capture staging receipts, confirm authorized operator reset/power boundary.
- [ ] The operator can see both STOCK and EXPERIMENTAL entries and stock
  default at GRUB; capture photo. If not, select/allow stock and STOP, no retry.
- [ ] Select `EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)` exactly once.
- [ ] Start 120-second observation clock at kernel handoff; no entry/argument edits.

## OBSERVE — capture before reset; no extra workload

- [ ] Record the new boot ID/kernel/cmdline/DMI and evidence of experimental selection.
- [ ] Raw hook log: one BEGIN → FILES-VERIFIED → PASS, no HOLD/error.
  Missing post-pivot log means first-owner NOT ESTABLISHED.
- [ ] Loaded derivative Build-ID note, Live state, PCI driver/module and DRM BDF.
- [ ] gma500drmfb, PCI IRQ16 and gma500 handler on IRQ16.
- [ ] Complete current-boot kernel log; no new warning/oops/panic/fault/taint.
- [ ] Physically normal display/KMS and expected userspace/slimski/Xorg.
- [ ] Record the SSH result separately; complete local evidence can prove first load without SSH.
- [ ] Preserve raw evidence/photos before recovery; record capture completion timing
  ≤120 seconds from handoff and linked nonempty boot IDs recorded.
- [ ] Fault, guard failure, trace/identity mismatch, hang or deadline → HOLD.

## STOP — regardless of apparent success

- [ ] NO fixed client, DRM open, ioctl, SGX/PDS/TA/raster or triangle.
- [ ] NO module unload/reload, original insertion, PCI/VT unbind, speculative reset.
- [ ] NO second experimental boot, reconnect loop or submission retry.

## RECOVERY — operator boundary, never hot restoration

- [ ] Preserve reachable evidence; operator performs only authorized reset/power boundary.
- [ ] Manually select normal stock title/ID using untouched stock kernel/initrd.
- [ ] Within 120 seconds: boot ID distinct from PRE-STAGE and experimental,
  stock hashes/cmdline and loaded original note; timing evidence recorded.
- [ ] Original Live; expected PCI/DRM/fb1280×800/IRQ16/VT0=0/VT1=1.
- [ ] slimski/Xorg/userspace, physically normal display and stock WLAN/SSH.
- [ ] Complete new-boot kernel log/taint; saved stock selection, no new fault.
- [ ] Require every check before LIVE RECOVERY PASS. Otherwise HOLD/STOP.
- [ ] Keep experimental files/evidence; optional file-only cleanup requires
  separate authorization after stock recovery, checking exact creation receipts.
