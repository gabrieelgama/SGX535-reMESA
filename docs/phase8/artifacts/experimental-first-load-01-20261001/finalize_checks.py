from pathlib import Path
import subprocess,json,hashlib,re,os
r=Path('/home/gama/sgx535-gfx');w=Path(__file__).parent;e=r/'docs/phase8/artifacts/experimental-first-load-01-20261001';g=e/'post-documentation-checks';g.mkdir()
env=json.loads(Path('/home/gama/sgx535-offline/candidate-01-build-20260930/build-environment.json').read_text());env.update(SGX535_I686_SYSROOT='/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root',PYTHONDONTWRITEBYTECODE='1')
checks=[]
def run(name,argv):
 cp=subprocess.run(argv,cwd=r,env=env,capture_output=True)
 (g/(name+'.stdout')).write_bytes(cp.stdout);(g/(name+'.stderr')).write_bytes(cp.stderr)
 checks.append({'name':name,'argv':argv,'exit_code':cp.returncode});(g/'checks.json').write_text(json.dumps(checks,indent=2)+'\n');assert cp.returncode==0,(name,cp.stderr.decode())
run('final-suite',['python3','-m','unittest','discover','-s','tools/psb-dri-re','-p','test_*.py'])
s=(g/'final-suite.stderr').read_text();assert 'Ran 249 tests' in s and '\nOK\n' in s and 'skipped' not in s
run('diff-check',['git','diff','--check'])
run('final-status',['git','status','--short'])
notice=(e/'checkpoint-notice.txt').read_bytes();allowed={x['path'] for x in json.loads((e/'allowed-checkpoint-updates.json').read_text())}
initial=json.loads((w/'initial-files.json').read_text());changes=[];same=0
for name,digest in initial.items():
 p=r/name;assert p.is_file(),('removed initial file',name)
 data=p.read_bytes();now=hashlib.sha256(data).hexdigest()
 if now==digest:same+=1;continue
 assert name in allowed,('unrelated modification',name)
 assert data.count(notice)==1 and hashlib.sha256(data.replace(notice,b'',1)).hexdigest()==digest,name
 changes.append({'path':name,'old_sha256':digest,'new_sha256':now,'historical_contents_preserved':True})
assert {x['path'] for x in changes}==allowed
names=set(subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard'],cwd=r,text=True).splitlines());added=sorted(names-set(initial))
expected=['docs/phase8/experimental-first-load-build-plan.md','docs/phase8/experimental-first-load-image-qualification.md','kernel/sgx535_frozen/first_load/preload.sh.in','tools/psb-dri-re/frozen_first_load_image.py','tools/psb-dri-re/verify_frozen_first_load_image.py','tools/psb-dri-re/test_frozen_first_load_image.py']
for name in added:assert name in expected or name.startswith(str(e.relative_to(r))+'/'),('unexpected new path',name)
text_files=expected+list(allowed)+[str((e/'README.md').relative_to(r))]
links=[]
for name in text_files:
 p=r/name;data=p.read_text()
 assert all(not line.endswith((' ','\t')) for line in data.splitlines()),('trailing whitespace',name)
 if name.endswith('.py'):compile(data,str(p),'exec')
 if name.endswith('.md'):
  for link in re.findall(r'\]\(([^)]+)\)',data):
   if '://' in link or link.startswith('#'):continue
   target=p.parent/link.split('#')[0]
   assert target.exists(),('missing link',name,link)
   links.append({'document':name,'target':link,'exists':True})
(g/'document-links.json').write_text(json.dumps(links,indent=2)+'\n')
actual=e/'build-03/initrd.img-sgx535-firstload-01';h=hashlib.sha256(actual.read_bytes()).hexdigest();assert h=='4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71'
(e/'working-tree-delta.json').write_text(json.dumps({'initial_files':len(initial),'unchanged_initial_files':same,'changed_preexisting_files':changes,'added_files':added,'removed_files':[],'production_driver_patches_and_binaries_unchanged':True,'historical_evidence_unchanged':True,'target_contact':False,'target_mutation':False,'new_candidate_build':False,'module_operations':False,'sgx_fire':False,'stage_commit_push':False},indent=2)+'\n')
(g/'summary.json').write_text(json.dumps({'scoped_tests':249,'skips':0,'final_git_diff_check':'PASS','added_text_whitespace_and_Python_parse':'PASS','documentation_links':'PASS','initial_file_preservation':'PASS','artifact_copy_hash':'PASS','gate_b':'BLOCKED','whitelist':[]},indent=2)+'\n')
print(json.dumps({'scoped_tests':249,'preserved_initial_files':same,'changed_current_notices':len(changes),'added_files':len(added),'artifact_sha256':h},indent=2))
