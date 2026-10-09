#!/usr/bin/env python3
"""Qualify the clean offline texture contract; never contacts SGX hardware."""
import argparse, json, os, subprocess
from pathlib import Path

p=argparse.ArgumentParser(); p.add_argument('--output',type=Path,required=True)
p.add_argument('--i386-cc',type=Path); p.add_argument('--i386-sysroot',type=Path)
p.add_argument('--qemu-i386',type=Path); a=p.parse_args(); a.output.mkdir(parents=True,exist_ok=True)
root=Path(__file__).resolve().parents[1]; src=[root/'remesa_texture.c',root/'tests/test_texture_contract.c']
variants=[('native',['cc'],['-O2'],[]),('ubsan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'],[])]
if any((a.i386_cc,a.i386_sysroot,a.qemu_i386)):
    if not all((a.i386_cc,a.i386_sysroot,a.qemu_i386)): raise SystemExit('i386 tools incomplete')
    variants.append(('i386',[str(a.i386_cc),'--sysroot='+str(a.i386_sysroot)],['-O2','-static','-march=i486'],[str(a.qemu_i386)]))
results={}
env=dict(os.environ)
if a.i386_sysroot:
    env['LD_LIBRARY_PATH']=str(a.i386_sysroot/'usr/lib/aarch64-linux-gnu')
for name,cc,flags,prefix in variants:
    exe=a.output/(name+'-texture'); cmd=cc+['-std=c11','-Wall','-Wextra','-Werror','-UNDEBUG','-DREMESA_TEXTURE_TEST','-I'+str(root)]+flags+[str(x) for x in src]+['-lm','-o',str(exe)]
    r=subprocess.run(cmd,capture_output=True,text=True,env=env); (a.output/(name+'-compile.log')).write_text(r.stdout+r.stderr)
    if r.returncode: raise SystemExit('compile failed: '+name)
    r=subprocess.run(prefix+[str(exe)],capture_output=True,text=True,env=env); (a.output/(name+'-run.log')).write_text(r.stdout+r.stderr)
    if r.returncode: raise SystemExit('run failed: '+name)
    results[name]={'PASS':True,'command':cmd,'output':r.stdout.strip()}
(a.output/'result.json').write_text(json.dumps({'scope':'offline texture/resource contracts; no SGX execution','variants':results},indent=2)+'\n')
print(json.dumps(results,indent=2))
