#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Strict primary-only identity fixtures and pinned real EGL overlay checks.
No physical DRM access, screen/context admission, GPU or presentation claims.
"""
import argparse,hashlib,json,os,subprocess,sys,tempfile,shutil
from pathlib import Path
from prepare_egl_primary import prepare,PIN
p=argparse.ArgumentParser(description=__doc__)
for n in ['mesa','output','drm-include']:p.add_argument('--'+n,type=Path,required=True)
p.add_argument('--i386-cc',type=Path);p.add_argument('--i386-sysroot',type=Path);p.add_argument('--qemu-i386',type=Path)
a=p.parse_args();a.output.mkdir(parents=True,exist_ok=False)
manifest=prepare(a.mesa,a.output/'overlay');cases={}
try:prepare(a.mesa,a.output/'overlay')
except ValueError:cases['existing_output_rejected']=True
else:raise AssertionError('Output overwrite')
with tempfile.TemporaryDirectory(prefix='remesa-egl-primary-') as t:
 base=Path(t)
 for rel in PIN:
  dest=base/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(a.mesa/rel,dest)
 (base/next(iter(PIN))).write_text('corrupt')
 try:prepare(base,base/'bad-overlay')
 except ValueError:cases['corrupt_pinned_source_rejected']=True
 else:raise AssertionError('Corrupt source accepted')
 assert not (base/'bad-overlay').exists()
 cases['failure_creates_no_partial_overlay']=True
assert all(hashlib.sha256((a.mesa/r).read_bytes()).hexdigest()==h for r,h in PIN.items());cases['originals_unchanged']=True
ed=(a.output/'overlay/src/egl/main/egldevice.c').read_text();assert 'dev->EXT_device_drm_render_node = has_render_node;' in ed;assert ': "EGL_EXT_device_drm";' in ed;cases['no_fake_rendernode_extension']=True
header=a.output/'overlay/src/egl/drivers/dri2'
root=Path(__file__).resolve().parents[2]
common=['-std=c11','-D_POSIX_C_SOURCE=200809L','-D_DEFAULT_SOURCE','-Wall','-Wextra','-Werror','-I'+str(a.drm_include),'-I'+str(a.drm_include/'libdrm'),'-I'+str(header),str(root/'mesa/sgx535/tests/test_egl_primary.c')]
variants=[('native',['cc'],['-O2'],[]),('ubsan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'],[])]
if any([a.i386_cc,a.i386_sysroot,a.qemu_i386]):
 if not all([a.i386_cc,a.i386_sysroot,a.qemu_i386]):raise SystemExit('Incomplete i386 triplet')
 variants.append(('i386',[str(a.i386_cc),'--sysroot='+str(a.i386_sysroot)],['-O2','-static','-march=i486'],[str(a.qemu_i386)]))
env=dict(os.environ)
if a.i386_sysroot:env['LD_LIBRARY_PATH']=str(a.i386_sysroot/'usr/lib/aarch64-linux-gnu')
results={}
for name,cc,flags,runner in variants:
 exe=a.output/name;cmd=cc+flags+common+['-o',str(exe)]
 r=subprocess.run(cmd,capture_output=True,text=True,env=env);(a.output/(name+'-compile.log')).write_text(r.stdout+r.stderr)
 if r.returncode:raise SystemExit('Compile failed: '+name)
 r=subprocess.run(runner+[str(exe)],capture_output=True,text=True,env=env);(a.output/(name+'-run.log')).write_text(r.stdout+r.stderr)
 if r.returncode:raise SystemExit('Test failed: '+name)
 results[name]={'PASS':True,'command':cmd,'runner':runner,'output':r.stdout.strip(),'sha256':hashlib.sha256(exe.read_bytes()).hexdigest()}
result={'scope':__doc__,'overlay':manifest,'overlay_cases':cases,'variants':results,'physical_submissions':0}
(a.output/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'variants':{k:v['PASS'] for k,v in results.items()},'overlay_cases':len(cases)}))
