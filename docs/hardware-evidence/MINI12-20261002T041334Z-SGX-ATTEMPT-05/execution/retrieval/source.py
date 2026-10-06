import os,stat,json,hashlib,base64,datetime
BASE="/root/sgx535-frozen-seq1-203a5b9b"
out={"scope":"read-only retrieval of one-shot wrapper evidence only; no DRM/SGX access","start_utc":datetime.datetime.now(datetime.timezone.utc).isoformat()}
dfd=os.open(BASE,os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW); ds=os.fstat(dfd);assert ds.st_uid==0 and stat.S_IMODE(ds.st_mode)==0o700
out["directory"]={"uid":ds.st_uid,"mode":stat.S_IMODE(ds.st_mode),"dev":ds.st_dev,"ino":ds.st_ino};out["files"]={}
for name in ("before.json","attempt-claimed","client.stdout","client.stderr","result.json","color.bin"):
 try:
  fd=os.open(name,os.O_RDONLY|os.O_NOFOLLOW,dir_fd=dfd)
 except FileNotFoundError:
  out["files"][name]={"present":False};continue
 st=os.fstat(fd);data=b""
 with os.fdopen(fd,"rb") as f:data=f.read()
 assert stat.S_ISREG(st.st_mode) and st.st_uid==0 and st.st_nlink==1 and stat.S_IMODE(st.st_mode)==0o600
 out["files"][name]={"present":True,"size":len(data),"sha256":hashlib.sha256(data).hexdigest(),"uid":st.st_uid,"mode":stat.S_IMODE(st.st_mode),"dev":st.st_dev,"ino":st.st_ino,"base64":base64.b64encode(data).decode()}
out["end_utc"]=datetime.datetime.now(datetime.timezone.utc).isoformat();print(json.dumps(out,separators=(",",":")))
