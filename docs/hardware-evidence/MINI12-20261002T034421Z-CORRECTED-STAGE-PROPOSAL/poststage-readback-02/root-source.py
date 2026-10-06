import os,stat,json,hashlib,subprocess
R='5.10.240-antix.1-486-smp'; ID='10d6abfa-e7a8-4311-97ae-213f503cee20'
paths={'kernel':('/boot/vmlinuz-'+R,5984416,'cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438'),'stock_initrd':('/boot/initrd.img-'+R,50863580,'f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341'),'cycle06_image':('/boot/initrd.img-'+R+'-sgx535-firstload-01',50804481,'4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71'),'corrected_image':('/boot/initrd.img-'+R+'-sgx535-firstload-corrected-01',50804478,'55e7a8be6f62c9a1d59f1c532b8790399c21a2706884747e504edfffb45bf52d'),'grub_cfg':('/boot/grub/grub.cfg',10438,'396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f'),'grubenv':('/boot/grub/grubenv',1024,'72c291233d508c8ed06305f0bf9d33130ea206bfaf67873dbe77413f77d19927'),'cycle06_custom_backup':('/boot/grub/custom.cfg.cycle06-preserved',974,'181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612'),'custom_cfg':('/boot/grub/custom.cfg',2040,'269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8')}
def ident(path,size,want):
 s=os.lstat(path);assert stat.S_ISREG(s.st_mode) and not stat.S_ISLNK(s.st_mode) and s.st_nlink==1
 fd=os.open(path,os.O_RDONLY|os.O_NOFOLLOW);b=b''
 while True:
  x=os.read(fd,1048576)
  if not x:break
  b+=x
 os.close(fd);h=hashlib.sha256(b).hexdigest()
 return {'size':len(b),'sha256':h,'uid':s.st_uid,'gid':s.st_gid,'mode':oct(stat.S_IMODE(s.st_mode)),'nlink':s.st_nlink,'pass':len(b)==size and h==want}
r={'boot_id':open('/proc/sys/kernel/random/boot_id').read().strip(),'kernel':os.uname().release,'arch':os.uname().machine,'cmdline':open('/proc/cmdline').read().strip(),'taint':open('/proc/sys/kernel/tainted').read().strip(),'loaded_module_note_sha256':hashlib.sha256(open('/sys/module/gma500_gfx/notes/.note.gnu.build-id','rb').read()).hexdigest(),'module_state':open('/sys/module/gma500_gfx/initstate').read().strip(),'pci_driver':os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver'),'drm_device':os.path.realpath('/sys/class/drm/card0/device'),'framebuffer':open('/sys/class/graphics/fb0/name').read().strip(),'destinations':{k:ident(*v) for k,v in paths.items()}}
assert r['boot_id']==ID and r['kernel']==R and r['arch']=='i686' and r['taint']=='12289' and r['module_state']=='live'
assert r['pci_driver']=='/sys/bus/pci/drivers/gma500' and r['drm_device']=='/sys/devices/pci0000:00/0000:00:02.0' and r['framebuffer']=='gma500drmfb'
assert all(x['pass'] for x in r['destinations'].values())
for k in ('corrected_image','cycle06_custom_backup','custom_cfg'):
 x=r['destinations'][k];assert x['uid']==0 and x['gid']==0 and x['mode']=='0o644', 'staged file metadata '+k
old=open('/boot/grub/custom.cfg.cycle06-preserved','rb').read();cfg=open('/boot/grub/custom.cfg','rb').read();assert cfg.startswith(old) and cfg.count(b'menuentry ')==2
assert cfg.count(b"EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)")==1
assert cfg.count(b"EXPERIMENTAL SGX535 rev121 CORRECTED FIRST-LOAD ONLY (no triangle)")==1
assert cfg.count(b"sgx535-rev121-firstload-corrected-01")==1 and cfg.count(b"/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-corrected-01")==1
assert b'savedefault' not in cfg and b'save_env' not in cfg
assert not os.path.lexists('/boot/grub/.custom.cfg.corrected-01.pending')
genv=subprocess.run(['grub-editenv','/boot/grub/grubenv','list'],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,check=True).stdout.strip();assert genv=='saved_entry=gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1'
r['custom_cfg_structure']={'old_exact_prefix':True,'entries':2,'cycle06_entry':True,'corrected_entry':True,'savedefault':False,'save_env':False,'saved_stock_default':True,'pending_temp_absent':True}
r['kernel_log']=subprocess.run(['dmesg'],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,check=True).stdout
print(json.dumps(r))
