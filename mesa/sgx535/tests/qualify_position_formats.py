#!/usr/bin/env python3
"""FLOAT2/3/4 host position fetching against pinned Mesa; zero hardware."""
import argparse, hashlib, json, os, subprocess
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('--cache', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a=p.parse_args()
here=Path(__file__).resolve().parents[1]
mesa=a.cache/'mesa-25.2.0'
pin=json.loads((here/'mesa-api-pin.json').read_text())
for name in ('src/gallium/include/pipe/p_state.h','src/gallium/include/pipe/p_defines.h','src/gallium/include/pipe/p_context.h'):
    if hashlib.sha256((mesa/name).read_bytes()).hexdigest()!=pin['interface_files'][name]:
        raise SystemExit('pinned header mismatch: '+name)
a.output.mkdir(parents=True,exist_ok=False)
base=['-std=c11','-D_POSIX_C_SOURCE=200809L','-DHAVE_ENDIAN_H','-Wall','-Wextra','-Werror',
      '-I'+str(here),'-I'+str(here.parents[1]/'include'),'-I'+str(a.cache/'build-i386/src'),'-I'+str(mesa/'src/gallium/include'),
      '-I'+str(mesa/'src'),'-I'+str(mesa/'include')]
root=Path('/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root')
qemu=Path('/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/tests/qemu-root/usr/bin/qemu-i386')
variants=[('native',['cc'],['-O2'],[]),('ubsan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'],[]),
          ('i386',[str(root/'usr/bin/i686-linux-gnu-gcc-14'),'--sysroot='+str(root)],['-O2','-static','-march=i486'],[str(qemu)])]
env=dict(os.environ)
env['LD_LIBRARY_PATH']=str(root/'usr/lib/aarch64-linux-gnu')
results=[]
for name, cc, flags, run_prefix in variants:
    exe=a.output/name
    command=cc+base+flags+[str(here/'remesa_draw.c'),str(here/'remesa_fixed_state.c'),str(here.parents[1]/'common/remesa/draw_packet.c'),str(here/'tests/test_position_formats.c'),'-o',str(exe)]
    build=subprocess.run(command,capture_output=True,text=True,env=env)
    (a.output/(name+'-build.log')).write_text(build.stdout+build.stderr)
    if build.returncode: raise SystemExit(name+' build failed')
    run=subprocess.run(run_prefix+[str(exe)],capture_output=True,text=True,env=env)
    (a.output/(name+'-run.log')).write_text(run.stdout+run.stderr)
    if run.returncode: raise SystemExit(name+' test failed')
    results.append({'variant':name,'command':command,'run':run_prefix+[str(exe)],'exit':run.returncode,'output':run.stdout.strip(),'sha256':hashlib.sha256(exe.read_bytes()).hexdigest()})
receipt={'status':'PASS','scope':'FLOAT2/3/4 host fetch/defaults and bounds; no GPU shader or SGX execution','processes':len(results),'results':results,'sources':{str(f):hashlib.sha256(f.read_bytes()).hexdigest() for f in [here/'remesa_draw.c',here/'remesa_draw.h',here/'remesa_fixed_state.c',here/'remesa_fixed_state.h',here.parents[1]/'common/remesa/draw_packet.c',here.parents[1]/'include/remesa/draw_packet.h',here/'tests/test_position_formats.c',Path(__file__)]}}
(a.output/'result.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt,indent=2))
