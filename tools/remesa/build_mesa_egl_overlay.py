#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build the complete real Mesa 25.2 EGL target with the primary-node overlay.

This replays the existing Meson compile/link model, replacing only the three
qualified EGL source units and the selected Gallium frontend. It never mutates
Mesa's source tree, build objects or installed artifacts and executes no EGL,
DRM, context, or GPU operation. The overlay's Meson feature conditional is
verified before deriving its REMESA_PRIMARY_GMA500 compilation definition.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import struct
import subprocess

from prepare_egl_primary import PIN, render

UNITS = ("src/egl/main/egldevice.c", "src/egl/drivers/dri2/platform_drm.c",
         "src/egl/drivers/dri2/egl_dri2.c")


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def validate_overlay(cache, overlay):
    mesa = cache / "mesa-25.2.0"
    if (mesa / "VERSION").read_text().strip() != "25.2.0":
        raise ValueError("only pinned Mesa 25.2.0 is supported")
    manifest_path = overlay / "egl-primary-integration.json"
    if not manifest_path.exists():
        manifest_path = overlay / "integration.json"
    manifest = json.loads(manifest_path.read_text())
    expected = render(mesa)
    if manifest.get("mesa") != "25.2.0" or manifest.get("originals") != PIN:
        raise ValueError("overlay manifest does not bind the pinned original sources")
    if set(manifest.get("files", {})) != set(expected):
        raise ValueError("overlay source/dependency list differs from generated contract")
    for relative, text in expected.items():
        original = overlay / relative
        if original.read_text() != text or digest(original) != manifest["files"][relative]:
            raise ValueError("overlay source/hash mismatch: " + relative)
    meson = expected["src/egl/meson.build"]
    admission = "if with_gallium_sgx535\n  c_args_for_egl += '-DREMESA_PRIMARY_GMA500'\nendif"
    if meson.count(admission) != 1:
        raise ValueError("overlay Meson definition is not the isolated sgx535 feature")
    options = json.loads((cache / "build-i386/meson-info/intro-buildoptions.json").read_text())
    drivers = [item["value"] for item in options if item["name"] == "gallium-drivers"]
    if drivers != [["sgx535"]]:
        raise ValueError("cached build is not the isolated sgx535 Gallium configuration")
    return manifest, expected


def validate_frontend(path):
    with path.open("rb") as stream:
        header = stream.read(20)
    if len(header) != 20 or header[:6] != b"\x7fELF\x01\x01" or struct.unpack_from("<HH", header, 16) != (3, 3):
        raise ValueError("frontend must be an ELF32 little-endian i386 shared library")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mesa-cache", required=True, type=Path)
    parser.add_argument("--overlay", required=True, type=Path)
    parser.add_argument("--frontend", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    cache, overlay, frontend, out = (value.resolve() for value in
                                     (args.mesa_cache, args.overlay, args.frontend, args.output))
    if out.is_relative_to(cache) or out.is_relative_to(overlay):
        raise ValueError("new output must be outside original Mesa cache and overlay")
    manifest, expected = validate_overlay(cache, overlay)
    validate_frontend(frontend)
    out.mkdir(parents=True, exist_ok=False)
    source, build = cache / "mesa-25.2.0", cache / "build-i386"
    env = os.environ.copy()
    env.update(json.loads((cache / "build-environment.json").read_text()))
    snapshots = out / "qualified-source"
    for relative in expected:
        copied = snapshots / relative
        copied.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(overlay / relative, copied)
    manifest_path = overlay / "egl-primary-integration.json"
    if not manifest_path.exists():
        manifest_path = overlay / "integration.json"
    shutil.copyfile(manifest_path, snapshots / "integration.json")
    immutable = {str(source / rel): digest(source / rel) for rel in PIN}
    immutable.update({str(overlay / rel): digest(overlay / rel) for rel in expected})
    immutable[str(manifest_path)] = digest(manifest_path)
    immutable[str(frontend)] = digest(frontend)
    original_egl = build / "src/egl/libEGL.so.1.0.0"
    immutable[str(original_egl)] = digest(original_egl)
    database = json.loads((build / "compile_commands.json").read_text())
    entries = {}
    for relative in UNITS:
        matching = [entry for entry in database
                    if (Path(entry["directory"]) / entry["file"]).resolve() == (source / relative).resolve()]
        if len(matching) != 1:
            raise ValueError("exact EGL source compilation entry missing: " + relative)
        entries[relative] = matching[0]
        original_object = build / matching[0]["output"]
        immutable[str(original_object)] = digest(original_object)
    commands = []

    def execute(command, name):
        command = [str(token) for token in command]
        result = subprocess.run(command, cwd=build, env=env,
                                capture_output=True, text=True, timeout=120)
        commands.append({"name": name, "command": command,
                         "cwd": str(build), "exit": result.returncode})
        (out / (name + ".stdout")).write_text(result.stdout)
        (out / (name + ".stderr")).write_text(result.stderr)
        if result.returncode:
            raise RuntimeError(name + " failed; original cache remains untouched; inspect saved stderr")
        return result

    queried = execute(["ninja", "-t", "commands", "src/egl/libEGL.so.1.0.0"], "query-original-complete-link")
    link_original = shlex.split(queried.stdout.splitlines()[-1])
    if link_original[link_original.index("-o") + 1] != "src/egl/libEGL.so.1.0.0":
        raise ValueError("unexpected actual EGL target")
    frontend_token = "src/gallium/targets/dri/libgallium-25.2.0.so"
    if link_original.count(frontend_token) != 1:
        raise ValueError("EGL frontend linkage is ambiguous")
    builds, object_metadata = [], {}
    for iteration in (1, 2):
        directory = out / ("build-" + str(iteration))
        directory.mkdir()
        replacements = {}
        for relative, entry in entries.items():
            obj = directory / Path(entry["output"]).name
            tokens = shlex.split(entry["command"])
            if "-DHAVE_SGX535" not in tokens:
                raise ValueError("original compilation does not enable qualified sgx535 feature")
            for flag in ("-o", "-MQ", "-MF"):
                tokens[tokens.index(flag) + 1] = str(obj) + (".d" if flag == "-MF" else "")
            tokens[tokens.index("-c") + 1] = str(snapshots / relative)
            # c_args_for_egl from the verified overlay Meson feature conditional.
            tokens += ["-DREMESA_PRIMARY_GMA500", "-I" + str(snapshots / "src/egl"),
                       "-I" + str((source / relative).parent)]
            execute(tokens, "compile-" + str(iteration) + "-" + Path(relative).stem)
            replacements[entry["output"]] = str(obj)
            object_metadata.setdefault(relative, []).append({"path": str(obj), "sha256": digest(obj)})
        library = directory / "libEGL.so.1.0.0"
        tokens = [replacements.get(token, token) for token in link_original]
        tokens[tokens.index("-o") + 1] = str(library)
        tokens[tokens.index(frontend_token)] = str(frontend)
        execute(tokens, "link-complete-EGL-" + str(iteration))
        builds.append(library)
    if digest(builds[0]) != digest(builds[1]):
        raise RuntimeError("complete EGL builds are not byte-identical")
    for path, sha in immutable.items():
        if digest(path) != sha:
            raise RuntimeError("original input changed during qualification: " + path)
    for relative, pair in object_metadata.items():
        if pair[0]["sha256"] != pair[1]["sha256"]:
            raise RuntimeError("private EGL object builds differ: " + relative)
    result = {"status": "PASS", "mesa_version": "25.2.0", "architecture": "ELF32 i386",
              "scope": "Complete real Mesa EGL shared library linked from existing exact Meson target plus qualified primary-only discovery overlay",
              "standalone_mock": False, "physical_operations": 0, "EGL_execution": "NOT PERFORMED by builder",
              "frontend": {"path": str(frontend), "sha256": digest(frontend)},
              "libEGL": [{"path": str(path), "sha256": digest(path)} for path in builds],
              "byte_identical": True, "overlay_manifest": manifest,
              "compile_definition_from_Meson": "REMESA_PRIMARY_GMA500; only when with_gallium_sgx535",
              "objects": object_metadata, "original_inputs_unchanged": immutable,
              "commands_record": str(out / "commands.json")}
    (out / "commands.json").write_text(json.dumps(commands, indent=2) + "\n")
    (out / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({"status": "PASS", "sha256": digest(builds[0]), "byte_identical": True}))


if __name__ == "__main__":
    main()
