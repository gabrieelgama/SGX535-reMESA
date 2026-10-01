set -eu
export PYTHONDONTWRITEBYTECODE=1
command -v python3 >/dev/null
exec python3 -I -B -S - <<'PYTHON'
import os,sys,stat,json,hashlib,re,subprocess,datetime
RELEASE='5.10.240-antix.1-486-smp'
BDF='/sys/bus/pci/devices/0000:00:02.0'
STOCK={'kernel':('/boot/vmlinuz-'+RELEASE,5984416,'cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438'),'initrd':('/boot/initrd.img-'+RELEASE,50863580,'f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341'),'grub_cfg':('/boot/grub/grub.cfg',10438,'396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f'),'grub_env':('/boot/grub/grubenv',1024,'72c291233d508c8ed06305f0bf9d33130ea206bfaf67873dbe77413f77d19927'),'original_module':('/lib/modules/'+RELEASE+'/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko',None,'7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb')}
STOCK_ID='gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1'
result={'start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'scope':'root read-only pre-stage stock/boot/ownership/filesystem/tool checks; no writes, no DRM open, no module/service operations','commands':[],'guards':[]}
def read(path):
 with open(path,'rb') as f:data=f.read()
 result.setdefault('read_paths',[]).append(path)
 return data
def txt(path):return read(path).decode().strip()
def check(condition,name):
 result['guards'].append({'guard':name,'pass':bool(condition)})
 if not condition:raise RuntimeError(name)
def run(argv):
 p=subprocess.run(argv,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
 row={'argv':argv,'exit_code':p.returncode,'stdout':p.stdout.decode(errors='replace'),'stderr':p.stderr.decode(errors='replace')};result['commands'].append(row)
 check(p.returncode==0 and not p.stderr,'read-only command '+str(argv))
 return row['stdout']
def file_record(path):
 st=os.lstat(path);check(stat.S_ISREG(st.st_mode),'regular stock file '+path)
 data=read(path);return {'path':path,'size':len(data),'sha256':hashlib.sha256(data).hexdigest(),'type':'regular','symlink':False,'uid':st.st_uid,'gid':st.st_gid,'mode':stat.S_IMODE(st.st_mode),'nlink':st.st_nlink,'dev':st.st_dev,'ino':st.st_ino}
def trusted(path):
 current='/'
 for part in path.strip('/').split('/'):
  current=os.path.join(current,part);st=os.lstat(current)
  check(stat.S_ISDIR(st.st_mode) and st.st_uid==0 and not stat.S_IMODE(st.st_mode)&0o022,'trusted directory '+current)
try:
 result['euid']=os.geteuid();check(os.geteuid()==0,'root capture')
 result['kernel']=run(['uname','-r']).strip();check(result['kernel']==RELEASE,'kernel release')
 result['architecture']=run(['uname','-m']).strip();check(result['architecture']=='i686','architecture')
 result['machine']=txt('/sys/class/dmi/id/product_name');check(result['machine']=='Inspiron 1210','DMI machine')
 result['proc_version']=txt('/proc/version');result['cmdline']=txt('/proc/cmdline')
 check(result['cmdline'].split()==('BOOT_IMAGE=/boot/vmlinuz-'+RELEASE+' root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro quiet selinux=0').split(),'stock command line')
 result['boot_id']=txt('/proc/sys/kernel/random/boot_id');result['taint']=int(txt('/proc/sys/kernel/tainted'));check(result['taint']==12289,'stock taint baseline')
 result['pci']={k:txt(BDF+'/'+k) for k in ['vendor','device','subsystem_vendor','subsystem_device','irq']}
 check(result['pci']=={'vendor':'0x8086','device':'0x8108','subsystem_vendor':'0x1028','subsystem_device':'0x02b1','irq':'16'},'PCI identity/IRQ')
 result['pci_driver']=os.path.realpath(BDF+'/driver');result['driver_module']=os.path.realpath(BDF+'/driver/module');check(result['pci_driver']=='/sys/bus/pci/drivers/gma500' and result['driver_module']=='/sys/module/gma500_gfx','original PCI ownership')
 result['module_state']=txt('/sys/module/gma500_gfx/initstate');check(result['module_state']=='live','original module Live')
 result['modules']=txt('/proc/modules');check(re.search(r'^gma500_gfx .* Live ',result['modules'],re.M) is not None,'module list Live')
 result['loaded_note_sha256']=hashlib.sha256(read('/sys/module/gma500_gfx/notes/.note.gnu.build-id')).hexdigest()
 expected_note=hashlib.sha256(bytes.fromhex('040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3')).hexdigest();check(result['loaded_note_sha256']==expected_note,'original loaded Build-ID note')
 result['drm_bdf']=os.path.basename(os.path.realpath('/sys/class/drm/card0/device'));check(result['drm_bdf']=='0000:00:02.0','DRM device ownership')
 st=os.lstat('/dev/dri/card0');result['drm_node']={'mode':st.st_mode,'major':os.major(st.st_rdev),'minor':os.minor(st.st_rdev)};check(stat.S_ISCHR(st.st_mode),'DRM node metadata only')
 result['proc_fb']=txt('/proc/fb');result['framebuffer']=txt('/sys/class/graphics/fb0/name');result['framebuffer_dimensions']=txt('/sys/class/graphics/fb0/virtual_size');check(result['framebuffer']=='gma500drmfb' and result['framebuffer_dimensions']=='1280,800','stock framebuffer')
 result['vtcon0']=int(txt('/sys/class/vtconsole/vtcon0/bind'));result['vtcon1']=int(txt('/sys/class/vtconsole/vtcon1/bind'));check(result['vtcon0']==0 and result['vtcon1']==1,'stock VT ownership')
 result['interrupts']=txt('/proc/interrupts');check(re.search(r'^\s*16:.*[\s,]gma500(?:[,\s]|$)',result['interrupts'],re.M) is not None,'IRQ16 gma500 handler')
 result['slimski_status']=run(['sv','status','/etc/runit/runsvdir/default/slimski']);check(result['slimski_status'].startswith('run:'),'slimski running')
 result['xorg_processes']=run(['pgrep','-a','Xorg']);check(bool(result['xorg_processes'].strip()),'Xorg running')
 result['kernel_log']=run(['dmesg']);check(re.search(r'WARNING:|BUG:|Oops:|Kernel panic|general protection fault|Call Trace:',result['kernel_log'],re.I) is None,'current-boot kernel health')
 result['stock']={}
 for key,(path,size,digest) in STOCK.items():
  row=file_record(path);result['stock'][key]=row;check(row['sha256']==digest and (size is None or row['size']==size),'stock bytes '+key)
 result['grub_env']=run(['grub-editenv','/boot/grub/grubenv','list']);check(result['grub_env'].strip()=='saved_entry='+STOCK_ID,'saved stock selection only')
 cfg=read('/boot/grub/grub.cfg').decode();check(STOCK_ID in cfg and 'custom.cfg' in cfg,'stock entry and custom.cfg source retained')
 result['destinations']={}
 for path in ['/boot/initrd.img-'+RELEASE+'-sgx535-firstload-01','/boot/grub/custom.cfg','/home/gama/sgx535-firstload-incoming-01']:
  result['destinations'][path]={'absent':not os.path.lexists(path)};check(not os.path.lexists(path),'staging destination absent '+path)
 trusted('/boot');trusted('/boot/grub')
 result['mountinfo']=txt('/proc/self/mountinfo');result['mounts']=txt('/proc/mounts');result['boot_free_bytes']=os.statvfs('/boot').f_bavail*os.statvfs('/boot').f_frsize;result['incoming_free_bytes']=os.statvfs('/home/gama').f_bavail*os.statvfs('/home/gama').f_frsize
 check(result['boot_free_bytes']>=134217728 and result['incoming_free_bytes']>=50804481+974,'staging free space')
 check(os.access('/boot',os.W_OK) and not (os.statvfs('/boot').f_flag&getattr(os,'ST_RDONLY',1)),'boot filesystem writable')
 result['gama_uid']=int(run(['id','-u','gama']).strip());result['gama_gid']=int(run(['id','-g','gama']).strip())
 result['python']=sys.version;result['python_executable']=sys.executable
 check(all(hasattr(os,k) for k in ['O_NOFOLLOW','O_EXCL','O_DIRECTORY','fsync','fchmod','fchown']),'exclusive no-follow/fsync facilities')
 check(txt('/proc/sys/kernel/random/boot_id')==result['boot_id'],'same boot at capture end')
 result['end_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();result['classification']='PASS READ-ONLY MACHINE GUARDS; operator prerequisites pending'
 print(json.dumps(result,indent=2))
except Exception as exc:
 result['classification']='STOP BEFORE STAGING';result['failure']=str(exc);print(json.dumps(result,indent=2));sys.exit(1)

PYTHON
