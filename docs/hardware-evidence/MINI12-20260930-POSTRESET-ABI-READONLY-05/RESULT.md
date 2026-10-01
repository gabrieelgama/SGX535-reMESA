# Evidence-corrected capture — service-status read STOP

The corrected exact DMI and P/O/E guards passed. The original module is Live,
refcount 2, with retained Build ID d8dcb4d38b774ad64799d5e13aaedede069371f3
and installed SHA-256 7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb.
PCI 8086:8108 is bound to gma500; card0 has the expected PCI device and node
metadata; gma500drmfb is 1280×800/32bpp/8192 stride; vtcon0=0/vtcon1=1.

The unprivileged `sv status` assignment returned nonzero and `set -e` stopped
before its captured status string could be printed. Build collection was
not entered. Capture 06 obtained that exact missing status message and
confirmed permission denial, rather than inferring a service-state change.
No service/module/PCI/VT/sysfs/MMIO/DRM/ioctl/SGX operation occurred.

The four new fixture tests first demonstrated rejection of the retained
identity with the old guards and then passed with the two exact corrections.
Full scoped suite: 191 tests PASS, including existing C harness/UBSan tests.

## Exact capture

UTC 2026-09-30T18:58:37.963835+00:00 to 2026-09-30T18:58:38.688531+00:00; exit 1.

- `capture.sh` SHA-256: `e958c5d0498dde533ba0ee22f4d668b780d62da63775fe389ff9db17210018a0`.
- `stdout.txt` SHA-256: `a21b8be51dcc078fa6074509eccde96d3a5092020e05cb09effab96d96881187`.
- `stderr.txt` SHA-256: `778dac97c0478ca77bb340196e7b8f61fcc0c1c3881028638d5fb80edb25cf3b`.

Exact argv/times/hashes are in capture.json; SHA256SUMS covers all artifacts.
No deployment, fixed ioctl, TA/raster work or Attempt 04 occurred.
