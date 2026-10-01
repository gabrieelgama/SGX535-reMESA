"""Stream one reviewed stage over pinned SSH with separate sudo authentication.

The credential is read from the local terminal and is never saved. The script
is sent only in the second SSH process, after authentication has completed.
"""

import getpass
import hashlib
import json
from pathlib import Path
import subprocess
import sys
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parents[2] / "tools" / "psb-dri-re"))
from mini12_sudo_transport import run_root_script  # noqa: E402

SSH = [
    "ssh", "-T", "-o", "BatchMode=yes", "-o", "ConnectTimeout=8",
    "-o", "ConnectionAttempts=1", "-o", "StrictHostKeyChecking=yes",
    "-o", "IdentitiesOnly=yes", "-o", "PasswordAuthentication=no",
    "-o", "KbdInteractiveAuthentication=no", "-o",
    "UserKnownHostsFile=/home/gama/sgx535-gfx/docs/hardware-evidence/MINI12-20260927-H0/ssh_known_hosts",
    "-i", "/home/gama/.ssh/id_ed25519_sgx535_h0", "-p", "22",
    "gama@192.168.18.90",
]


def main():
    if len(sys.argv) != 2 or sys.argv[1] not in {
        "precheck", "load_original", "console_restore", "service_restore", "final", "failure_status",
    }:
        raise SystemExit("unknown reviewed stage")
    name = sys.argv[1]
    script = (ROOT / f"{name}.sh").read_bytes()
    credential = getpass.getpass("Mini 12 sudo credential: ")
    started = datetime.now(timezone.utc).isoformat()
    try:
        result = run_root_script(SSH, script, credential, timeout=90)
        out, err, code = result.stdout, result.stderr, result.returncode
    except subprocess.TimeoutExpired as exc:
        out, err, code = exc.stdout or b"", exc.stderr or b"", 124
    ended = datetime.now(timezone.utc).isoformat()
    del credential
    (ROOT / f"{name}.stdout").write_bytes(out)
    (ROOT / f"{name}.stderr").write_bytes(err)
    record = {
        "stage": name,
        "started_utc": started,
        "ended_utc": ended,
        "exit": code,
        "script_sha256": hashlib.sha256(script).hexdigest(),
        "stdout_sha256": hashlib.sha256(out).hexdigest(),
        "stderr_sha256": hashlib.sha256(err).hexdigest(),
        "ssh": SSH,
        "auth_remote_command": "sudo -S -p '' -v",
        "script_remote_command": "sudo -n -p '' sh -s",
    }
    (ROOT / f"{name}.capture.json").write_text(
        json.dumps(record, indent=2) + "\n"
    )
    print(f"CAPTURE stdout={len(out)} bytes stderr={len(err)} bytes", flush=True)
    sys.stderr.buffer.write(err)
    print(f"STAGE_EXIT={code}", flush=True)
    raise SystemExit(code)


if __name__ == "__main__":
    main()
