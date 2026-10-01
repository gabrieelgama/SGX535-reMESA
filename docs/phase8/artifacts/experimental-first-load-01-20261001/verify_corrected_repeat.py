from pathlib import Path
import json,hashlib,subprocess
w=Path(__file__).parent
paths=[w/f'build-0{n}/initrd.img-sgx535-firstload-01' for n in [3,4]]
a,b=[p.read_bytes() for p in paths];assert a==b
result={'result':'BYTE-IDENTICAL','images':[str(p) for p in paths],'bytes':len(a),'sha256':hashlib.sha256(a).hexdigest(),'supersedes':'corrected-repeat-build.json whose runner retained old build01/02 read paths; no image is altered','shell_checks':[]}
for n in [3,4]:
 for name in ['sgx535-first-load.sh','modified-init']:
  argv=['/bin/sh','-n',str(w/f'build-0{n}'/name)];cp=subprocess.run(argv,capture_output=True)
  assert cp.returncode==0 and not cp.stdout and not cp.stderr
  result['shell_checks'].append({'argv':argv,'exit_code':cp.returncode,'stdout':'','stderr':''})
(w/'verified-corrected-repeat.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result,indent=2))
