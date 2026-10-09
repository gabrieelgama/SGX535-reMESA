#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Qualify ordered seeded evidence transcript; simulated verifier, no device access."""
import argparse
import hashlib
import json
import os
import re
from pathlib import Path
import subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--output', required=True, type=Path)
p.add_argument('--i386-sysroot', required=True, type=Path)
p.add_argument('--qemu-i386', required=True, type=Path)
p.add_argument('--reference', required=True, type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
a.output.mkdir(parents=True, exist_ok=False)
reference_sha = hashlib.sha256(a.reference.read_bytes()).hexdigest()
if reference_sha != '52b2aeb7316820f955d1b9a46cb55a888efb8f74de916acd7ec7ddff6d99ebb5':
    raise SystemExit('STOP: sealed FIRE3 reference identity mismatch')
sources = [root/'common/remesa/framebuffer_observation.c',
           root/'common/remesa/seeded_observation.c',
           root/'common/remesa/seeded_color.c',
           root/'mesa/sgx535/remesa_render_pass.c',
           root/'common/remesa/seeded_session.c',
           root/'common/remesa/tests/test_seeded_session.c']
inputs = sources + [root/'include/remesa/framebuffer_observation.h',
                    root/'include/remesa/seeded_observation.h',
                    root/'include/remesa/seeded_session.h',
                    root/'include/remesa/seeded_color.h',
                    root/'mesa/sgx535/remesa_render_pass.h', Path(__file__).resolve()]
env = dict(os.environ)
env['LD_LIBRARY_PATH'] = str(a.i386_sysroot/'usr/lib/aarch64-linux-gnu')
variants = [('native', ['cc'], ['-O2'], []),
            ('ubsan', ['cc'], ['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'], []),
            ('i386', [str(a.i386_sysroot/'usr/bin/i686-linux-gnu-gcc-14'),
                       '--sysroot='+str(a.i386_sysroot)],
             ['-O2','-static','-march=i486'], [str(a.qemu_i386)])]
results = {}
case_counts = set()
for name, compiler, flags, emulator in variants:
    binary = a.output/(name+'-session')
    commands = {'compile': compiler+['-std=c11','-Wall','-Wextra','-Werror','-UNDEBUG',
                '-I'+str(root/'include'),'-I'+str(root/'mesa/sgx535')]+flags+[str(s) for s in sources]+['-o',str(binary)],
                'run': emulator+[str(binary),str(a.reference)]}
    results[name] = {}
    for step, command in commands.items():
        r = subprocess.run(command, env=env, capture_output=True, text=True, timeout=60)
        (a.output/(name+'-'+step+'.log')).write_text(r.stdout+r.stderr)
        results[name][step] = {'command':command,'exit':r.returncode}
        if r.returncode:
            (a.output/'failure.json').write_text(json.dumps(results,indent=2)+'\n')
            raise SystemExit(name+' '+step+' failed')
    counts = re.fullmatch(r'(\d+) checks PASS; simulated evidence verifier and CPU byte comparison only\n', r.stdout)
    if not counts:
        raise SystemExit(name+' unexpected test result')
    results[name]['checks'] = int(counts.group(1))
    case_counts.add(int(counts.group(1)))
    results[name]['sha256'] = hashlib.sha256(binary.read_bytes()).hexdigest()
if len(case_counts) != 1:
    raise SystemExit('STOP: cross-architecture check-count discrepancy')
checks = case_counts.pop()
receipt = {'classification':'OFFLINE QUALIFIED','processes':3,'checks_per_process':checks,
           'hardware_interactions':0,'sgx_submissions':0,
           'scope':'Ordered caller-supplied evidence records, custody and exact seeded FIRE3 comparison; no authentication, execution or rendering',
           'reference':{'path':str(a.reference),'sha256':reference_sha},
           'limitations':['Caller identities require independent authentication',
                          'No GPU visibility, load/store or completion established',
                          'No hardware dimension capability advertised',
                          'Lifecycle proof supplied by fixtures is simulated'],
           'variants':results,'source_sha256':{
               str(f.relative_to(root)):hashlib.sha256(f.read_bytes()).hexdigest() for f in inputs}}
(a.output/'result.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps({'processes':3,'checks_per_process':checks,'hardware_interactions':0}))
