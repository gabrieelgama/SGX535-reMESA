# Cycle06: first ownership established; no SGX execution

The first STOCK preflight stopped because sudo was unavailable. The operator
explicitly authorized one fresh read-only preflight after local authentication.
That capture passed54/54 guards; the failed capture remains unchanged.

The operator authorized reuse of the previously reviewed menu photo/configuration
with fresh unchanged GRUB/default/staged-file checks and a current menu/default
witness. No fresh photo was claimed. EXPERIMENTAL was selected exactly once.
They confirmed normal physical display/userspace, local sudo success and arrival
within the600-second watch.

The single experimental capture passed55/55 guards. Boot ID:
`29e27f75-7c84-4537-9ab8-8138bc3eac1d`, distinct from fresh STOCK
`8c89c9c9-d8c5-48b8-bfad-a4afa0442257`. Automatic capture uptime was
130.27–137.27 seconds. Status0, empty stderr, exact pinned transport/root/wrapper,
sameboot receipt and stock/staged creation receipts passed.

The exact ordered hook trace was retrieved:

```text
SGX535-FIRSTLOAD BEGIN
SGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE
SGX535-FIRSTLOAD PASS: derivative first owner; boot may continue
```

Expected derivative note, Live state, PCI/DRM/framebuffer/VT/IRQ16 ownership,
slimski and Xorg passed. Kernel health contains no selected faults; the exact
bounded loader/ACPI/backlight diagnostics remain visible. Taint12289 is unchanged.
The actual pinned hook checked an unbound device/no prior owner and exact payload
hashes before insertion. Trace plus loaded identity and complete current-boot
capture establishes first ownership under the reviewed operational contract.
FIRST OWNER: PASS. LIVE FIRST LOAD: PASS.

Cycle05 already captured STOCK restoration across distinct boot identities,
with55/55 recovery guards and unchanged original/stock/default/staged identities.
That retained recovery evidence is reused; no extra rehearsal or hot restoration
is required. EXPERIMENTAL remains running under the operator's explicit permission.
No recovery reboot has been performed in Cycle06.

The exact-action predicate review is `gate-b-decision.json`; the proposed SGX
boundary is `proposed-first-sgx-action.json`. Gate B is PASS for readiness of the
existing frozen32x32 off-screen action on this boot and exact derivative.
Whitelist: `[MINI12-SGX535-REV121-FROZEN-32x32-SEQ1]`. All hot-removal/replacement/
restoration paths remain BLOCKED and excluded. Publication/ISP/L12/FT-AUX remain
scoped accepted assumptions, not architectural proof. SGX EXECUTION IS NOT
AUTHORIZED. Separate explicit permission is required before client staging,
DRM open, fixed ioctl or SGX action. One scene, sequence1, one TA/raster maximum,
5*HZ/300,000-sample bounds, HOLD retention and no retry remain unchanged.

The proposed operation captures attributable TA/raster status and4096-byte color
readback. It does not program LCD scanout. Physical-display handoff remains
unimplemented/unqualified; no visible triangle is claimed.

22 post-capture v2 tests passed. The actual raw receipt and supplied-record gate
passed independent provenance review. Source image/entry/derivative/client pins
remain exact. The recent330-test/14-analysis/three-UBSan/CRC/generator/dry checks
are retained in Cycle05; no driver, image or client changed since those checks.
git diff --check passes. No restaging, hot unload/replacement, fixed ioctl,
controlled SGX/PDS/TA/raster operation or triangle occurred. No Git staging,
commit or push. Stop here for the separately authorized SGX boundary.
