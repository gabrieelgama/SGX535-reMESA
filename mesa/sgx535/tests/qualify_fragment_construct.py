#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Offline NIR semantic admission qualification; never opens a DRM device."""
import argparse, hashlib, json, os, shlex, subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--cache',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
repo=Path(__file__).resolve().parents[3];m=a.cache;s=m/'mesa-25.2.0';b=m/'build-i386';a.output.mkdir(parents=True,exist_ok=True)
env=dict(os.environ);env.update(json.loads((m/'build-environment.json').read_text()))
base=['-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-std=c11','-D_GNU_SOURCE','-DHAVE_ENDIAN_H','-DHAVE_SECURE_GETENV','-DHAVE_PTHREAD','-DHAVE_STRUCT_TIMESPEC','-DREMESA_WITH_MESA_HEADERS','-DREMESA_WITH_NIR']
base+=['-I'+str(x) for x in [repo/'include',repo/'mesa/sgx535',s/'include',s/'src',s/'src/gallium/include',s/'src/compiler/nir',b/'src/compiler/nir',b/'src/compiler',b/'src']]
sources=[repo/'mesa/sgx535/remesa_nir.c',b/'src/compiler/nir/nir_intrinsics.c',s/'src/util/ralloc.c',s/'src/util/u_printf.c']
q='/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/tests/qemu-root/usr/bin/qemu-i386'
results={}
for name,cc,flags,run in [('native',['cc'],['-O2'],[]),('ubsan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'],[]),('i386',[str(m/'wrappers/cc')],['-O2','-march=i486','-static'],[q])]:
 exe=a.output/name;cmd=cc+base+flags+list(map(str,sources))+[str(repo/'mesa/sgx535/tests/test_fragment_construct.c'),'-o',str(exe)]
 r=subprocess.run(cmd,env=env,text=True,capture_output=True);(a.output/(name+'-compile.log')).write_text(r.stdout+r.stderr)
 if r.returncode:raise SystemExit(name+' compilation failed')
 r=subprocess.run(run+[str(exe)],env=env,text=True,capture_output=True);(a.output/(name+'-run.log')).write_text(r.stdout+r.stderr)
 if r.returncode:raise SystemExit(name+' execution failed')
 results[name]={'pass':True,'command':cmd,'output':r.stdout.strip()}
# Actual builder linked to the already built, public Mesa 25.2.0 NIR library.
records=json.loads((b/'compile_commands.json').read_text())
record=next(x for x in records if x['file'].endswith('/remesa_nir.c'))
args=record.get('arguments') or shlex.split(record['command']);filtered=[];skip=False
for arg in args:
 if skip:skip=False;continue
 if arg in ('-o','-MF','-MQ'):skip=True;continue
 if arg in ('-c','-MD','-DNDEBUG') or arg==record['file']:continue
 filtered.append(arg)
filtered.insert(1,'-I'+str(repo/'mesa/sgx535'))
exe=a.output/'builder-i386'
cmd=filtered+[str(repo/'mesa/sgx535/tests/test_fragment_construct_builder.c'),str(repo/'mesa/sgx535/remesa_nir.c'),'-Wl,--start-group']+[str(b/x) for x in ['src/compiler/nir/libnir.a','src/compiler/libcompiler.a','src/util/libmesa_util.a']]+['-Wl,--end-group',str(m/'build-tools/native/usr/i686-linux-gnu/lib/libstdc++.so.6.0.33'),'-lm','-ldl','-latomic','-pthread','-o',str(exe)]
r=subprocess.run(cmd,cwd=b,env=env,text=True,capture_output=True);(a.output/'builder-compile.log').write_text(r.stdout+r.stderr)
if r.returncode:raise SystemExit('actual builder compilation failed')
run=[q,'-L',str(m/'runtime'),str(exe)]
r=subprocess.run(run,env=env,text=True,capture_output=True);(a.output/'builder-run.log').write_text(r.stdout+r.stderr)
if r.returncode:raise SystemExit('actual builder execution failed')
results['builder-i386']={'pass':True,'command':cmd,'run':run,'output':r.stdout.strip()}
results['source_sha256']={str(x.relative_to(repo)):hashlib.sha256(x.read_bytes()).hexdigest() for x in [repo/'mesa/sgx535/remesa_nir.c',repo/'mesa/sgx535/remesa_nir.h',Path(__file__),repo/'mesa/sgx535/tests/test_fragment_construct.c',repo/'mesa/sgx535/tests/test_fragment_construct_builder.c']}
results['scope']='CPU semantic recognition of existing constant hardware profiles; no SGX execution or new ISA'
(a.output/'result.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
