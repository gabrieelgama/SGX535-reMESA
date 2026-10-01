"""Local capture runner. Reverify identity before one bounded passive snapshot."""

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


def run(label, script_name):
    script = (BASE / script_name).read_bytes()
    started = utc()
    try:
        proc = subprocess.run(SSH, input=script, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, timeout=45, check=False)
        status, out, err = proc.returncode, proc.stdout, proc.stderr
    except subprocess.TimeoutExpired as exc:
        status, out, err = "LOCAL_TIMEOUT", exc.stdout or b"", exc.stderr or b""
    ended = utc()
    (RAW / (label + ".stdout")).write_bytes(out)
    (RAW / (label + ".stderr")).write_bytes(err)
    meta = {
        "command_argv": SSH, "stdin_source": script_name, "stdin_sha256": sha(script),
        "host_started_utc": started, "host_finished_utc": ended,
        "exit_status": status, "stdout_bytes": len(out), "stderr_bytes": len(err),
        "stdout_sha256": sha(out), "stderr_sha256": sha(err),
    }
    (RAW / (label + ".meta.json")).write_text(json.dumps(meta, indent=2) + "\n")
    return status, out


def main():
    if RAW.exists():
        raise SystemExit("Refusing to overwrite an existing passive capture")
    RAW.mkdir()
    status, out = run("identity", "identity.py")
    if status != 0:
        raise SystemExit("Identity preflight did not complete; snapshot not run")
    try:
        identity = json.loads(out)
    except (ValueError, UnicodeError):
        raise SystemExit("Identity preflight was not valid JSON; snapshot not run")
    expected = {
        "arch": "i686", "sys_vendor": "Dell Inc.", "product_name": "Inspiron 1210",
        "board_name": "0X605H", "bios_version": "A02",
        "pci_vendor": "0x8086", "pci_device": "0x8108", "pci_revision": "0x06",
        "pci_subsystem_vendor": "0x1028", "pci_subsystem_device": "0x02b1",
    }
    mismatch = {key: {"expected": value, "observed": identity.get(key)}
                for key, value in expected.items() if identity.get(key) != value}
    if "Atom(TM) CPU Z520" not in identity.get("cpu_model", ""):
        mismatch["cpu_model"] = {"expected_substring": "Atom(TM) CPU Z520",
                                  "observed": identity.get("cpu_model")}
    (RAW / "identity-check.json").write_text(json.dumps({"match": not mismatch,
                                                         "mismatch": mismatch}, indent=2) + "\n")
    if mismatch:
        raise SystemExit("Identity mismatch; snapshot not run")
    status, _ = run("snapshot", "snapshot.py")
    if status != 0:
        raise SystemExit("Passive snapshot did not complete")
    print("Identity matched; one passive snapshot captured")


if __name__ == "__main__":
    main()
