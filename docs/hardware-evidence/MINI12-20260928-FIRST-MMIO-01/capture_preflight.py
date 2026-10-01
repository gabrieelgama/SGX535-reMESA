"""Local one-shot capture of first-read identity, state and mechanism availability."""

import datetime
import hashlib
import json
import pathlib
import subprocess


BASE = pathlib.Path(__file__).resolve().parent
RAW = BASE / "raw"
SSH = [
    "ssh", "-T", "-o", "BatchMode=yes", "-o", "ConnectTimeout=8",
    "-o", "StrictHostKeyChecking=yes", "-o", "IdentitiesOnly=yes",
    "-o", "PasswordAuthentication=no", "-o", "KbdInteractiveAuthentication=no",
    "-o", "UserKnownHostsFile=/home/gama/sgx535-gfx/docs/hardware-evidence/MINI12-20260927-H0/ssh_known_hosts",
    "-i", "/home/gama/.ssh/id_ed25519_sgx535_h0", "-p", "22",
    "gama@192.168.18.90", "python3 -",
]


def utc():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    if RAW.exists():
        raise SystemExit("Refusing to overwrite first-read session preflight")
    RAW.mkdir()
    source = (BASE / "preflight.py").read_bytes()
    started = utc()
    try:
        proc = subprocess.run(SSH, input=source, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, timeout=45, check=False)
        status, out, err = proc.returncode, proc.stdout, proc.stderr
    except subprocess.TimeoutExpired as exc:
        status, out, err = "LOCAL_TIMEOUT", exc.stdout or b"", exc.stderr or b""
    ended = utc()
    (RAW / "preflight.stdout").write_bytes(out)
    (RAW / "preflight.stderr").write_bytes(err)
    meta = {
        "command_argv": SSH, "stdin_source": "preflight.py", "stdin_sha256": sha(source),
        "host_started_utc": started, "host_finished_utc": ended,
        "exit_status": status, "stdout_bytes": len(out), "stderr_bytes": len(err),
        "stdout_sha256": sha(out), "stderr_sha256": sha(err),
    }
    (RAW / "preflight.meta.json").write_text(json.dumps(meta, indent=2) + "\n")
    if status != 0:
        raise SystemExit("Read-only preflight did not complete; no MMIO attempt")
    print("Read-only preflight captured; no MMIO attempt")


if __name__ == "__main__":
    main()
