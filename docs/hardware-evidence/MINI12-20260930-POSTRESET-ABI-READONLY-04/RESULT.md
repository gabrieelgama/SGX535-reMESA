# Capture 04 — false identity guard STOP

Pinned SSH succeeded; this was the first successful connection after the
operator's network update. The script stopped at its identity check before
any module/display/build-file reads. Raw output established the expected
5.10.240-antix.1-486-smp i686 kernel (#6, GCC Debian 14.2.0-19, GNU ld 2.44),
boot ID 8ae37532-19d4-4ec7-9c29-a791acfd889f, raw DMI `Inspiron 1210   ` and
taint 12289. Offline comparison proved the script incorrectly omitted the
already retained three DMI spaces and required zero taint despite the retained
P/O/E baseline. This was a checker defect, not contradictory target identity.

The raw capture/script remains unchanged. The exact-value correction was
made only in capture 05 and regression-tested offline (four methods; full
191-test scoped suite PASS). No taint generalization or safety acceptance:
new forced-operation/WARN/OOPS/lockup bits still reject. No target write or
SGX action occurred.

## Exact capture

UTC 2026-09-30T18:50:57.472709+00:00 to 2026-09-30T18:50:58.965046+00:00; exit 1.

- `capture.sh` SHA-256: `b9591dab67bedfab789bd82d1c228da0d76201eeccfe9389728bc8a0c5166003`.
- `stdout.txt` SHA-256: `6ad2f5263342dee8a3842d1af46c731191e32e4999762785e4a06141d8f8f582`.
- `stderr.txt` SHA-256: `4a50f384f35eca3bd35fb85acf55f0a39373bafa4f3f2fa95e5c9c396d9a35fc`.

Exact argv/times/hashes are in capture.json; SHA256SUMS covers all artifacts.
No deployment, fixed ioctl, TA/raster work or Attempt 04 occurred.
