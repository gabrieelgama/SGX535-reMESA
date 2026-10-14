#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Validate actual shader callback lowering/lifetime; no hardware bytes or submissions."""
import json,os,subprocess,argparse,shlex,hashlib,sys
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--cache',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--test',choices=['test_shader_program.c','test_native_shader.c','test_shader_submit.c','test_uniform_pipeline.c','test_clip_pipeline.c','test_driver_nir.c','test_persistent_shader.c','test_shader_memory_transport.c','test_pds_data_backing.c','test_fragment_constant.c','test_retained_fragment.c'],default='test_shader_program.c');p.add_argument('--archive-only',action='store_true',help='Run only the newly built Mesa archive test; avoid unchanged source recompilations');a=p.parse_args()
repo=Path(__file__).resolve().parents[3];m=a.cache;s=m/'mesa-25.2.0';b=m/'build-i386';a.output.mkdir(parents=True,exist_ok=True)
env=dict(os.environ);env.update(json.loads((m/'build-environment.json').read_text()))
base=['-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-std=c11','-D_GNU_SOURCE','-DHAVE_ENDIAN_H','-DHAVE_SECURE_GETENV','-DHAVE_PTHREAD','-DHAVE_STRUCT_TIMESPEC','-DREMESA_WITH_MESA_HEADERS','-DREMESA_WITH_NIR']
base+=['-I'+str(x) for x in [repo/'include',repo/'mesa/sgx535',s/'include',s/'src',s/'src/gallium/include',s/'src/compiler/nir',b/'src/compiler/nir',b/'src/compiler',b/'src']]
sources=[repo/'mesa/sgx535'/x for x in ['remesa_nir.c','remesa_shader_ir.c','remesa_usse.c','remesa_pds.c','remesa_shader_program.c','remesa_fragment_body.c','remesa_shader_storage.c','remesa_program_registers.c','remesa_nir_uniform.c','remesa_fixed_state.c','remesa_render_pass.c','remesa_sampler_state.c','remesa_texture_binding.c','remesa_texture_descriptor.c','remesa_screen.c','remesa_draw.c','remesa_retained_transport.c','remesa_linux.c','remesa_persistent.c','remesa_linux_memory.c']]+[repo/'kernel/remesa'/x for x in ['registry.c','resource.c','provider.c']]+[repo/'common/remesa/draw_packet.c',repo/'common/remesa/retained_evidence.c',repo/'common/remesa/shader_request.c',b/'src/compiler/nir/nir_intrinsics.c',s/'src/util/ralloc.c',s/'src/util/u_printf.c']
q='/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/tests/qemu-root/usr/bin/qemu-i386'
results={}
wrapper_names={'test_shader_program.c':['calloc'],'test_native_shader.c':['calloc','malloc','free'],'test_uniform_pipeline.c':['malloc'],'test_shader_memory_transport.c':['ioctl','open','open64','close'],'test_retained_fragment.c':['ioctl','open','open64','close'],'test_pds_data_backing.c':['ioctl','open','open64','close','malloc','calloc','free']}
wrap=['-Wl,--wrap='+n for n in wrapper_names.get(a.test,[])]
cases=['good','split','entry-corrupt','alloc','budget','create','pin','lease','initial-corrupt','alias','write','unlease','corrupt'] if a.test=='test_pds_data_backing.c' else [None]
carrier_data=sys.stdin.buffer.read() if a.test=='test_retained_fragment.c' else None
if carrier_data is not None and len(carrier_data)!=180224:raise SystemExit('Expected exact six-role opaque carrier input on stdin')
if a.test=='test_retained_fragment.c':cases=['good','disabled','late','green','shape','source-corrupt','write-fail','info-stale','ioctl-fail','auth-fail','color-mismatch']
if a.test=='test_retained_fragment.c':base=[('-isystem'+x[2:] if x.startswith('-I'+str(m)) else x) for x in base]
carrier_input=carrier_data.decode('latin1') if carrier_data is not None else None
variants=[] if a.archive_only else [('native',['cc'],['-O2'],[]),('ubsan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'],[]),('i386',[str(m/'wrappers/cc')],['-O2','-march=i486','-static'],[q])]
for name,cc,flags,run in variants:
 exe=a.output/name;cmd=cc+base+flags+list(map(str,sources))+[str(repo/'mesa/sgx535/tests'/a.test)]+wrap+['-o',str(exe)]
 r=subprocess.run(cmd,env=env,text=True,capture_output=True);(a.output/(name+'-compile.log')).write_text(r.stdout+r.stderr)
 if r.returncode:raise SystemExit(name+' compilation failed')
 runs=[]
 for case in cases:
  runner=run+[str(exe)]+([case] if case else [])
  r=subprocess.run(runner,env=env,text=True,capture_output=True,timeout=60,encoding='latin1',input=carrier_input)
  (a.output/(name+('-'+case if case else '')+'-run.log')).write_text(r.stdout+r.stderr)
  if r.returncode:raise SystemExit(name+' execution failed: '+str(case))
  runs.append({'case':case,'command':runner,'exit':r.returncode,'output':r.stdout.strip()})
 results[name]={'pass':True,'command':cmd,'output':'\n'.join(x['output'] for x in runs),'runs':runs}
 (a.output/'result.partial.json').write_text(json.dumps(results,indent=2)+'\n')
# Exercise the actual Mesa-built driver objects too. The preceding variants
# intentionally compile sources for native/UBSan coverage; they do not alone
# prove that the Meson archive contains the same callback implementation.
record=next(x for x in json.loads((b/'compile_commands.json').read_text()) if x['file'].endswith('/remesa_nir.c'))
args=record.get('arguments') or shlex.split(record['command']);flags=[];skip=False
for arg in args:
 if skip:skip=False;continue
 if arg in ('-o','-MF','-MQ'):skip=True;continue
 if arg in ('-c','-MD','-DNDEBUG') or arg==record['file']:continue
 flags.append(arg)
flags.insert(1,'-I'+str(repo/'mesa/sgx535'));flags+=['-UNDEBUG','-DREMESA_WITH_MESA_HEADERS','-DREMESA_WITH_NIR']
archive=b/'src/gallium/drivers/sgx535/libremesa_loader.a'
if not archive.is_file():raise SystemExit('Actual Mesa driver archive missing')
# Do not silently test stale cache sources or rebuild/reselect an artifact.
# Match the staged sources to this checkout, then require Ninja to consider
# the actual driver archive current. Its digest is bound into the receipt.
driver=s/'src/gallium/drivers/sgx535';archive_inputs={}
for path in sorted(driver.rglob('*')):
 if not path.is_file() or path.suffix not in ('.c','.h'):continue
 relative=path.relative_to(driver)
 if str(relative) in ('draw_packet.c','shader_request.c','retained_evidence.c'):original=repo/'common/remesa'/relative.name
 elif str(relative) in ('registry.c','resource.c','provider.c'):original=repo/'kernel/remesa'/relative.name
 elif relative.parts[:2]==('include','remesa'):original=repo/'include/remesa'/relative.name
 elif str(relative)=='include/drm/drm.h':
  if path.read_text()!='/* SPDX-License-Identifier: MIT */\n#include <drm.h>\n':raise SystemExit('Actual Mesa DRM shim mismatch')
  archive_inputs[str(path)]=hashlib.sha256(path.read_bytes()).hexdigest();continue
 else:original=repo/'mesa/sgx535'/relative
 if not original.is_file() or path.read_bytes()!=original.read_bytes():raise SystemExit('Actual Mesa archive source mismatch: '+str(relative))
 archive_inputs[str(path)]=hashlib.sha256(path.read_bytes()).hexdigest()
freshness=['ninja','-C',str(b),'-n','src/gallium/drivers/sgx535/libremesa_loader.a']
r=subprocess.run(freshness,env=env,text=True,capture_output=True);(a.output/'archive-freshness.log').write_text(r.stdout+r.stderr)
if r.returncode or 'no work to do' not in r.stdout:raise SystemExit('Actual Mesa archive is stale; qualify a rebuilt artifact first')
archive_identity=hashlib.sha256(archive.read_bytes()).hexdigest()
libraries=['src/compiler/nir/libnir.a','src/compiler/libcompiler.a','src/util/libmesa_util.a','src/util/libmesa_util_simd.a','src/util/blake3/libblake3.a','src/util/format/libmesa_format.a','src/c11/impl/libmesa_util_c11.a']
exe=a.output/'actual-archive-i386'
cmd=flags+[str(repo/'mesa/sgx535/tests'/a.test)]+wrap+['-Wl,--start-group',str(archive)]+[str(b/n) for n in libraries if (b/n).exists()]+['-Wl,--end-group','-L'+str(m/'i386-deps/usr/lib/i386-linux-gnu'),'-ldrm','-lm','-ldl','-latomic','-pthread',str(m/'build-tools/native/usr/i686-linux-gnu/lib/libstdc++.so.6.0.33'),'-o',str(exe)]
r=subprocess.run(cmd,cwd=b,env=env,text=True,capture_output=True);(a.output/'actual-archive-compile.log').write_text(r.stdout+r.stderr)
if r.returncode:raise SystemExit('Actual Mesa archive callback link failed')
runner=[q,'-L',str(m/'runtime'),str(exe)]
runs=[]
for case in cases:
 case_runner=runner+([case] if case else [])
 r=subprocess.run(case_runner,env=env,text=True,capture_output=True,timeout=60,encoding='latin1',input=carrier_input)
 (a.output/('actual-archive'+('-'+case if case else '')+'-run.log')).write_text(r.stdout+r.stderr)
 if r.returncode:raise SystemExit('Actual Mesa archive callback execution failed: '+str(case))
 runs.append({'case':case,'command':case_runner,'exit':r.returncode,'output':r.stdout.strip()})
if hashlib.sha256(archive.read_bytes()).hexdigest()!=archive_identity:raise SystemExit('Actual Mesa archive changed during qualification')
if any(hashlib.sha256(Path(path).read_bytes()).hexdigest()!=value for path,value in archive_inputs.items()):raise SystemExit('Actual Mesa staged inputs changed during qualification')
results['actual-archive-i386']={'pass':True,'command':cmd,'runner':runner,'output':'\n'.join(x['output'] for x in runs),'runs':runs,'archive_sha256':archive_identity,'binary_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),'archive_inputs':archive_inputs,'freshness_command':freshness,'scope':'Actual Meson-built driver archive and Mesa NIR libraries; offline candidate storage/callback fixtures, no GPU submission'}
if carrier_data is not None:results['carrier_input_sha256']=hashlib.sha256(carrier_data).hexdigest()
(a.output/'result.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
