import sys,json,base64,datetime,hashlib,importlib.util
from pathlib import Path
sys.dont_write_bytecode=True
P=Path('/home/gama/sgx535-gfx');render=Path(Path('/tmp/sgx535-square-continuation-workspace').read_text());W=Path('/home/gama/sgx535-offline')/('phase8-square-display-one-publication-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ'));W.mkdir();Path('/tmp/sgx535-square-display-workspace').write_text(str(W))
sp=importlib.util.spec_from_file_location('c',render/'controller.py');c=importlib.util.module_from_spec(sp);sp.loader.exec_module(c);c.ROOT=W
qualification=Path('/tmp/square-display-qualification.json').read_bytes();(W/'qualification.json').write_bytes(qualification);assert json.loads(qualification)['passed']==27
(W/'controller-audit-PASS.json').write_text(json.dumps({'scope':'display-only path; exact source; CPU27/27 PASS','sgx_invocations':0,'display_attempts_maximum':1}))
remote='/home/gama/sgx535-square-display-'+W.name.split('publication-')[1];toolsdir=remote+'-tools';evidence=remote+'-evidence'
files={}
for name in ['triangle_pixels.py','triangle_display.py','triangle_display_centered.py','square_display_centered.py']:
 b=(P/'tools/display'/name).read_bytes();(W/name).write_bytes(b);files[name]=base64.b64encode(b).decode()
source=(render/'one-authorized-square-call/originals/color.original.bin').read_bytes();assert hashlib.sha256(source).hexdigest()=='6e9af8e8b6576b979aa78816d44bd3b0ffd31b70e169a982e83736d63ab91729';(W/'source.bin').write_bytes(source);files['source.bin']=base64.b64encode(source).decode()
s='import os,json,base64,hashlib\nfrom pathlib import Path\nfiles='+repr(files)+'\nroot=Path('+repr(toolsdir)+')\nassert not Path('+repr(evidence)+').exists()\n'+'''root.mkdir(mode=0o700,exist_ok=False)
receipt={}
for name,encoded in files.items():
 data=base64.b64decode(encoded,validate=True);path=root/name
 fd=os.open(path,os.O_CREAT|os.O_EXCL|os.O_WRONLY|os.O_NOFOLLOW,0o600)
 with os.fdopen(fd,'wb') as f:f.write(data);f.flush();os.fsync(f.fileno())
 assert path.read_bytes()==data
 receipt[name]={'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}
fd=os.open(root,os.O_DIRECTORY|os.O_RDONLY);os.fsync(fd);os.close(fd)
print(json.dumps({'deployment':str(root),'verified_files':receipt,'display_mutations':0,'sgx_invocations':0}))
'''
v,_=c.once('tools-deployment',s,timeout=40);print('isolated display tools deployed; no display write',flush=True)
s='import subprocess,json\np=subprocess.run('+repr(['python3','-B',toolsdir+'/square_display_centered.py','--inspect'])+',capture_output=True,timeout=30)\nprint(p.stdout.decode(),end="")\nif p.returncode or p.stderr:raise RuntimeError(p.stderr.decode())\n'
v,_=c.once('passive-display-inspection',s,timeout=40);r=v[-1];target=r['target'];assert target['boot_id']=='83ee4ff8-a7f1-4468-b348-d10627f38d17' and target['loaded_driver_note_sha256']=='35ec9f4e576a83a3990ebda4f8aee5869c0e4ea32e1e35969fa48abffa7dc832' and target['loaded_observer_note_sha256']=='6afbac1cf48a211135e2a4b4ffc30f96ace64f6d397071595bbc328966bb8af0' and (target['width'],target['height'],target['depth'])==(1280,800,24) and target['scanout']['layout']['pitch']==5120
card={'schema':'SGX535_DISPLAY_PUBLICATION_CARD_V1','variant':'CENTERED_SEALED_SQUARE_NEAREST_NEIGHBOR_10X','scope':'ONE DISPLAY-ONLY CPU PUBLICATION; NO SGX','target':target,'source':{'path':'docs/phase8/artifacts/MULTI-TRIANGLE-20261006/live-campaign/one-authorized-square-call/originals/color.original.bin','bytes':4096,'sha256':hashlib.sha256(source).hexdigest(),'representation':'1024 little-endian ARGB8888 words; stride128; immutable SGX square','magenta_pixels':256,'zero_pixels':768},'hold_seconds':15,'affected_rectangle':{'x':480,'y':240,'width':320,'height':320},'visible_square_bbox_inclusive':[560,320,719,479],'expected_publication_pixels':{'magenta':25600,'zero':76800},'source_pitch':128,'publication_pitch':1280,'scanout_pitch':5120,'affected_scanout_row_intervals':[[y*5120+1920,y*5120+3200] for y in range(240,560)],'restore':'preserve original409600 bytes before publication; restore exact region then verify all bytes','tools':{str(Path('tools/display')/n):{'bytes':len(base64.b64decode(files[n])),'sha256':hashlib.sha256(base64.b64decode(files[n])).hexdigest()} for n in files if n.endswith('.py')},'evidence_directory':evidence,'maximum_publications':1,'no_retry':True,'sgx_execution_authorized':False,'stop_conditions':['exact source/tool/destination mismatch','owner/boot/module/mode change','backup failure before publication','publication/restoration/evidence loss or ambiguity; STOP no retry'],'physical_visibility':'requires operator confirmation; machine publication/restoration independently captured'}
raw=(json.dumps(card,indent=2)+'\n').encode();authorization={'scope':card['scope'],'card_sha256':hashlib.sha256(raw).hexdigest(),'boot_id':target['boot_id'],'maximum_publications':1,'display_publication_authorized':True,'sgx_execution_authorized':False}
(W/'card.json').write_bytes(raw);(W/'authorization.json').write_text(json.dumps(authorization,indent=2)+'\n');(W/'remote-paths.json').write_text(json.dumps({'tools':toolsdir,'evidence':evidence},indent=2));(P/'docs/phase8/square-display-execution-card-20261006.json').write_bytes(raw)
print('fresh display:',json.dumps(target),flush=True);print('card ready; expected160x160 magenta square centered at560,320 inside320x320 region')
