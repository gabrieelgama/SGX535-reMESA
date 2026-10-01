# Kernel-health guard qualification — 2026-10-01

The immutable first-load cycle01 failed preflight remains unchanged. Its exact
stock log triggers the old case-insensitive substring BUG: in BL bug:; ACPI
PowerButton warnings also trigger the old WARNING: rule. No historical PASS is
invented. Historical capture tests use preserved-before code, not the corrected
helper. Red run:12 tests,10 failures/4 errors. Corrected regression suite:12 PASS;
full repository:278 tests,zero skips;14 boot-analysis;17 procedure tests included;
three UBSan harnesses,generator,both candidate target CRC checks PASS. Dry hash
and expected --complete architectural refusal unchanged (see checks/).

The actual reusable guard retains fault detection and adds missing lockup,
WARN_ON,general-protection and graphics-fault forms. BUG: requires a word/report
boundary; DEBUG: is not BUG:. The exact source-generated BL diagnostic is parsed
into register fields; only observed zero/zero once is the bounded stock form.
Changed fields,extra text/repetition reject. Four typed ACPI PowerButton forms
have observed maximum counts2/2/1/1. Their identities/counts remain visible in a
receipt. Unknown ACPI warnings/errors,repetitions,real BUG/oops/panic/call-trace,
lockup or new graphics faults reject even when added to the retained baseline.
No arbitrary caller-supplied log becomes an accepted fault whitelist. This does
not establish that the PowerButton/backlight diagnostics are generally safe.
It reconciles the reviewed known-good operational baseline with its health guard.

Only numeric dmesg timestamps/loglevel/kernel transport prefixes are stripped;
message fields,case and content remain significant for bounded diagnostics.
Driver,boot,staging and first-owner bounds are unchanged. No image/candidate
was rebuilt. Gate B BLOCKED;SGX whitelist[];no SGX authorization. The renewed
operator instruction permits a fresh complete non-SGX first-load preflight after
this offline qualification; it does not permit triangle execution.
