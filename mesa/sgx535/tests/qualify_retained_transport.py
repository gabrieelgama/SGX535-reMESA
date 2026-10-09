#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Interposed retained ioctl transport, no live node or hardware operation."""
import argparse,hashlib,json,os,subprocess,shutil
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True)
p.add_argument('--i386-cc',type=Path);p.add_argument('--i386-sysroot',type=Path);p.add_argument('--qemu-i386',type=Path)
a=p.parse_args();here=Path(__file__).resolve().parents[1];root=here.parents[1];a.output.mkdir(parents=True,exist_ok=True)
inc=a.output/'headers/drm';inc.mkdir(parents=True,exist_ok=True)
for h in ['drm.h','drm_mode.h']:shutil.copyfile('/usr/include/drm/'+h,inc/h)
variants=[('native',['cc'],['-O2'],[]),('ubsan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'],[])]
if any([a.i386_cc,a.i386_sysroot,a.qemu_i386]):
 if not all([a.i386_cc,a.i386_sysroot,a.qemu_i386]):raise SystemExit('Incomplete i386 configuration')
 variants.append(('i386',[str(a.i386_cc),'--sysroot='+str(a.i386_sysroot)],['-O2','-static','-march=i486'],[str(a.qemu_i386)]))
env=dict(os.environ)
if a.i386_sysroot:env['LD_LIBRARY_PATH']=str(a.i386_sysroot/'usr/lib/aarch64-linux-gnu')
result={'scope':'Interposed ioctl responses ONLY; no physical GPU completion','hardware_interactions':0,'variants':{}}
for name,cc,flags,runner in variants:
 exe=a.output/name;cmd=cc+['-std=c11','-Wall','-Wextra','-Werror','-I'+str(root/'include'),'-I'+str(here),'-I'+str(a.output/'headers')]+flags+[str(here/'remesa_retained_transport.c'),str(here/'tests/test_retained_transport.c'),'-Wl,--wrap=ioctl','-o',str(exe)]
 r=subprocess.run(cmd,capture_output=True,text=True,env=env);(a.output/(name+'-compile.log')).write_text(r.stdout+r.stderr)
 if r.returncode:raise SystemExit('Compile failed '+name)
 r=subprocess.run(runner+[str(exe)],capture_output=True,text=True,env=env);(a.output/(name+'-run.log')).write_text(r.stdout+r.stderr)
 if r.returncode:raise SystemExit('Test failed '+name)
 result['variants'][name]={'PASS':True,'processes':1,'command':cmd,'output':r.stdout}
inputs=[here/'remesa_retained_transport.c',here/'remesa_retained_transport.h',here/'tests/test_retained_transport.c',Path(__file__),root/'include/remesa/retained_uapi.h']
result['source_sha256']={str(x.relative_to(root)):hashlib.sha256(x.read_bytes()).hexdigest() for x in inputs}
result['public_linux_drm_headers']={h:hashlib.sha256((inc/h).read_bytes()).hexdigest() for h in ['drm.h','drm_mode.h']}
(a.output/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(result['variants'])
