from pathlib import Path
import subprocess,hashlib,json,shutil,time
r=Path('/home/gama/sgx535-gfx');w=Path('/home/gama/sgx535-offline/candidate-01-build-20260930');previous=Path('/home/gama/sgx535-offline/candidate-01-20260930');source=Path('/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp');out=Path('/home/gama/sgx535-offline/antix-kbuild-successor-20260930/output')
def h(p):return hashlib.sha256(p.read_bytes()).hexdigest()
inputs=json.loads((previous/'inputs.json').read_text())
for path,digest in inputs.items():assert h(Path(path))==digest,path
(w/'inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
checks=json.loads((previous/'tool-identity-check.json').read_text())
for row in checks:assert h(Path(row['path']))==row['sha256'],row['path']
(w/'tool-identity-check.json').write_text(json.dumps(checks,indent=2)+'\n')
before=json.loads((previous/'prepared-state-before.json').read_text())
for rel,digest in before.items():assert h(out/rel)==digest,rel
(w/'prepared-state-before.json').write_text(json.dumps(before,indent=2)+'\n')
for p in ('vmlinux.symvers','Module.symvers'):assert not (out/p).exists(),p
mod=w/'module';shutil.copytree(source/'drivers/gpu/drm/gma500',mod);assert not list(mod.glob('*.o'));assert not list(mod.glob('*.ko'))
original_manifest=json.loads((previous/'original-gma500-source.json').read_text())
for rel,digest in original_manifest.items():assert h(mod/rel)==digest,rel
(w/'original-source-manifest.json').write_text(json.dumps(original_manifest,indent=2)+'\n')
fixed=json.loads((previous/'fixed-inputs.json').read_text())
for row in fixed:
 p=r/row['repository_path'];assert h(p)==row['sha256'],p;shutil.copyfile(p,mod/row['module_filename'])
(w/'fixed-inputs.json').write_text(json.dumps(fixed,indent=2)+'\n')
patches=[]
for name in ('antix-fixed-makefile.patch','antix-fixed-irq.patch','antix-fixed-ioctl.patch'):
 p=r/'kernel/sgx535_frozen/patches'/name;shutil.copyfile(p,w/('after-'+name))
 for dry in (True,False):
  argv=['/usr/bin/patch','--batch','--fuzz=0','-p5','-i',str(p)]+(['--dry-run'] if dry else [])
  cp=subprocess.run(argv,cwd=mod,capture_output=True,text=True);row={'patch':name,'sha256':h(p),'argv':argv,'cwd':str(mod),'exit_code':cp.returncode,'stdout':cp.stdout,'stderr':cp.stderr};patches.append(row)
  assert cp.returncode==0 and 'offset' not in cp.stdout and 'fuzz' not in cp.stdout,(name,cp.stdout,cp.stderr)
assert not list(mod.glob('*.rej'));(w/'patch-application.json').write_text(json.dumps(patches,indent=2)+'\n')
(w/'integrated-source-manifest.json').write_text(json.dumps({str(p.relative_to(mod)):h(p) for p in sorted(mod.rglob('*')) if p.is_file()},indent=2)+'\n')
target=r/'docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/build_Module_symvers';shutil.copyfile(target,out/'Module.symvers');assert h(out/'Module.symvers')=='faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca'
(w/'table-installation.json').write_text(json.dumps({'source':str(target),'destination':str(out/'Module.symvers'),'sha256':h(out/'Module.symvers'),'method':'byte-exact ordinary Kbuild input copy; not edited'},indent=2)+'\n')
env=json.loads((previous/'build-environment.json').read_text());env.update(KBUILD_BUILD_TIMESTAMP=time.strftime('%Y-%m-%d %H:%M:%S UTC',time.gmtime()),SOURCE_DATE_EPOCH=str(int(time.time())))
(w/'build-environment.json').write_text(json.dumps(env,indent=2)+'\n');(w/'paths.json').write_text(json.dumps({'source':str(source),'output':str(out),'module':str(mod),'workspace':str(w)},indent=2)+'\n')
args=['make','-C',str(source),'O='+str(out),'ARCH=x86']+[k+'='+env[k] for k in ('CC','CROSS_COMPILE','LD','AS','HOSTCC','HOSTCFLAGS','HOSTLDFLAGS')]+['LOCALVERSION=-486-smp','M='+str(mod),'V=1','modules']
(w/'build-argv.json').write_text(json.dumps(args,indent=2)+'\n')
with (w/'progress.md').open('a') as f:f.write('Task 2 complete: fresh 71+15 inputs, all three corrected/retained patches apply exactly with fuzz=0/no offsets/no rejects. Qualified input/tool/prepared hashes verified. Captured target table installed byte-exact as normal output-tree input. Build command/environment recorded.\n')
print('Kbuild command prepared',len(args),'arguments; sources',len(list(mod.glob('*'))))
