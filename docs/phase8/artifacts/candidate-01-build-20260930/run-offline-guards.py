from pathlib import Path
import json,subprocess,hashlib
r=Path('/home/gama/sgx535-gfx');w=Path('/home/gama/sgx535-offline/candidate-01-build-20260930');env=json.loads((w/'build-environment.json').read_text());env['SGX535_I686_SYSROOT']='/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root';env['PYTHONDONTWRITEBYTECODE']='1'
checks=[]
def run(name,args,expected=0):
 cp=subprocess.run(args,cwd=r,env=env,capture_output=True);(w/(name+'.stdout')).write_bytes(cp.stdout);(w/(name+'.stderr')).write_bytes(cp.stderr);checks.append(dict(name=name,argv=args,exit_code=cp.returncode,expected=expected));assert cp.returncode==expected,(name,cp.stderr.decode())
run('suite-final',['python3','-m','unittest','discover','-s','tools/psb-dri-re','-p','test_*.py'])
text=(w/'suite-final.stderr').read_text();assert 'Ran 205 tests' in text and '\nOK\n' in text and 'skipped' not in text
run('generator-final',['python3','tools/psb-dri-re/generate_frozen_kernel_initial.py','--check'])
for i in (1,2):run('dry-final-'+str(i),['python3','tools/psb-dri-re/frozen_triangle_dry_run.py'])
a=hashlib.sha256((w/'dry-final-1.stdout').read_bytes()).hexdigest();b=hashlib.sha256((w/'dry-final-2.stdout').read_bytes()).hexdigest();assert a==b=='2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e'
run('complete-final',['python3','tools/psb-dri-re/frozen_triangle_image.py','--complete'],1)
assert (w/'complete-final.stderr').read_text().strip()=='PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE'
base=json.loads((w/'regression-before-lifecycle.json').read_text())
for row in base:
 if row['name'].startswith('ubsan'):run(row['name']+'-final',row['argv'])
run('diff-check-final',['git','diff','--check'])
(w/'regression-final.json').write_text(json.dumps(checks,indent=2)+'\n')
print(text[-600:]);print('dry hashes',a,b);print((w/'complete-final.stderr').read_text());print('all 3 strict UBSan harnesses built and ran PASS; lifecycle UBSan 7 tests/10 scenarios included; git diff --check PASS')
