#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check immutable host candidate storage; never creates GPU resources."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument('--cache',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args()
repo=Path(__file__).resolve().parents[3]
env=dict(os.environ)
env.update(json.loads((a.cache/'build-environment.json').read_text()))
a.output.mkdir(parents=True,exist_ok=True)
sources=[repo/'mesa/sgx535/remesa_shader_storage.c',repo/'mesa/sgx535/tests/test_shader_storage.c']
qemu='/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/tests/qemu-root/usr/bin/qemu-i386'
base=['-std=c11','-Wall','-Wextra','-Werror','-Wmissing-prototypes','-Wshadow',
      '-I'+str(repo/'mesa/sgx535'),'-Wl,--wrap=malloc','-Wl,--wrap=calloc','-Wl,--wrap=free']
result={'scope':'Immutable host CPU-record candidate ownership; no GPU allocations or submission',
        'sha256':{str(x.relative_to(repo)):hashlib.sha256(x.read_bytes()).hexdigest() for x in sources+[repo/'mesa/sgx535/remesa_shader_storage.h']}}
for name,cc,flags,runner in [('native',['cc'],['-O2'],[]),
    ('ubsan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'],[]),
    ('i386',[str(a.cache/'wrappers/cc')],['-O2','-march=i486','-static'],[qemu])]:
    exe=a.output/name
    cmd=cc+base+flags+list(map(str,sources))+['-o',str(exe)]
    run=subprocess.run(cmd,env=env,text=True,capture_output=True)
    (a.output/(name+'-compile.log')).write_text(run.stdout+run.stderr)
    result[name]={'compile_command':cmd,'compile_exit':run.returncode}
    if run.returncode:
        (a.output/'result.json').write_text(json.dumps(result,indent=2)+'\n')
        raise SystemExit(name+' compilation failed')
    cmd=runner+[str(exe)]
    run=subprocess.run(cmd,env=env,text=True,capture_output=True)
    (a.output/(name+'-run.log')).write_text(run.stdout+run.stderr)
    result[name].update(run_command=cmd,run_exit=run.returncode,output=run.stdout.strip())
    if run.returncode:
        (a.output/'result.json').write_text(json.dumps(result,indent=2)+'\n')
        raise SystemExit(name+' execution failed')
(a.output/'result.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result,indent=2))
