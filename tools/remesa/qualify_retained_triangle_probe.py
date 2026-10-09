#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Interposed full memory plus retained laboratory caller, no live node or hardware operation."""
import argparse,hashlib,json,os,subprocess,shutil
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True)
p.add_argument('--mesa',type=Path,required=True);p.add_argument('--generated',type=Path,required=True)
p.add_argument('--i386-cc',type=Path);p.add_argument('--i386-sysroot',type=Path);p.add_argument('--qemu-i386',type=Path)
a=p.parse_args();here=Path(__file__).resolve().parent;root=here.parents[1];mesa=root/'mesa/sgx535';a.output.mkdir(parents=True,exist_ok=True)
pin=json.loads((mesa/'mesa-api-pin.json').read_text())
for name,h in pin['interface_files'].items():
 if hashlib.sha256((a.mesa/name).read_bytes()).hexdigest()!=h:raise SystemExit('Pinned Mesa interface mismatch: '+name)
if hashlib.sha256((a.generated/'util/format/u_format_gen.h').read_bytes()).hexdigest()!=pin['generated_formats_sha256']:raise SystemExit('Pinned Mesa generated formats mismatch')
gallium_flags=['-DREMESA_WITH_MESA_HEADERS','-D_POSIX_C_SOURCE=200809L','-DHAVE_ENDIAN_H','-I'+str(a.generated),'-I'+str(a.mesa/'src/gallium/include'),'-I'+str(a.mesa/'src'),'-I'+str(a.mesa/'include')]
shared=[here/'retained_probe_evidence.c',mesa/'remesa_fixed_state.c',mesa/'remesa_render_pass.c',mesa/'remesa_sampler_state.c','remesa_texture_binding.c',mesa/'remesa_screen.c',mesa/'remesa_draw.c',mesa/'remesa_retained_gallium.c',mesa/'remesa_retained_readback.c',root/'common/remesa/draw_packet.c',root/'common/remesa/retained_evidence.c']
inc=a.output/'headers/drm';inc.mkdir(parents=True,exist_ok=True)
for h in ['drm.h','drm_mode.h']:shutil.copyfile('/usr/include/drm/'+h,inc/h)
variants=[('native',['cc'],['-O2'],[]),('ubsan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'],[])]
if any([a.i386_cc,a.i386_sysroot,a.qemu_i386]):
 if not all([a.i386_cc,a.i386_sysroot,a.qemu_i386]):raise SystemExit('Incomplete i386 configuration')
 variants.append(('i386',[str(a.i386_cc),'--sysroot='+str(a.i386_sysroot)],['-O2','-static','-march=i486'],[str(a.qemu_i386)]))
env=dict(os.environ)
if a.i386_sysroot:env['LD_LIBRARY_PATH']=str(a.i386_sysroot/'usr/lib/aarch64-linux-gnu')
result={'scope':'Real Linuxmemorytransport/caller with interposed ownership and completion ONLY; no physical GPU completion','hardware_interactions':0,'variants':{}}
for name,cc,flags,runner in variants:
 exe=a.output/name;cmd=cc+gallium_flags+['-std=c11','-Wall','-Wextra','-Werror','-I'+str(root/'include'),'-I'+str(here),'-I'+str(mesa),'-I'+str(a.output/'headers')]+flags+[str(mesa/'remesa_retained_transport.c'),str(mesa/'remesa_linux_memory.c'),str(here/'retained_triangle_probe.c')]+[str(p) for p in shared]+[str(here/'tests/test_retained_triangle_probe.c'),'-Wl,--wrap=ioctl','-o',str(exe)]
 r=subprocess.run(cmd,capture_output=True,text=True,env=env);(a.output/(name+'-compile.log')).write_text(r.stdout+r.stderr)
 if r.returncode:raise SystemExit('Compile failed '+name)
 r=subprocess.run(runner+[str(exe)],capture_output=True,text=True,env=env);(a.output/(name+'-run.log')).write_text(r.stdout+r.stderr)
 if r.returncode:raise SystemExit('Test failed '+name)
 result['variants'][name]={'PASS':True,'processes':1,'command':cmd,'output':r.stdout}
 app=a.output/(name+'-application')
 application=cc+gallium_flags+['-std=c11','-D_POSIX_C_SOURCE=200809L','-Wall','-Wextra','-Werror','-I'+str(root/'include'),'-I'+str(here),'-I'+str(mesa),'-I'+str(a.output/'headers')]+flags+[str(mesa/'remesa_retained_transport.c'),str(mesa/'remesa_linux_memory.c'),str(here/'retained_triangle_probe.c')]+[str(p) for p in shared]+[str(here/'retained_triangle_probe_main.c'),'-o',str(app)]
 built=subprocess.run(application,capture_output=True,text=True,env=env);(a.output/(name+'-application-compile.log')).write_text(built.stdout+built.stderr)
 if built.returncode:raise SystemExit('Application compile failed '+name)
 result['variants'][name]['application']={'compiled':True,'executed':False,'command':application,'sha256':hashlib.sha256(app.read_bytes()).hexdigest(),'bytes':app.stat().st_size}

inputs=[mesa/'remesa_retained_transport.c',mesa/'remesa_linux_memory.c',here/'retained_triangle_probe.c',here/'retained_triangle_probe.h',here/'retained_triangle_probe_main.c',here/'tests/test_retained_triangle_probe.c',Path(__file__),root/'include/remesa/retained_uapi.h',root/'include/remesa/memory_uapi.h']
inputs+=shared+[mesa/'remesa_screen.h',mesa/'remesa_retained_gallium.h',mesa/'remesa_retained_readback.h',root/'include/remesa/retained_evidence.h',mesa/'mesa-api-pin.json']
result['mesa_pin']=pin['version']
result['Mesa_interface_hashes']=pin['interface_files']
result['source_sha256']={str(x.relative_to(root)):hashlib.sha256(x.read_bytes()).hexdigest() for x in inputs}
result['public_linux_drm_headers']={h:hashlib.sha256((inc/h).read_bytes()).hexdigest() for h in ['drm.h','drm_mode.h']}
(a.output/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(result['variants'])
