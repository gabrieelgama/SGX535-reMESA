#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build and execute the real EGL probe, with all host ioctls forbidden.

An expected EGL initialization rejection qualifies the probe and identifies
the current environment's first failed call; it does not qualify OpenGL.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--mesa-cache", type=Path, required=True)
    parser.add_argument("--frontend", type=Path, required=True)
    parser.add_argument("--dril", type=Path, required=True)
    parser.add_argument("--qemu-i386", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    repo = Path(__file__).resolve().parents[2]
    cache = args.mesa_cache.resolve()
    source = repo / "tools/remesa/mesa_egl_context_probe.c"
    env = os.environ.copy()
    env.update(json.loads((cache / "build-environment.json").read_text()))
    commands = []

    def execute(command, name, expected=0, run_env=None):
        command = [str(x) for x in command]
        commands.append({"name": name, "command": command})
        result = subprocess.run(command, env=run_env or env,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                text=True, timeout=60)
        (out / (name + ".stdout")).write_text(result.stdout)
        (out / (name + ".stderr")).write_text(result.stderr)
        if result.returncode != expected:
            raise RuntimeError(f"{name}: exit {result.returncode}, expected {expected}")
        return result

    # The native host filter remains effective while QEMU translates guest
    # syscalls. Guest prctl(seccomp) itself is not reliable under user QEMU.
    launcher_source = out / "ioctl_sandbox_exec.c"
    launcher_source.write_text('''/* SPDX-License-Identifier: MIT */
#include "remesa_loader_sandbox.h"
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
int main(int argc,char **argv) {
 if(argc<2)return 2;
 int error=remesa_loader_deny_ioctls();
 if(error){fprintf(stderr,"sandbox setup: %d\\n",error);return 2;}
 errno=0;if(ioctl(-1,0,0)!=-1||errno!=EPERM)return 2;
 fprintf(stderr,"HOST_IOCTL_SANDBOX=ENFORCED\\n");
 execvp(argv[1],argv+1);perror("execvp");return 2;
}
''')
    launcher = out / "ioctl-sandbox-exec"
    execute(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
             "-I" + str(repo / "tools/remesa"), launcher_source,
             "-o", launcher], "host-sandbox-build")
    for variant, flags in (("native", []),
                           ("ubsan", ["-fsanitize=undefined", "-fno-sanitize-recover=all"]),
                           ("i386", [])):
        compiler = cache / "wrappers/cc" if variant == "i386" else "cc"
        binary = out / ("mesa-egl-context-probe-" + variant)
        execute([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                 *flags, "-I" + str(cache / "mesa-25.2.0/include"),
                 source, "-ldl", "-o", binary], variant + "-build")
        prefix = [launcher]
        if variant == "i386":
            prefix += [args.qemu_i386, "-L", cache / "runtime"]
        result = execute([*prefix, binary, out / "missing-libEGL.so"],
                         variant + "-missing-library", expected=1)
        if '"stage":"dlopen_libEGL","success":false' not in result.stdout:
            raise RuntimeError("Missing-library failure was not attributed")
        renderer_result = execute([*prefix, binary, "--test-renderer-admission"],
                                  variant + "-renderer-admission")
        if "12 unsupported identities rejected" not in renderer_result.stdout:
            raise RuntimeError("Renderer rejection cases were not executed")

    duplicate = out / "mesa-egl-context-probe-i386-build-2"
    execute([cache / "wrappers/cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
             "-I" + str(cache / "mesa-25.2.0/include"), source, "-ldl",
             "-o", duplicate], "i386-repro-build")
    if digest(duplicate) != digest(out / "mesa-egl-context-probe-i386"):
        raise RuntimeError("i386 probe builds are not byte-identical")

    runtime = out / "runtime"
    runtime.mkdir()
    libraries = {
        "libEGL.so.1.0.0": cache / "build-i386/src/egl/libEGL.so.1.0.0",
        "libgbm.so.1.0.0": cache / "build-i386/src/gbm/libgbm.so.1.0.0",
        "libgallium-25.2.0.so": args.frontend,
        "gma500_dri.so": args.dril,
    }
    identities = {}
    for name, original in libraries.items():
        target = runtime / name
        shutil.copyfile(original, target)
        identities[name] = {"source": str(original), "sha256": digest(target)}
    (runtime / "libgbm.so.1").symlink_to("libgbm.so.1.0.0")
    run_env = env.copy()
    run_env.update({"LD_LIBRARY_PATH": str(runtime) + ":" + str(cache / "runtime/lib"),
                    "LIBGL_DRIVERS_PATH": str(runtime),
                    "GBM_BACKENDS_PATH": str(cache / "install-i386/lib/gbm"),
                    "EGL_LOG_LEVEL": "debug", "LIBGL_DEBUG": "verbose",
                    "LIBGL_ALWAYS_SOFTWARE": "false",
                    "MESA_LOADER_DRIVER_OVERRIDE": "gma500"})
    result = execute([launcher, args.qemu_i386, "-L", cache / "runtime",
                      out / "mesa-egl-context-probe-i386",
                      runtime / "libEGL.so.1.0.0"],
                     "actual-mesa-egl", expected=1, run_env=run_env)
    records = [json.loads(line) for line in result.stdout.splitlines()
               if line.startswith("{")]
    failures = [record for record in records if not record["success"]]
    if not failures or failures[0]["stage"] != "eglInitialize":
        raise RuntimeError("Unexpected first EGL failure; inspect recorded logs")
    terminated = [record for record in records if record["stage"] == "eglTerminate"]
    if len(terminated) != 1 or not terminated[0]["success"]:
        raise RuntimeError("Failed EGL initialization did not terminate its display exactly once")
    invalid_devices = []
    for name, device in (("missing", out / "missing-card"),
                         ("regular", source), ("foreign", Path("/dev/null"))):
        rejected = execute([launcher, args.qemu_i386, "-L", cache / "runtime",
                            out / "mesa-egl-context-probe-i386",
                            runtime / "libEGL.so.1.0.0", "--drm-device", device,
                            "--libgbm", runtime / "libgbm.so.1.0.0"],
                           "gbm-reject-" + name, expected=1, run_env=run_env)
        if '"stage":"drm_device_identity","success":false' not in rejected.stdout:
            raise RuntimeError("Invalid device did not fail identity admission")
        if '"stage":"gbm_create_device"' in rejected.stdout:
            raise RuntimeError("Invalid descriptor reached GBM")
        invalid_devices.append({"case": name, "PASS": True})
    receipt = {
        "status": "PASS", "scope": "Actual Mesa EGL entry points under i386 QEMU; host ioctl denial, no software context accepted, no GPU work",
        "open_gl_context": "BLOCKED", "first_failed_operation": failures[0],
        "calls_reached": [record["stage"] for record in records],
        "not_reached": ["eglBindAPI", "eglChooseConfig", "eglCreateContext", "eglMakeCurrent"],
        "physical_submissions": 0, "simulated_contexts": 0,
        "mesa_software_retry_attempted": "Falling back to surfaceless swrast" in result.stderr,
        "software_context_accepted": False,
        "executed_test_processes": 10,
        "renderer_admission": "Exact reMESA SGX535 only; 12 unsupported identities rejected in native, UBSan and i386",
        "initialize_failure_cleanup": "Actual eglTerminate succeeded exactly once before unloading libEGL",
        "gbm_device_rejections": invalid_devices,
        "gbm_platform": "Implemented, strict retained DRM/PCI 8086:8108 descriptor validation; genuine qualified device unavailable in offline sandbox",
        "client_source_sha256": digest(source),
        "client_i386_sha256": digest(out / "mesa-egl-context-probe-i386"),
        "i386_reproducible": True,
        "libraries": identities,
        "environment_limit": "All ioctls denied before QEMU; no qualified SGX DRM device exposed. This rejection does not test a physical SGX screen.",
        "driver_contract_limit": "Validated SGX descriptor create_screen currently returns ENOTSUP after diagnostic ownership validation; shader/raster/texture baseline is incomplete.",
    }
    (out / "commands.json").write_text(json.dumps(commands, indent=2) + "\n")
    (out / "result.json").write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps(receipt, indent=2))


if __name__ == "__main__":
    main()
