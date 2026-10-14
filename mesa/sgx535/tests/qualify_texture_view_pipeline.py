#!/usr/bin/env python3
"""Actual Gallium view callbacks with mocked backing/fences; zero hardware."""
import argparse, hashlib, json, os, subprocess
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('--cache',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args()
here=Path(__file__).resolve().parents[1]
mesa=a.cache/'mesa-25.2.0'
pin=json.loads((here/'mesa-api-pin.json').read_text())
for name,expected in pin['interface_files'].items():
    if hashlib.sha256((mesa/name).read_bytes()).hexdigest()!=expected:
        raise SystemExit('pinned header mismatch: '+name)
a.output.mkdir(parents=True,exist_ok=False)
base=['-std=c11','-D_POSIX_C_SOURCE=200809L','-DREMESA_WITH_MESA_HEADERS',
      '-DHAVE_ENDIAN_H','-Wall','-Wextra','-Werror','-I'+str(here),
      '-I'+str(here.parents[1]/'include'),'-I'+str(a.cache/'build-i386/src'),
      '-I'+str(mesa/'src/gallium/include'),'-isystem',str(mesa/'src/gallium/auxiliary'),
      '-I'+str(mesa/'src'),'-I'+str(mesa/'include')]
sources=[here/name for name in ['remesa_fixed_state.c','remesa_render_pass.c',
    'remesa_sampler_state.c','remesa_texture_binding.c','remesa_texture_descriptor.c','remesa_screen.c',
    'remesa_draw.c','remesa_present.c','tests/test_texture_view_pipeline.c']]
sources.append(here.parents[1]/'common/remesa/draw_packet.c')
root=Path('/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root')
qemu=Path('/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/tests/qemu-root/usr/bin/qemu-i386')
variants=[('native',['cc'],['-O2'],[]),
    ('ubsan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'],[]),
    ('i386',[str(root/'usr/bin/i686-linux-gnu-gcc-14'),'--sysroot='+str(root)],
        ['-O2','-static','-march=i486'],[str(qemu)])]
env=dict(os.environ); env['LD_LIBRARY_PATH']=str(root/'usr/lib/aarch64-linux-gnu')
results=[]
for name,cc,flags,prefix in variants:
    exe=a.output/name; command=cc+base+flags+[str(s) for s in sources]+['-o',str(exe)]
    build=subprocess.run(command,capture_output=True,text=True,env=env)
    (a.output/(name+'-build.log')).write_text(build.stdout+build.stderr)
    if build.returncode: raise SystemExit(name+' build failed')
    run=subprocess.run(prefix+[str(exe)],capture_output=True,text=True,env=env)
    (a.output/(name+'-run.log')).write_text(run.stdout+run.stderr)
    if run.returncode: raise SystemExit(name+' test failed')
    results.append({'variant':name,'command':command,'run':prefix+[str(exe)],
        'exit':run.returncode,'output':run.stdout.strip(),
        'sha256':hashlib.sha256(exe.read_bytes()).hexdigest()})
receipt={'status':'PASS','scope':'Real Gallium texture view callbacks with mocked backing/fences; no texture sampling, GPU draw or OpenGL context','processes':len(results),
    'results':results,'sources':{str(s):hashlib.sha256(s.read_bytes()).hexdigest() for s in sources}}
(a.output/'result.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt,indent=2))
