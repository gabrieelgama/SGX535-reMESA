"""One pinned, unprivileged read-only SSH capture; no remote files or secrets."""
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]
PREVIOUS = REPO / 'docs/hardware-evidence/MINI12-20260930-TRIANGLE-PREFLIGHT/capture.json'


def main():
    if (HERE / 'capture.json').exists():
        raise SystemExit('existing capture must not be overwritten')
    argv = json.loads(PREVIOUS.read_text())['command_argv']
    if argv[-1] != 'sh -s':
        raise SystemExit('unexpected retained SSH transport')
    argv[-1] = "sudo -n -p '' sh -s"
    script = (HERE / 'capture.sh').read_bytes()
    subprocess.run(['sh', '-n'], input=script, check=True)
    started = datetime.now(timezone.utc).isoformat()
    try:
        result = subprocess.run(argv, input=script, capture_output=True, timeout=90)
        out, err, code = result.stdout, result.stderr, result.returncode
    except subprocess.TimeoutExpired as exc:
        out, err, code = exc.stdout or b'', exc.stderr or b'', 124
    ended = datetime.now(timezone.utc).isoformat()
    (HERE / 'stdout.txt').write_bytes(out)
    (HERE / 'stderr.txt').write_bytes(err)
    record = {
        'command_argv': argv,
        'local_start_utc': started,
        'local_end_utc': ended,
        'exit_status': code,
        'script_sha256': hashlib.sha256(script).hexdigest(),
        'stdout_sha256': hashlib.sha256(out).hexdigest(),
        'stderr_sha256': hashlib.sha256(err).hexdigest(),
        'stdout_bytes': len(out), 'stderr_bytes': len(err),
        'scope': 'read-only post-reset state and installed kernel build inputs; no DRM open/MMIO/ioctl/state changes',
    }
    (HERE / 'capture.json').write_text(json.dumps(record, indent=2) + '\n')
    print(json.dumps(record, indent=2))
    sys.stderr.buffer.write(err)
    raise SystemExit(code or (1 if err else 0))


if __name__ == '__main__':
    main()
