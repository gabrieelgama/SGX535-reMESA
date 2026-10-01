from pathlib import Path
import sys,json,gzip,zlib,hashlib,subprocess
r=Path('/home/gama/sgx535-gfx');w=Path(__file__).parent
sys.path.insert(0,str(r/'tools/psb-dri-re'))
import frozen_first_load_image as c
import verify_frozen_first_load_image as v
inputs=c.load_inputs(r);data=(w/'build-03/initrd.img-sgx535-firstload-01').read_bytes();offset=len(inputs['prefix'])
main=zlib.decompress(data[offset:],31);rows,end=c.scan(main)
results=[]
mutants=w/'negative-images';mutants.mkdir()
for case in ['init-not-executable','payload-inode-collision','corrupt-private-payload','corrupt-early-prefix']:
 new=[]
 for row in rows:
  f=list(row['fields']);payload=row['data'];raw=row['raw']
  if case=='init-not-executable' and row['name']=='init': f[1]=0o100644;raw=c.record(row['name'],payload,f)
  if case=='payload-inode-collision' and row['name']==c.PAYLOAD:f[0]=rows[0]['fields'][0];raw=c.record(row['name'],payload,f)
  if case=='corrupt-private-payload' and row['name']==c.PAYLOAD:payload=payload[:-1]+bytes([payload[-1]^1]);raw=c.record(row['name'],payload,f)
  new.append(raw)
 raw=b''.join(new)+c.record('TRAILER!!!',b'');raw+=b'\0'*((-len(raw))%512)
 prefix=inputs['prefix']
 if case=='corrupt-early-prefix':prefix=bytes([prefix[0]^1])+prefix[1:]
 bad=prefix+gzip.compress(raw,compresslevel=1,mtime=0)
 p=mutants/(case+'.img');p.write_bytes(bad)
 proof=mutants/(case+'-UNEXPECTED-proof')
 try:v.verify(r,p,proof)
 except ValueError as err:
  assert not proof.exists(),case
  results.append({'case':case,'bytes':len(bad),'sha256':hashlib.sha256(bad).hexdigest(),'classification':'REJECTED','reason':str(err),'proof_directory_created':False})
 else:raise AssertionError('mutation admitted '+case)
(mutants/'results.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))
