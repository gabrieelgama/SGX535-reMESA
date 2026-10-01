"""Separate sudo validation from a pinned SSH root-script invocation.

The first remote process receives credential bytes but has no shell script.
The second receives script bytes but no credential, and uses noninteractive
sudo so an expired timestamp fails before starting the shell.
"""

import subprocess


class AuthError(RuntimeError):
    """Authentication failed or had unexpected output; no script was sent."""


def run_root_script(ssh_prefix, script, credential, *, runner=subprocess.run,
                    timeout=20):
    if not ssh_prefix or not isinstance(script, bytes) or not script:
        raise ValueError("pinned SSH prefix and nonempty script bytes required")
    if not credential or "\n" in credential or "\r" in credential:
        raise ValueError("invalid credential format")

    auth = runner([*ssh_prefix, "sudo -S -p '' -v"],
                  input=(credential + "\n").encode(),
                  capture_output=True, timeout=timeout)
    if auth.returncode or auth.stdout or auth.stderr:
        raise AuthError("sudo validation failed or produced unexpected output")

    return runner([*ssh_prefix, "sudo -n -p '' sh -s"], input=script,
                  capture_output=True, timeout=timeout)
