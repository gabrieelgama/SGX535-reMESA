#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Qualify CPU-only framebuffer observation contracts; no device access."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--output', required=True, type=Path)
p.add_argument('--i386-sysroot', required=True, type=Path)
p.add_argument('--qemu-i386', required=True, type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
a.output.mkdir(parents=True, exist_ok=False)
sources = [root/'common/remesa/framebuffer_observation.c',
           root/'common/remesa/tests/test_framebuffer_observation.c']
inputs = sources + [root/'include/remesa/framebuffer_observation.h', Path(__file__).resolve()]
env = dict(os.environ)
env['LD_LIBRARY_PATH'] = str(a.i386_sysroot/'usr/lib/aarch64-linux-gnu')
variants = [('native', ['cc'], ['-O2'], []),
            ('ubsan', ['cc'], ['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'], []),
            ('i386', [str(a.i386_sysroot/'usr/bin/i686-linux-gnu-gcc-14'),
                       '--sysroot='+str(a.i386_sysroot)],
             ['-O2','-static','-march=i486'], [str(a.qemu_i386)])]
results = {}
for name, compiler, flags, emulator in variants:
    binary = a.output/(name+'-observation')
    commands = {'compile': compiler+['-std=c11','-Wall','-Wextra','-Werror','-UNDEBUG',
                '-I'+str(root/'include')]+flags+[str(s) for s in sources]+['-o',str(binary)],
                'run': emulator+[str(binary)]}
    results[name] = {}
    for step, command in commands.items():
        r = subprocess.run(command, env=env, capture_output=True, text=True)
        (a.output/(name+'-'+step+'.log')).write_text(r.stdout+r.stderr)
        results[name][step] = {'command':command,'exit':r.returncode}
        if r.returncode:
            (a.output/'failure.json').write_text(json.dumps(results,indent=2)+'\n')
            raise SystemExit(name+' '+step+' failed')
    results[name]['sha256'] = hashlib.sha256(binary.read_bytes()).hexdigest()
receipt = {'classification':'OFFLINE QUALIFIED','processes':3,'dimension_cases_per_process':11,
           'hardware_interactions':0,'sgx_submissions':0,
           'scope':'Dynamic independent-coverage preservation/discard observation verification, not rendering',
           'limitations':['Caller identities require independent authentication',
                          'No GPU visibility, load/store or completion established',
                          'No hardware dimension capability advertised'],
           'variants':results,'source_sha256':{
               str(f.relative_to(root)):hashlib.sha256(f.read_bytes()).hexdigest() for f in inputs}}
(a.output/'result.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps({'processes':3,'dimension_cases_per_process':11,'hardware_interactions':0}))
