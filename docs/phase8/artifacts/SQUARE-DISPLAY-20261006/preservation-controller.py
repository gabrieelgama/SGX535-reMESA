from pathlib import Path
import sys,json,base64,hashlib,importlib.util,datetime,os,struct
sys.dont_write_bytecode=True
W=Path(Path('/tmp/sgx535-square-display-workspace').read_text());render=Path(Path('/tmp/sgx535-square-continuation-workspace').read_text());sp=importlib.util.spec_from_file_location('c',render/'controller.py');c=importlib.util.module_from_spec(sp);sp.loader.exec_module(c);c.ROOT=W;remote=json.loads((W/'remote-paths.json').read_text())
s='import json,base64,os,stat,hashlib\nfrom pathlib import Path\np=Path('+repr(remote['evidence'])+')\n'+'''st=os.lstat(p);assert stat.S_ISDIR(st.st_mode) and st.st_uid==1000 and (st.st_mode&0o777)==0o700
files={}
fd=os.open(p,os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW)
try:
 for name in sorted(os.listdir(fd)):
  f=os.open(name,os.O_RDONLY|os.O_NOFOLLOW|os.O_NONBLOCK,dir_fd=fd)
  try:
   a=os.fstat(f);assert stat.S_ISREG(a.st_mode) and a.st_nlink==1 and a.st_uid==1000 and a.st_size<=1048576
   with os.fdopen(f,'rb',closefd=False) as h:data=h.read(1048577)
   z=os.fstat(f);assert (a.st_dev,a.st_ino,a.st_size,a.st_mtime_ns,a.st_ctime_ns)==(z.st_dev,z.st_ino,z.st_size,z.st_mtime_ns,z.st_ctime_ns) and len(data)==a.st_size
   files[name]={'payload_base64':base64.b64encode(data).decode(),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'dev':a.st_dev,'ino':a.st_ino}
  finally:os.close(f)
finally:os.close(fd)
assert (os.lstat(p).st_dev,os.lstat(p).st_ino)==(st.st_dev,st.st_ino)
print(json.dumps({'files':files,'sgx_invocations':0,'directory':str(p),'boot_id':Path('/proc/sys/kernel/random/boot_id').read_text().strip()}))
'''
v,_=c.once('read-only-evidence-retrieval',s,timeout=120);r=v[-1];assert r['boot_id']=='83ee4ff8-a7f1-4468-b348-d10627f38d17';D=W/'originals';D.mkdir();rows=[]
for name,x in sorted(r['files'].items()):
 b=base64.b64decode(x['payload_base64'],validate=True);assert len(b)==x['bytes'] and hashlib.sha256(b).hexdigest()==x['sha256'];fd=os.open(D/name,os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW,0o400)
 with os.fdopen(fd,'wb') as f:f.write(b);f.flush();os.fsync(f.fileno())
 rows.append({'path':name,'bytes':len(b),'sha256':x['sha256']})
remote_seal=json.loads((D/'seal.json').read_text());assert all(len((D/n).read_bytes())==x['bytes'] and hashlib.sha256((D/n).read_bytes()).hexdigest()==x['sha256'] for n,x in remote_seal['files'].items());assert set(r['files'])==set(remote_seal['files'])|{'seal.json'}
(D/'originals.seal.json').write_text(json.dumps({'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'files':rows,'count':len(rows),'remote_seal_verified':True,'preservation_before_interpretation':True},indent=2)+'\n')
print('Originals preserved before interpretation; remote9-file seal verified; local originals',len(rows),flush=True)
# Interpret only after originals are sealed.
sys.path.insert(0,'/home/gama/sgx535-gfx/tools/display');import square_display_centered as square
source=(D/'source.original.bin').read_bytes();square.validate_square(source,square.SOURCE_SHA);published=(D/'published.original.bin').read_bytes();before=(D/'screen-before.original.bin').read_bytes();restored=(D/'restored.original.bin').read_bytes();expected=square.centered.enlarge(source);assert len(before)==len(restored)==len(published)==409600
rgbmatch=all(published[i:i+3]==expected[i:i+3] for i in range(0,len(expected),4));restoration=before==restored
values=struct.unpack('<102400I',published);low=[v&0xffffff for v in values];nonzero=[(480+i%320,240+i//320) for i,v in enumerate(low) if v];bbox=[min(x for x,y in nonzero),min(y for x,y in nonzero),max(x for x,y in nonzero),max(y for x,y in nonzero)] if nonzero else None
result={'classification':'SOFTWARE_SQUARE_PUBLICATION_AND_RESTORATION_CONFIRMED_PHYSICAL_OBSERVATION_PENDING' if rgbmatch and restoration else 'DISPLAY_PUBLICATION_FAILED','architecture':'SGX_RENDERED_PRESERVED_SQUARE + CPU/XORG_PUBLICATION; NOT DIRECT SGX SCANOUT','source_sha256':square.SOURCE_SHA,'boot_id':r['boot_id'],'source_pixels':1024,'source_magenta_pixels':256,'source_zero_pixels':768,'presentation_scale':10,'affected_rectangle':{'x':480,'y':240,'width':320,'height':320},'hold_seconds':15,'publication_rgb_match':rgbmatch,'restoration_exact_all409600bytes':restoration,'published_magenta_pixels':low.count(0xff00ff),'published_zero_pixels':low.count(0),'unique_rgb_values':[hex(x) for x in sorted(set(low))],'magenta_bbox_inclusive':bbox,'coordinates':'all integer(x,y) with560<=x<=719 and320<=y<=479' if bbox==[560,320,719,479] and low.count(0xff00ff)==25600 else 'see preserved published.original.bin','display_attempts':1,'sgx_invocations_this_display_task':0,'authorization_consumed':True,'further_display_authorized':False,'further_sgx_authorized':False,'remote_original_seal_verified':True,'originals_preserved_before_interpretation':True,'operator_observation':'PENDING','source_originals':str(D),'card':str(W/'card.json')}
(W/'interpretation.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
