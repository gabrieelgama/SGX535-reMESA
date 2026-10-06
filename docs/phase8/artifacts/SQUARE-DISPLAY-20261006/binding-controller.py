from pathlib import Path
import json,sys,importlib.util,base64
sys.dont_write_bytecode=True
W=Path(Path('/tmp/sgx535-square-display-workspace').read_text());render=Path(Path('/tmp/sgx535-square-continuation-workspace').read_text());sp=importlib.util.spec_from_file_location('c',render/'controller.py');c=importlib.util.module_from_spec(sp);sp.loader.exec_module(c);c.ROOT=W;remote=json.loads((W/'remote-paths.json').read_text());card=json.loads((W/'card.json').read_text());assert json.loads((W/'native-ubsan-i386-publication-layout.json').read_text())['classification'].startswith('PASS')
files={n:base64.b64encode((W/n).read_bytes()).decode() for n in ['card.json','authorization.json']};s='import os,json,base64,hashlib\nfrom pathlib import Path\nroot=Path('+repr(remote['tools'])+')\nfiles='+repr(files)+'\n'+'''receipt={}
assert root.is_dir() and os.stat(root).st_uid==1000 and (os.stat(root).st_mode&0o777)==0o700
for name,encoded in files.items():
 data=base64.b64decode(encoded,validate=True);fd=os.open(root/name,os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW,0o600)
 with os.fdopen(fd,'wb') as f:f.write(data);f.flush();os.fsync(f.fileno())
 assert (root/name).read_bytes()==data;receipt[name]={'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}
fd=os.open(root,os.O_DIRECTORY|os.O_RDONLY);os.fsync(fd);os.close(fd)
print(json.dumps(receipt))
''';c.once('exact-card-authorization-deployment',s,timeout=40)
s='import json,subprocess\np=subprocess.run('+repr(['python3','-B',remote['tools']+'/square_display_centered.py','--inspect'])+',capture_output=True,timeout=30)\nassert p.returncode==0 and not p.stderr\nr=json.loads(p.stdout)\nassert r["target"]=='+repr(card['target'])+'\nprint(json.dumps({"classification":"PASS FRESH EXACT DESTINATION CONTINUITY","target":r["target"],"read_only":True,"sgx_invocations":0}))\n';c.once('immediate-display-precheck',s,timeout=40);print('display-only card, authorization, and fresh destination continuity PASS; no display mutation')
