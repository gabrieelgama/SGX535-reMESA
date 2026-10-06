import json,base64
from pathlib import Path
p=Path('/home/gama/sgx535-display-FIRE3-video-20261006T071903Z-evidence')
files={}
for f in p.iterdir():
 assert f.is_file() and not f.is_symlink()
 assert f.stat().st_size<=262144
 files[f.name]=base64.b64encode(f.read_bytes()).decode()
print(json.dumps(files))
