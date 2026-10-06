import os,stat,json,hashlib,base64
EVIDENCE='/root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c/evidence-83ee4ff8-a7f1-4468-b348-d10627f38d17'
PIN={'path': '/root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c/evidence-83ee4ff8-a7f1-4468-b348-d10627f38d17', 'dev': 2049, 'ino': 524773, 'uid': 0, 'gid': 0, 'mode': '0o700'}
record={'scope':'READ-ONLY RETRIEVAL OF ALREADY PRESERVED ORIGINALS; NO CLIENT/IOCTL','files':{},'errors':[]}
fd=os.open(EVIDENCE,os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW)
st=os.fstat(fd)
assert st.st_uid==0 and st.st_gid==0 and stat.S_IMODE(st.st_mode)==0o700 and (st.st_dev,st.st_ino)==(PIN['dev'],PIN['ino'])
for name in sorted(os.listdir(fd)):
 try:
  handle=os.open(name,os.O_RDONLY|os.O_NOFOLLOW|os.O_NONBLOCK,dir_fd=fd)
  try:
   before=os.fstat(handle);assert stat.S_ISREG(before.st_mode) and before.st_nlink==1 and before.st_uid==0 and before.st_gid==0 and before.st_size<=8*1024*1024
   data=bytearray()
   while True:
    chunk=os.read(handle,1048576)
    if not chunk:break
    data.extend(chunk)
   after=os.fstat(handle)
   assert (before.st_dev,before.st_ino,before.st_size,before.st_mtime_ns,before.st_ctime_ns,before.st_nlink)==(after.st_dev,after.st_ino,after.st_size,after.st_mtime_ns,after.st_ctime_ns,after.st_nlink) and len(data)==before.st_size
   record['files'][name]={'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'dev':before.st_dev,'ino':before.st_ino,'uid':before.st_uid,'gid':before.st_gid,'mode':stat.S_IMODE(before.st_mode),'nlink':before.st_nlink,'payload_base64':base64.b64encode(data).decode()}
  finally:os.close(handle)
 except BaseException as exc:record['errors'].append({'name':name,'error':repr(exc)})
os.close(fd)
with open('/proc/sys/kernel/random/boot_id') as b:record['retrieval_boot_id']=b.read().strip()
print(json.dumps(record,indent=2),flush=True)
