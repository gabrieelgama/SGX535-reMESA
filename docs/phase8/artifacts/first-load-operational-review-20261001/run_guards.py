from pathlib import Path
import hashlib,json,subprocess
r=Path('/home/gama/sgx535-gfx');w=Path('/home/gama/sgx535-offline/boot-provenance-readonly-20261001T025947Z')
e=r/'docs/hardware-evidence/MINI12-20261001T030754Z-BOOT-PROVENANCE-READONLY-02'
a=e/'offline-analysis';g=Path('/home/gama/sgx535-offline/first-load-operational-review-20261001T050731Z/checks');g.mkdir(exist_ok=False)
p=Path('/home/gama/sgx535-offline/candidate-01-build-20260930')
env=json.loads((p/'build-environment.json').read_text());env['SGX535_I686_SYSROOT']='/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root';env['PYTHONDONTWRITEBYTECODE']='1'
records=[]
def run(name,argv,cwd=r,expected=0):
 cp=subprocess.run(argv,cwd=cwd,env=env,capture_output=True)
 (g/(name+'.stdout')).write_bytes(cp.stdout);(g/(name+'.stderr')).write_bytes(cp.stderr)
 records.append({'name':name,'argv':argv,'cwd':str(cwd),'exit_code':cp.returncode,'expected':expected})
 (g/'checks.json').write_text(json.dumps(records,indent=2)+'\n')
 assert cp.returncode==expected,(name,cp.stderr.decode())
run('analysis-helper-tests',['python3','-m','unittest','test_decode_capture','test_read_initramfs','-v'],a)
run('suite',['python3','-m','unittest','discover','-s','tools/psb-dri-re','-p','test_*.py'])
s=(g/'suite.stderr').read_text();assert 'Ran 261 tests' in s and '\nOK\n' in s and 'skipped' not in s,s
run('procedure-plan',['python3','tools/psb-dri-re/frozen_first_load_procedure.py'])
for row in json.loads((r/'docs/phase8/artifacts/experimental-first-load-01-20261001/abi-recheck-commands.json').read_text()):
 label='abi-'+Path(row['argv'][-2]).parent.name
 run(label,row['argv'])
run('generator',['python3','tools/psb-dri-re/generate_frozen_kernel_initial.py','--check'])
for i in (1,2):run('dry-'+str(i),['python3','tools/psb-dri-re/frozen_triangle_dry_run.py'])
hashes=[hashlib.sha256((g/f'dry-{i}.stdout').read_bytes()).hexdigest() for i in (1,2)]
assert hashes==['2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e']*2
run('complete',['python3','tools/psb-dri-re/frozen_triangle_image.py','--complete'],expected=1)
assert (g/'complete.stderr').read_text().strip()=='PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE'
for row in json.loads((p/'regression-before-lifecycle.json').read_text()):
 if row['name'].startswith('ubsan'):
  argv=[v.replace(str(p)+'/',str(g)+'/') for v in row['argv']]
  run(row['name'],argv)
run('git-diff-check',['git','diff','--check'])
run('git-status',['git','status','--short'])
(g/'summary.json').write_text(json.dumps({'repository_tests':261,'repository_skips':0,'analysis_helper_tests':14,'generator':'PASS','strict_ubsan_harnesses':3,'dry_hashes':hashes,'complete':'PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE','git_diff_check':'PASS'},indent=2)+'\n')
print(s[-180:]);print('Helper tests14, generator, three UBSan harnesses, both dry hashes and expected complete refusal PASS.')
