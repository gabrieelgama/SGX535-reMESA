import os,stat,json,hashlib,pwd,datetime
R='5.10.240-antix.1-486-smp'
BOOT='d78d349e-daac-43aa-b7f9-156485506ca5'
paths=['/boot/initrd.img-'+R+'-sgx535-firstload-diagnostic-01','/boot/grub/custom.cfg.pre-diagnostic-01','/boot/grub/.custom.cfg.diagnostic-01.pending','/home/gama/sgx535-firstload-diagnostic-incoming-01']
result={'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'guards':[],'paths':{}}
def check(name,ok,detail=None):result['guards'].append({'name':name,'pass':bool(ok),'detail':detail})
try:
 boot=open('/proc/sys/kernel/random/boot_id').read().strip();result['boot_id']=boot;check('same STOCK boot',boot==BOOT)
 check('expected architecture',os.uname().machine=='i686' and os.uname().release==R)
 check('stock module live',open('/sys/module/gma500_gfx/initstate').read().strip()=='live')
 check('stock PCI owner',os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver')=='/sys/bus/pci/drivers/gma500')
 for p in ['/boot','/boot/grub']:
  st=os.lstat(p);check('trusted parent '+p,stat.S_ISDIR(st.st_mode) and st.st_uid==0 and st.st_mode&0o022==0,{'uid':st.st_uid,'mode':oct(stat.S_IMODE(st.st_mode)),'dev':st.st_dev})
 st=os.lstat('/home/gama');uid=pwd.getpwnam('gama').pw_uid;gid=pwd.getpwnam('gama').pw_gid
 result['gama_identity']={'uid':uid,'gid':gid};check('target user identity',uid==1000 and gid==1000)
 check('target home directory',stat.S_ISDIR(st.st_mode) and st.st_uid==uid and st.st_mode&0o022==0,{'uid':st.st_uid,'mode':oct(stat.S_IMODE(st.st_mode))})
 for p in paths:
  try:st=os.lstat(p);result['paths'][p]={'absent':False,'mode':oct(stat.S_IMODE(st.st_mode))}
  except FileNotFoundError:result['paths'][p]={'absent':True}
  check('new path absent '+p,result['paths'][p]['absent'])
 for p,minimum in [('/boot',134217728+50805273),('/home/gama',50805273+16777216)]:
  v=os.statvfs(p);free=v.f_bavail*v.f_frsize;result['free_bytes_'+p]=free
  check('free space '+p,free>=minimum,{'free':free,'minimum':minimum})
  check('writable filesystem '+p,not(v.f_flag&getattr(os,'ST_RDONLY',1)))
 st=os.lstat('/boot/grub/custom.cfg');check('current custom.cfg regular root single-link',stat.S_ISREG(st.st_mode) and st.st_nlink==1 and st.st_uid==0 and st.st_gid==0 and stat.S_IMODE(st.st_mode)==0o644)
 with open('/boot/grub/custom.cfg','rb') as f:data=f.read()
 result['current_custom_cfg_sha256']=hashlib.sha256(data).hexdigest();check('current two-entry config',len(data)==2040 and result['current_custom_cfg_sha256']=='269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8')
except Exception as e:result['guards'].append({'name':'capture completed','pass':False,'detail':repr(e)})
result['classification']='PASS' if all(x['pass'] for x in result['guards']) else 'BLOCKED'
result['pass_count']=sum(x['pass'] for x in result['guards']);result['guard_count']=len(result['guards'])
print(json.dumps(result,indent=2,sort_keys=True))
