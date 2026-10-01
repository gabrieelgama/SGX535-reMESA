# Narrow service diagnostic — permission refusal

Same boot/release verified. Public process metadata establishes runit PID 1,
runsv slimski PID 1380 -> slimski PID 1398 -> Xorg PID 1793. The reviewed
service symlink still resolves to /etc/sv/slimski. Its supervise directory is
root-owned mode 0700; an unprivileged absence check cannot establish whether
status exists inside it. `sv status` returned 1 with its exact captured message:
`unable to open supervise/ok: access denied`. The script propagated STOP after
preserving metadata. This is a read-permission failure, not evidence that
slimski stopped. No build inputs were read and no state was changed.

## Exact capture

UTC 2026-09-30T19:02:53.419629+00:00 to 2026-09-30T19:02:54.027735+00:00; exit 1.

- `capture.sh` SHA-256: `2b6dae735249159a1d2cf9320d8c567acd7dcc168032e8d556f5b27ed709a3bd`.
- `stdout.txt` SHA-256: `f4e2ca21ddfc2bd73da7d2247e4f7a6a03f29e2f6416240c2751ebb3c054026e`.
- `stderr.txt` SHA-256: `b4a7722870495fa9d665ceb928b4e7ffad166f6d651f13f27536940941a36821`.

Exact argv/times/hashes are in capture.json; SHA256SUMS covers all artifacts.
No deployment, fixed ioctl, TA/raster work or Attempt 04 occurred.
