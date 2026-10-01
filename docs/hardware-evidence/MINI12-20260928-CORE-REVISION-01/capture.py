"""Capture bounded target build/install commands without invoking the helper."""

from datetime import datetime, timezone
from pathlib import Path
import hashlib
import json
import subprocess

BASE = Path(__file__).resolve().parent
RAW = BASE / "raw"
RAW.mkdir(exist_ok=True)
SSH_OPTIONS = [
    "-o", "BatchMode=yes", "-o", "ConnectTimeout=8",
    "-o", "StrictHostKeyChecking=yes", "-o", "IdentitiesOnly=yes",
    "-o", "PasswordAuthentication=no", "-o", "KbdInteractiveAuthentication=no",
    "-o", "UserKnownHostsFile=/home/gama/sgx535-gfx/docs/hardware-evidence/MINI12-20260927-H0/ssh_known_hosts",
    "-i", "/home/gama/.ssh/id_ed25519_sgx535_h0",
]
TARGET = "gama@192.168.18.90"


def utc():
    return datetime.now(timezone.utc).isoformat()


def sha(data):
    return hashlib.sha256(data).hexdigest()


def capture(label, argv, stdin=None, timeout=60):
    paths = [RAW / f"{label}.{suffix}" for suffix in ("stdout", "stderr", "meta.json")]
    if stdin is not None:
        paths.append(RAW / f"{label}.stdin")
    if any(path.exists() for path in paths):
        raise RuntimeError(f"refusing to overwrite {label}")
    started = utc()
    try:
        proc = subprocess.run(argv, input=stdin, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, timeout=timeout, check=False)
        status, stdout, stderr = proc.returncode, proc.stdout, proc.stderr
    except subprocess.TimeoutExpired as exc:
        status, stdout, stderr = "LOCAL_TIMEOUT", exc.stdout or b"", exc.stderr or b""
    finished = utc()
    paths[0].write_bytes(stdout)
    paths[1].write_bytes(stderr)
    if stdin is not None:
        paths[3].write_bytes(stdin)
    metadata = {
        "command_argv": argv, "host_started_utc": started,
        "host_finished_utc": finished, "exit_status": status,
        "stdin_bytes": len(stdin) if stdin is not None else None,
        "stdin_sha256": sha(stdin) if stdin is not None else None,
        "stdout_bytes": len(stdout), "stdout_sha256": sha(stdout),
        "stderr_bytes": len(stderr), "stderr_sha256": sha(stderr),
    }
    paths[2].write_text(json.dumps(metadata, indent=2) + "\n")
    return metadata, stdout, stderr


def ssh(label, command, stdin=None, timeout=60):
    return capture(label, ["ssh", "-T", *SSH_OPTIONS, "-p", "22", TARGET, command],
                   stdin=stdin, timeout=timeout)


def scp(label, source_paths, remote_dir):
    return capture(label, ["scp", *SSH_OPTIONS, "-P", "22", *map(str, source_paths),
                           f"{TARGET}:{remote_dir}/"], timeout=60)
