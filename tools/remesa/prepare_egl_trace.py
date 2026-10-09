#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Instrument actual Mesa 25.2 EGL in an isolated output directory.

Only logging calls are inserted. Existing predicates, return values, device
selection, and context admission remain unchanged. No cache source or object
is written: exact Meson compile/link commands are replayed with copied source,
private objects, and a private libEGL. This is diagnostic source instrumentation,
not a DRI device simulation or an OpenGL capability override.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess

PREFIX = "REMESA_EGL_SOURCE_TRACE "
HEADER = '''/* reMESA diagnostic overlay: no admission or return changes. */
#include <stdio.h>
#include <stdlib.h>
static void
remesa_egl_source_trace(const char *function, const char *event, int value)
{
   const char *enabled = getenv("REMESA_EGL_SOURCE_TRACE");
   if (enabled && enabled[0] == '1' && enabled[1] == '\\0')
      fprintf(stderr, "REMESA_EGL_SOURCE_TRACE {\\"function\\":\\"%s\\",\\"event\\":\\"%s\\",\\"value\\":%d}\\n", function, event, value);
}
'''
PATCHES = {
    "src/egl/main/egldevice.c": [
        ("   num_devs = drmGetDevices2(0, devices, ARRAY_SIZE(devices));",
         "   num_devs = drmGetDevices2(0, devices, ARRAY_SIZE(devices));\n"
         '   remesa_egl_source_trace("_eglDeviceRefreshList", "drmGetDevices2_return", num_devs);'),
        ("      if (!(devices[i]->available_nodes & (1 << DRM_NODE_RENDER))) {",
         '      remesa_egl_source_trace("_eglDeviceRefreshList", "available_nodes", devices[i]->available_nodes);\n'
         "      if (!(devices[i]->available_nodes & (1 << DRM_NODE_RENDER))) {\n"
         '         remesa_egl_source_trace("_eglDeviceRefreshList", "no_render_node_skipped", 1);'),
    ],
    "src/egl/drivers/dri2/platform_surfaceless.c": [
        ("   const unsigned node_type = swrast ? DRM_NODE_PRIMARY : DRM_NODE_RENDER;",
         "   const unsigned node_type = swrast ? DRM_NODE_PRIMARY : DRM_NODE_RENDER;\n"
         '   remesa_egl_source_trace("surfaceless_probe_device", "required_node_type", node_type);'),
        ("   if (!dev_list)\n      return false;",
         "   if (!dev_list) {\n"
         '      remesa_egl_source_trace("surfaceless_probe_device", "no_usable_drm_device", 0);\n'
         "      return false;\n   }"),
        ("   driver_loaded = surfaceless_probe_device(disp, disp->Options.ForceSoftware,\n                                            disp->Options.Zink);",
         '   remesa_egl_source_trace("dri2_initialize_surfaceless", "force_software", disp->Options.ForceSoftware);\n'
         "   driver_loaded = surfaceless_probe_device(disp, disp->Options.ForceSoftware,\n                                            disp->Options.Zink);\n"
         '   remesa_egl_source_trace("dri2_initialize_surfaceless", "probe_device_return", driver_loaded);'),
        ("      driver_loaded = surfaceless_probe_device_sw(disp);",
         "      driver_loaded = surfaceless_probe_device_sw(disp);\n"
         '      remesa_egl_source_trace("dri2_initialize_surfaceless", "software_probe_return", driver_loaded);'),
        ("cleanup:\n   return _eglError(EGL_NOT_INITIALIZED, err);",
         "cleanup:\n"
         '   remesa_egl_source_trace("dri2_initialize_surfaceless", "EGL_NOT_INITIALIZED_driver_not_loaded", EGL_NOT_INITIALIZED);\n'
         "   return _eglError(EGL_NOT_INITIALIZED, err);"),
    ],
    "src/egl/drivers/dri2/platform_drm.c": [
        ("   if (loader_is_device_render_capable(fd_display_gpu))\n      return fd_display_gpu;",
         "   if (loader_is_device_render_capable(fd_display_gpu)) {\n"
         '      remesa_egl_source_trace("get_fd_render_gpu_drm", "display_fd_has_render_node", fd_display_gpu);\n'
         "      return fd_display_gpu;\n   }\n"
         '   remesa_egl_source_trace("get_fd_render_gpu_drm", "display_only_query_render_device", fd_display_gpu);'),
        ("      if (dri2_dpy->fd_render_gpu < 0) {\n         err = \"DRI2: failed to get compatible render device\";",
         '      remesa_egl_source_trace("dri2_initialize_drm", "render_gpu_fd", dri2_dpy->fd_render_gpu);\n'
         "      if (dri2_dpy->fd_render_gpu < 0) {\n         err = \"DRI2: failed to get compatible render device\";"),
        ("   if (!dri2_setup_device(disp, dri2_dpy->gbm_dri->software)) {",
         '   remesa_egl_source_trace("dri2_initialize_drm", "setup_device_software", dri2_dpy->gbm_dri->software);\n'
         "   if (!dri2_setup_device(disp, dri2_dpy->gbm_dri->software)) {\n"
         '      remesa_egl_source_trace("dri2_initialize_drm", "setup_device_return", 0);'),
    ],
    "src/egl/drivers/dri2/egl_dri2.c": [
        ("   int screen_fd = dri2_dpy->swrast_not_kms ? -1 : dri2_dpy->fd_render_gpu;",
         "   int screen_fd = dri2_dpy->swrast_not_kms ? -1 : dri2_dpy->fd_render_gpu;\n"
         '   remesa_egl_source_trace("dri2_create_screen", "driCreateNewScreen3_fd", screen_fd);'),
        ("   if (dri2_dpy->dri_screen_render_gpu == NULL) {",
         '   remesa_egl_source_trace("dri2_create_screen", "driCreateNewScreen3_nonnull", dri2_dpy->dri_screen_render_gpu != NULL);\n'
         "   if (dri2_dpy->dri_screen_render_gpu == NULL) {"),
        ("   loader_set_logger(_eglLog);\n\n   switch (disp->Platform) {",
         "   loader_set_logger(_eglLog);\n\n"
         '   remesa_egl_source_trace("dri2_initialize", "platform", disp->Platform);\n'
         "   switch (disp->Platform) {"),
        ("   if (!ret) {\n      dri2_display_destroy(disp);",
         '   remesa_egl_source_trace("dri2_initialize", "platform_initialize_return", ret);\n'
         "   if (!ret) {\n      dri2_display_destroy(disp);"),
    ],
}


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def instrument(relative, text):
    if relative not in PATCHES:
        raise ValueError("unsupported source")
    if "remesa_egl_source_trace" in text:
        raise ValueError("source already instrumented")
    for before, after in PATCHES[relative]:
        if text.count(before) != 1:
            raise ValueError(f"{relative}: exact source anchor count must be one: {before!r}")
        text = text.replace(before, after, 1)
    # Header goes after source's existing includes so feature-test declarations
    # and Mesa configuration stay in their original order.
    indices = [index for index, line in enumerate(text.splitlines(keepends=True))
               if line.startswith("#include ")]
    lines = text.splitlines(keepends=True)
    lines.insert(max(indices) + 1, HEADER)
    return "".join(lines)


def trace_records(stderr):
    records = []
    for line in stderr.splitlines():
        if line.startswith(PREFIX):
            record = json.loads(line[len(PREFIX):])
            if set(record) != {"function", "event", "value"} or not isinstance(record["function"], str) or not isinstance(record["event"], str) or type(record["value"]) is not int:
                raise ValueError("malformed source trace")
            records.append(record)
    return records


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mesa-cache", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--frontend", required=True, type=Path)
    parser.add_argument("--probe", required=True, type=Path)
    parser.add_argument("--sandbox", required=True, type=Path)
    parser.add_argument("--qemu-i386", required=True, type=Path)
    args = parser.parse_args()
    cache, out = args.mesa_cache.resolve(), args.output.resolve()
    if out.is_relative_to(cache):
        raise ValueError("trace output must be outside the original Mesa cache")
    if (cache / "mesa-25.2.0/VERSION").read_text().strip() != "25.2.0":
        raise ValueError("only the pinned Mesa 25.2.0 source is supported")
    out.mkdir(parents=True, exist_ok=False)
    build, source_root = cache / "build-i386", cache / "mesa-25.2.0"
    env = os.environ.copy()
    env.update(json.loads((cache / "build-environment.json").read_text()))
    commands, source_metadata, replacements, immutable = [], {}, {}, {}

    def execute(command, name, expected=0, run_env=None):
        result = subprocess.run([str(x) for x in command], cwd=build,
                                env=run_env or env, capture_output=True,
                                text=True, timeout=120)
        commands.append({"name": name, "command": [str(x) for x in command], "cwd": str(build), "exit": result.returncode})
        (out / (name + ".stdout")).write_text(result.stdout)
        (out / (name + ".stderr")).write_text(result.stderr)
        if result.returncode != expected:
            raise RuntimeError(f"{name}: exit {result.returncode}, expected {expected}; see saved stderr")
        return result

    compile_commands = json.loads((build / "compile_commands.json").read_text())
    for relative in PATCHES:
        original = source_root / relative
        immutable[str(original)] = digest(original)
        copied = out / "source" / relative
        copied.parent.mkdir(parents=True, exist_ok=True)
        copied.write_text(instrument(relative, original.read_text()))
        candidates = [item for item in compile_commands
                      if (Path(item["directory"]) / item["file"]).resolve() == original.resolve()]
        if len(candidates) != 1:
            raise RuntimeError("exact EGL compile command unavailable")
        entry = candidates[0]
        obj = out / "objects" / Path(entry["output"]).name
        obj.parent.mkdir(exist_ok=True)
        immutable[str(build / entry["output"])] = digest(build / entry["output"])
        tokens = shlex.split(entry["command"])
        for flag in ("-o", "-MQ", "-MF"):
            index = tokens.index(flag) + 1
            tokens[index] = str(obj) + (".d" if flag == "-MF" else "")
        tokens[tokens.index("-c") + 1] = str(copied)
        # Copied EGL units have local include files (egl_dri2.h etc.) in the
        # original directory; this adds include resolution without code edits.
        tokens += ["-I" + str(original.parent)]
        execute(tokens, "compile-" + original.stem)
        replacements[entry["output"]] = str(obj)
        source_metadata[relative] = {"original_sha256": digest(original), "instrumented_sha256": digest(copied), "logging_anchors": len(PATCHES[relative])}

    queried = execute(["ninja", "-t", "commands", "src/egl/libEGL.so.1.0.0"], "query-link-command")
    tokens = shlex.split(queried.stdout.splitlines()[-1])
    if tokens[tokens.index("-o") + 1] != "src/egl/libEGL.so.1.0.0":
        raise RuntimeError("unexpected actual Meson link target")
    runtime = out / "runtime"
    runtime.mkdir()
    libegl = runtime / "libEGL.so.1.0.0"
    tokens[tokens.index("-o") + 1] = str(libegl)
    frontend_token = "src/gallium/targets/dri/libgallium-25.2.0.so"
    if tokens.count(frontend_token) != 1:
        raise RuntimeError("actual EGL frontend linkage is ambiguous")
    tokens = [replacements.get(token, token) for token in tokens]
    tokens[tokens.index(frontend_token)] = str(args.frontend.resolve())
    original_egl = build / "src/egl/libEGL.so.1.0.0"
    immutable[str(original_egl)] = digest(original_egl)
    execute(tokens, "link-instrumented-libEGL")
    for name, original in (("libgallium-25.2.0.so", args.frontend),
                           ("libgbm.so.1.0.0", build / "src/gbm/libgbm.so.1.0.0")):
        shutil.copyfile(original, runtime / name)
    (runtime / "libgbm.so.1").symlink_to("libgbm.so.1.0.0")
    run_env = env.copy()
    run_env.update({"LD_LIBRARY_PATH": str(runtime) + ":" + str(cache / "runtime/lib"),
                    "LIBGL_DRIVERS_PATH": str(runtime), "EGL_LOG_LEVEL": "debug",
                    "LIBGL_ALWAYS_SOFTWARE": "false", "MESA_LOADER_DRIVER_OVERRIDE": "gma500",
                    "REMESA_EGL_SOURCE_TRACE": "1"})
    command = [args.sandbox, args.qemu_i386, "-L", cache / "runtime", args.probe, libegl]
    traced = execute(command, "actual-egl-traced", expected=1, run_env=run_env)
    if "HOST_IOCTL_SANDBOX=ENFORCED" not in traced.stderr:
        raise RuntimeError("host ioctl-denial launcher did not attest enforcement")
    api_records = [json.loads(line) for line in traced.stdout.splitlines() if line.startswith("{")]
    if api_records != [
            {"stage": "eglGetPlatformDisplay", "success": True, "egl_error": 0x3000},
            {"stage": "eglInitialize", "success": False, "egl_error": 0x3001},
            {"stage": "eglTerminate", "success": True, "egl_error": 0x3000}]:
        raise RuntimeError("unexpected actual EGL API sequence or cleanup result")
    records = trace_records(traced.stderr)
    expected_events = {
        ("dri2_initialize_surfaceless", "force_software", 0),
        ("surfaceless_probe_device", "no_usable_drm_device", 0),
        ("dri2_initialize_surfaceless", "EGL_NOT_INITIALIZED_driver_not_loaded", 0x3001),
        ("dri2_create_screen", "driCreateNewScreen3_fd", -1),
        ("dri2_create_screen", "driCreateNewScreen3_nonnull", 0),
    }
    events = {(r["function"], r["event"], r["value"]) for r in records}
    if not expected_events <= events:
        raise RuntimeError("offline EGL failure attribution changed; inspect recorded events")
    if any(r["event"] == "driCreateNewScreen3_nonnull" and r["value"] for r in records):
        raise RuntimeError("unexpected screen: offline failure qualification must not claim a context")
    run_env["REMESA_EGL_SOURCE_TRACE"] = "0"
    untraced = execute(command, "actual-egl-untraced", expected=1, run_env=run_env)
    if trace_records(untraced.stderr) or traced.stdout != untraced.stdout:
        raise RuntimeError("logging toggle changed API result or leaked trace")
    for path, sha in immutable.items():
        if digest(path) != sha:
            raise RuntimeError("immutable Mesa source/artifact changed: " + path)
    software = None
    hardware_calls = software_calls = 0
    for record in records:
        if record["event"] == "force_software":
            software = bool(record["value"])
        elif record["event"] == "driCreateNewScreen3_fd":
            if software is None:
                raise RuntimeError("DRI screen call lacks initialization attribution")
            if software:
                software_calls += 1
            else:
                hardware_calls += 1
    if hardware_calls != 0 or software_calls != 1:
        raise RuntimeError("unexpected hardware/software screen call counts")
    result = {"status": "PASS", "source_instrumentation": source_metadata,
              "libEGL_sha256": digest(libegl), "frontend_sha256": digest(args.frontend),
              "actual_events": records, "actual_EGL_api_stdout": traced.stdout,
              "executed_test_processes": 2, "hardware_submissions": 0,
              "ioctl_sandbox_enforced": True,
              "client_sha256": digest(args.probe), "host_sandbox_sha256": digest(args.sandbox),
              "hardware_dri_screen_reached": False, "software_retry_screen_null": True,
              "hardware_dri_screen_create_calls": hardware_calls,
              "software_dri_screen_create_calls": software_calls,
              "drm_device_counts": [r["value"] for r in records if r["event"] == "drmGetDevices2_return"],
              "GL_context": "BLOCKED", "cause": "No usable DRM render node in actual surfaceless hardware probe; software retry driCreateNewScreen3(-1) returns NULL. No mock DRM or context.",
              "trace_disabled_same_API_result": True, "original_Mesa_files_unchanged": immutable}
    (out / "commands.json").write_text(json.dumps(commands, indent=2) + "\n")
    (out / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
