# Privileged read-only capture — authentication STOP

The approved pinned SSH connection succeeded, but `sudo -n -p '' sh -s`
returned `sudo: a password is required`. No root shell/script started;
stdout is empty. No credential was supplied or recorded, and no alternative
privilege method or automatic authentication retry was used. The operator
was asked to authenticate locally with sudo -v before continuing this same
read-only scope. No target file/build input, display/module/DRM/MMIO/SGX state
was changed or consumed. Gate B remains BLOCKED; whitelist [].

## Exact capture

UTC 2026-09-30T19:05:11.110492+00:00 to 2026-09-30T19:05:12.127269+00:00; exit 1.

- `capture.sh` SHA-256: `0ac523679107da2bf3aebc7e1dc14b18431a10bd0f2fe6ed8dde70be184b1fb0`.
- `stdout.txt` SHA-256: `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.
- `stderr.txt` SHA-256: `9b3a3b7a78f239c4473bb193476b1a860c91f1c2b4fab9c7639eceea414a60fb`.

Exact argv/times/hashes are in capture.json; SHA256SUMS covers all artifacts.
No deployment, fixed ioctl, TA/raster work or Attempt 04 occurred.
