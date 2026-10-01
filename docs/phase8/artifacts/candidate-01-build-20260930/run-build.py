from pathlib import Path
import json,subprocess,time,sys
w=Path('/home/gama/sgx535-offline/candidate-01-build-20260930');label=sys.argv[1]
args=json.loads((w/'build-argv.json').read_text());env=json.loads((w/'build-environment.json').read_text());start=time.time()
with (w/(label+'.stdout')).open('wb') as out,(w/(label+'.stderr')).open('wb') as err:p=subprocess.run(args,env=env,stdout=out,stderr=err)
record={'argv':args,'environment':env,'exit_code':p.returncode,'started_unix':start,'ended_unix':time.time()};(w/(label+'.json')).write_text(json.dumps(record,indent=2)+'\n')
print(label,'exit',p.returncode,'seconds',round(time.time()-start,3));print((w/(label+'.stderr')).read_text()[-6000:]);sys.exit(p.returncode)
