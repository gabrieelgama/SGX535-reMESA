#!/usr/bin/env python3
"""Frontend pass epochs/lifetime gates only; no device access or GPU output."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
here = Path(__file__).resolve().parents[1]
args.output.mkdir(parents=True, exist_ok=False)
inputs = [here/'remesa_render_pass.c', here/'remesa_render_pass.h',
          here/'tests/test_render_pass.c', Path(__file__)]

def hashes():
    return {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}

before = hashes()
root = Path('/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root')
qemu = Path('/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/tests/qemu-root/usr/bin/qemu-i386')
variants = [
    ('native', ['cc'], ['-O2'], []),
    ('ubsan', ['cc'], ['-O1', '-fsanitize=undefined', '-fno-sanitize-recover=all'], []),
    ('i386', [str(root/'usr/bin/i686-linux-gnu-gcc-14'), '--sysroot='+str(root)],
     ['-O2', '-static', '-march=i486'], [str(qemu)])]
env = dict(os.environ)
env['LD_LIBRARY_PATH'] = str(root/'usr/lib/aarch64-linux-gnu')
base = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-Wmissing-prototypes', '-I'+str(here)]
sources = [str(here/'remesa_render_pass.c'), str(here/'tests/test_render_pass.c')]
results = []
for name, cc, flags, run_prefix in variants:
    exe = args.output/name
    command = cc+base+flags+sources+['-o', str(exe)]
    build = subprocess.run(command, capture_output=True, text=True, env=env)
    (args.output/(name+'-build.log')).write_text(build.stdout+build.stderr)
    if build.returncode:
        raise SystemExit(name+' build failed')
    run = subprocess.run(run_prefix+[str(exe)], capture_output=True, text=True, env=env)
    (args.output/(name+'-run.log')).write_text(run.stdout+run.stderr)
    if run.returncode:
        raise SystemExit(name+' test failed')
    digest = hashlib.sha256(exe.read_bytes()).hexdigest()
    if name == 'i386':
        repeat = args.output/'i386-repeat'
        rebuilt = subprocess.run(cc+base+flags+sources+['-o', str(repeat)],
                                 capture_output=True, text=True, env=env)
        (args.output/'i386-repeat-build.log').write_text(rebuilt.stdout+rebuilt.stderr)
        if rebuilt.returncode or hashlib.sha256(repeat.read_bytes()).hexdigest() != digest:
            raise SystemExit('i386 deterministic rebuild failed')
    results.append({'variant': name, 'command': command,
                    'run': run_prefix+[str(exe)], 'exit': run.returncode,
                    'output': run.stdout.strip(), 'sha256': digest})
if before != hashes():
    raise SystemExit('source changed during qualification')
result = {'status': 'PASS', 'scope': 'Frontend content epochs and lifetime gates; simulated completion, no SGX',
          'processes': len(results), 'deterministic_i386': True,
          'sources': before, 'results': results, 'gpu_submissions': 0}
(args.output/'result.json').write_text(json.dumps(result, indent=2)+'\n')
print(json.dumps(result, indent=2))
