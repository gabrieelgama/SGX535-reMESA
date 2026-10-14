#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Pure public MMIO field codec qualification; no device access or shader ISA."""
import argparse
import hashlib
import json
import os
import shlex
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('--cache', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
p.add_argument('--test', default='test_program_registers.c',
               choices=['test_program_registers.c', 'test_program_entry.c'])
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=False)
repo = Path(__file__).resolve().parents[3]
here = repo / 'mesa/sgx535'
env = dict(os.environ)
env.update(json.loads((a.cache / 'build-environment.json').read_text()))
base = ['-std=c11', '-UNDEBUG', '-Wall', '-Wextra', '-Werror', '-I' + str(here)]
sources = [here / 'remesa_program_registers.c', here / 'tests' / a.test]
qemu = Path('/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/tests/qemu-root/usr/bin/qemu-i386')
results = []
variants = [('native', ['cc'], ['-O2'], []),
            ('ubsan', ['cc'], ['-O1', '-fsanitize=undefined', '-fno-sanitize-recover=all'], []),
            ('i386', [str(a.cache / 'wrappers/cc')], ['-O2', '-march=i486', '-static'], [str(qemu)])]
for name, cc, flags, runner in variants:
    exe = a.output / name
    cmd = cc + base + flags + list(map(str, sources)) + ['-o', str(exe)]
    build = subprocess.run(cmd, cwd=repo, env=env, capture_output=True, text=True)
    (a.output / (name + '-compile.log')).write_text(build.stdout + build.stderr)
    if build.returncode:
        raise SystemExit(name + ' compile failed')
    run = subprocess.run(runner + [str(exe)], env=env, capture_output=True, text=True, timeout=60)
    (a.output / (name + '-run.log')).write_text(run.stdout + run.stderr)
    if run.returncode:
        raise SystemExit(name + ' test failed')
    results.append({'variant': name, 'build_command': cmd, 'run_command': runner + [str(exe)],
                    'exit': run.returncode, 'stdout': run.stdout.strip(),
                    'binary_sha256': hashlib.sha256(exe.read_bytes()).hexdigest()})

dependencies = set()
dependency_commands = []
for i, source in enumerate(sources):
    cmd = ['cc'] + base + ['-M', str(source)]
    result = subprocess.run(cmd, cwd=repo, env=env, capture_output=True, text=True)
    (a.output / ('dependencies-' + str(i) + '.txt')).write_text(result.stdout + result.stderr)
    if result.returncode:
        raise SystemExit('dependency inventory failed')
    dependency_commands.append(cmd)
    for name in shlex.split(result.stdout.replace('\\\n', ' ').split(':', 1)[1]):
        path = Path(name)
        if not path.is_absolute():
            path = repo / path
        dependencies.add(str(path.resolve()))
inputs = sources + [here / 'remesa_program_registers.h', Path(__file__).resolve(),
                    a.cache / 'build-environment.json']
(a.output / 'access.json').write_text(json.dumps({
    'scope': 'Explicit reads and compiler-reported dependencies, not a syscall audit',
    'files': sorted(set(map(str, inputs)) | dependencies),
    'dependency_commands': dependency_commands,
    'prohibited_shader_material_accessed': False,
    'physical_hardware_contact': False}, indent=2) + '\n')
(a.output / 'result.json').write_text(json.dumps({
    'status': 'PASS', 'classification': 'OFFLINE QUALIFIED', 'processes': len(results),
    'scope': 'Pure public register-field packing. No ISA correctness or SGX535 rev121 physical qualification.',
    'sgx_submissions': 0, 'tests': results,
    'sources': {str(f): hashlib.sha256(f.read_bytes()).hexdigest() for f in inputs}}, indent=2) + '\n')
print(json.dumps(results, indent=2))
