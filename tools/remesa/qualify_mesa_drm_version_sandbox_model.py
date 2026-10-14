#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Model the captured production seccomp BPF; never execute real DRM or EGL.
The fake prctl and BPF interpreter are explicitly simulated. This does not
replace native/i386 production kernel-enforcement selftests before deployment.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mesa-cache", required=True, type=Path)
    parser.add_argument("--qemu-i386", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[2]
    source = repo / "tools/remesa/test_mesa_drm_version_sandbox.c"
    production = repo / "tools/remesa/mesa_drm_version_sandbox.c"
    out, cache = args.output.resolve(), args.mesa_cache.resolve()
    out.mkdir(parents=True, exist_ok=False)
    source_identity = digest(source)
    production_identity = digest(production)
    env = os.environ.copy()
    env.update(json.loads((cache / "build-environment.json").read_text()))
    results = {}
    for variant, compiler, extra, runner in (
            ("native", "cc", ["-O2"], []),
            ("ubsan", "cc", ["-O1", "-fsanitize=undefined", "-fno-sanitize-recover=all"], []),
            ("i386", cache / "wrappers/cc", ["-O2"], [args.qemu_i386, "-L", cache / "runtime"])):
        binary = out / variant
        command = [str(compiler), "-std=c11", "-Wall", "-Wextra", "-Werror", "-UNDEBUG",
                   "-I" + str(cache / "i386-deps/usr/include/libdrm"), *extra, str(source), "-o", str(binary)]
        built = subprocess.run(command, env=env, capture_output=True, text=True, timeout=60)
        (out / (variant + "-compile.stdout")).write_text(built.stdout)
        (out / (variant + "-compile.stderr")).write_text(built.stderr)
        if built.returncode:
            raise RuntimeError(variant + " model compilation failed")
        executed = subprocess.run([str(token) for token in runner] + [str(binary)],
                                  env=env, capture_output=True, text=True, timeout=60)
        (out / (variant + ".stdout")).write_text(executed.stdout)
        (out / (variant + ".stderr")).write_text(executed.stderr)
        if executed.returncode or "SIMULATED_PRODUCTION_BPF_MODEL=PASS" not in executed.stdout:
            raise RuntimeError(variant + " model test failed")
        results[variant] = {"status": "PASS", "compile_command": command,
                            "runner": [str(token) for token in runner], "output": executed.stdout.strip(),
                            "sha256": digest(binary)}
    if digest(production) != production_identity or digest(source) != source_identity:
        raise RuntimeError("source changed during model qualification")
    receipt = {"status": "PASS", "scope": __doc__, "test_processes": 3,
               "physical_operations": 0, "real_seccomp_installations": 0,
               "source_sha256": source_identity, "production_sha256": production_identity,
               "variants": results,
               "x32_coverage": "Foreign-architecture rejection on i386/aarch64; same-architecture JSET requires an x86_64 build",
               "mandatory_live_prerequisite": "Production --selftest must enforce the actual filter on the Mini 12 before EGL; unprivileged, inherited descriptors closed"}
    (out / "result.json").write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps(receipt, indent=2))


if __name__ == "__main__":
    main()
