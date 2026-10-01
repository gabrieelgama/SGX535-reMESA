"""Stream one reviewed stage over pinned SSH with separate sudo authentication.

The credential is read from the local terminal and is never saved. The script
is sent only in the second SSH process, after authentication has completed.
"""

import getpass
import hashlib
import json
from pathlib import Path
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
        "stage0", "stop", "release_rebind", "restore", "final"
    }:
        raise SystemExit("usage: run_stage.py stage0|stop|release_rebind|restore|final")
    name = sys.argv[1]
    script = (ROOT / f"{name}.sh").read_bytes()
    credential = getpass.getpass("Mini 12 sudo credential: ")
    started = datetime.now(timezone.utc).isoformat()
    result = run_root_script(SSH, script, credential, timeout=90)
    ended = datetime.now(timezone.utc).isoformat()
    del credential
    (ROOT / f"{name}.stdout").write_bytes(result.stdout)
    (ROOT / f"{name}.stderr").write_bytes(result.stderr)
    record = {
        "stage": name,
        "started_utc": started,
        "ended_utc": ended,
        "exit": result.returncode,
        "script_sha256": hashlib.sha256(script).hexdigest(),
        "stdout_sha256": hashlib.sha256(result.stdout).hexdigest(),
        "stderr_sha256": hashlib.sha256(result.stderr).hexdigest(),
        "ssh": SSH,
        "auth_remote_command": "sudo -S -p '' -v",
        "script_remote_command": "sudo -n -p '' sh -s",
    }
    (ROOT / f"{name}.capture.json").write_text(
        json.dumps(record, indent=2) + "\n"
    )
    sys.stdout.buffer.write(result.stdout)
    sys.stderr.buffer.write(result.stderr)
    print(f"STAGE_EXIT={result.returncode}", flush=True)
    raise SystemExit(result.returncode)


if __name__ == "__main__":
    main()
