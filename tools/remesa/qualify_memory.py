#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compile real memory-service source with injected kernel APIs; no hardware.
Transport tests interpose ioctl, so no device is opened or ioctl reaches Linux.
"""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--output',type=Path,required=True)
p.add_argument('--mesa',type=Path,required=True);p.add_argument('--generated',type=Path,required=True)
p.add_argument('--skip-probe-build',action='store_true',help='Preserve previously qualified probe binaries; test service/transport only')
p.add_argument('--targets',help='Focused comma-separated kernel,transport,dispatch targets')
p.add_argument('--i386-cc',type=Path);p.add_argument('--i386-sysroot',type=Path);p.add_argument('--qemu-i386',type=Path)
a=p.parse_args();root=Path(__file__).resolve().parents[2];a.output.mkdir(parents=True,exist_ok=True)
pin=json.loads((root/'mesa/sgx535/mesa-api-pin.json').read_text())
for f,h in pin['interface_files'].items():
 if hashlib.sha256((a.mesa/f).read_bytes()).hexdigest()!=h:raise SystemExit('Mesa pin mismatch: '+f)
if hashlib.sha256((a.generated/'util/format/u_format_gen.h').read_bytes()).hexdigest()!=pin['generated_formats_sha256']:raise SystemExit('Generated Mesa format mismatch')
variants=[('native',['cc'],['-O2'],[]),('ubsan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'],[])]
if any([a.i386_cc,a.i386_sysroot,a.qemu_i386]):
 if not all([a.i386_cc,a.i386_sysroot,a.qemu_i386]):raise SystemExit('Incomplete i386 triplet')
 variants.append(('i386',[str(a.i386_cc),'--sysroot='+str(a.i386_sysroot)],['-O2','-static','-march=i486'],[str(a.qemu_i386)]))
env=dict(os.environ)
if a.i386_sysroot:env['LD_LIBRARY_PATH']=str(a.i386_sysroot/'usr/lib/aarch64-linux-gnu')
result={'scope':'Host kernel API injection and interposed ioctl, NOT real kernel execution','SGX_invocations':0,'hardware_interactions':0,'variants':{}}
selected=a.targets.split(',') if a.targets else ['kernel','transport','dispatch']
if len(set(selected))!=len(selected) or any(t not in ('kernel','transport','dispatch') for t in selected):
 raise SystemExit('Unknown or duplicate target')
for name,cc,flags,runner in variants:
 result['variants'][name]={}
 for target,source in [('kernel','test_gem_memory.c'),('dispatch','test_gem_dispatch.c'),('transport','test_linux_memory.c')]:
  if target not in selected:continue
  sources=['kernel/remesa/gma500/tests/'+source]
  if target=='transport':sources+=['mesa/sgx535/remesa_linux_memory.c','mesa/sgx535/remesa_persistent.c','kernel/remesa/resource.c','kernel/remesa/registry.c','kernel/remesa/provider.c','common/remesa/draw_packet.c','mesa/sgx535/remesa_fixed_state.c','mesa/sgx535/remesa_render_pass.c','mesa/sgx535/remesa_sampler_state.c','mesa/sgx535/remesa_screen.c','mesa/sgx535/remesa_draw.c','tools/remesa/memory_probe.c']
  exe=a.output/(name+'-'+target)
  mesa_flags=['-DREMESA_WITH_MESA_HEADERS','-D_POSIX_C_SOURCE=200809L','-DHAVE_ENDIAN_H','-I'+str(a.generated),'-I'+str(a.mesa/'src/gallium/include'),'-I'+str(a.mesa/'src'),'-I'+str(a.mesa/'include')] if target=='transport' else []
  cmd=cc+mesa_flags+['-std=c11','-Wall','-Wextra','-Werror','-I'+str(root/'include'),'-I'+str(root/'mesa/sgx535'),'-I'+str(root/'tools/remesa'),'-I'+str(root/'kernel/remesa/gma500/tests')]+flags+[str(root/f) for f in sources]+['-o',str(exe)]
  c=subprocess.run(cmd,capture_output=True,text=True,env=env);(a.output/(name+'-'+target+'-compile.log')).write_text(c.stdout+c.stderr)
  if c.returncode:raise SystemExit('Compile failed: '+name+'/'+target)
  runenv=dict(env);
  if target=="transport":runenv["REMESA_MEMORY_TEST_TRANSCRIPT"]=str(a.output/(name+"-mock-transcript.bin"))
  commands=[runner+[str(exe),str(n)] for n in range(19)] if target=='dispatch' else [runner+[str(exe)]]
  runs=[subprocess.run(command,capture_output=True,text=True,env=runenv) for command in commands]
  c=subprocess.CompletedProcess(commands, next((r.returncode for r in runs if r.returncode),0), ''.join(r.stdout for r in runs), ''.join(r.stderr for r in runs));(a.output/(name+'-'+target+'-run.log')).write_text(c.stdout+c.stderr)
  if c.returncode:raise SystemExit('Test failed: '+name+'/'+target)
  result['variants'][name][target]={'PASS':True,'command':cmd,'output':c.stdout.strip()}
files=['tools/remesa/memory_probe.c','tools/remesa/memory_probe.h','tools/remesa/memory_probe_main.c','include/remesa/memory_uapi.h','kernel/remesa/gma500/remesa_gem_memory.c','kernel/remesa/gma500/remesa_gem_memory.h','kernel/remesa/gma500/tests/test_gem_shim.h','kernel/remesa/gma500/tests/test_gem_memory.c','kernel/remesa/gma500/tests/test_gem_dispatch.c','kernel/remesa/gma500/tests/test_linux_memory.c','mesa/sgx535/remesa_linux_memory.c','mesa/sgx535/remesa_linux_memory.h','mesa/sgx535/remesa_persistent.c','mesa/sgx535/remesa_persistent.h']
for name,cc,flags,runner in variants:
 if a.skip_probe_build:continue
 exe=a.output/(name+'-memory-probe')
 cmd=cc+['-std=c11','-D_POSIX_C_SOURCE=200809L','-Wall','-Wextra','-Werror','-I'+str(root/'include'),'-I'+str(root/'mesa/sgx535'),'-I'+str(root/'tools/remesa')]+flags+[str(root/f) for f in ['tools/remesa/memory_probe.c','tools/remesa/memory_probe_main.c','mesa/sgx535/remesa_linux_memory.c']]+['-o',str(exe)]
 c=subprocess.run(cmd,capture_output=True,text=True,env=env);(a.output/(name+'-probe-compile.log')).write_text(c.stdout+c.stderr)
 if c.returncode:raise SystemExit('Probe compile failed: '+name)
 result['variants'][name]['probe_binary']={'PASS':True,'executed':False,'command':cmd,'sha256':hashlib.sha256(exe.read_bytes()).hexdigest()}
result['source_sha256']={f:hashlib.sha256((root/f).read_bytes()).hexdigest() for f in files}
(a.output/'result.json').write_text(json.dumps(result,indent=2)+'\n');print({name:{t:v['PASS'] for t,v in tests.items()} for name,tests in result['variants'].items()})
