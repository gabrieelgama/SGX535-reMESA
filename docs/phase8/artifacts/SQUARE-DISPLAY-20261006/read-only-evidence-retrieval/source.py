import json,base64,os,stat,hashlib
from pathlib import Path
p=Path('/home/gama/sgx535-square-display-20261006T090214Z-evidence')
st=os.lstat(p);assert stat.S_ISDIR(st.st_mode) and st.st_uid==1000 and (st.st_mode&0o777)==0o700
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
