from pathlib import Path
import json,shutil,hashlib,subprocess
r=Path('/home/gama/sgx535-gfx');w=Path('/home/gama/sgx535-offline/candidate-01-build-20260930');s=Path('/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp');mod=w/'module'
def h(p):return hashlib.sha256(p.read_bytes()).hexdigest()
(mod).rename(w/'build-6-candidate02-first')
shutil.copytree(s/'drivers/gpu/drm/gma500',mod)
for rel,sha in json.loads((w/'original-source-manifest.json').read_text()).items():assert h(mod/rel)==sha,rel
for row in json.loads((w/'fixed-inputs-lifecycle.json').read_text()):
 p=r/row['repository_path'];assert h(p)==row['sha256'];shutil.copyfile(p,mod/row['module_filename'])
records=[]
for name in ('antix-fixed-makefile.patch','antix-fixed-irq.patch','antix-irq-lifecycle.patch','antix-fixed-ioctl.patch'):
 p=r/'kernel/sgx535_frozen/patches'/name
 for dry in (True,False):
  argv=['patch','--batch','--fuzz=0','-p5','-i',str(p)]+(['--dry-run'] if dry else [])
  cp=subprocess.run(argv,cwd=mod,capture_output=True,text=True)
  records.append(dict(patch=name,sha256=h(p),argv=argv,cwd=str(mod),exit_code=cp.returncode,stdout=cp.stdout,stderr=cp.stderr))
  assert cp.returncode==0 and not cp.stderr and 'offset' not in cp.stdout and 'fuzz' not in cp.stdout,(name,cp.stdout,cp.stderr)
assert not list(mod.glob('*.rej'))
(w/'lifecycle-repeat-patch-application.json').write_text(json.dumps(records,indent=2)+'\n')
for rel,sha in json.loads((w/'integrated-source-manifest-lifecycle-final.json').read_text()).items():assert h(mod/rel)==sha,rel
print('clean repeat staging source-equivalent, four strict patches pass')
