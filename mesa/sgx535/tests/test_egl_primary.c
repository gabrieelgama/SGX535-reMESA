/* SPDX-License-Identifier: MIT */
/* Interposed identity fixtures; no physical DRM access or GPU execution. */
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <xf86drm.h>
static unsigned mode,opens,devices,versions,free_devices,free_versions;
static int backing;
static char *nodes[DRM_NODE_MAX]={"/fixture/primary",NULL,NULL};
static drmPciDeviceInfo pci={.vendor_id=0x8086,.device_id=0x8108};
static drmDevice device={.nodes=nodes,.available_nodes=1<<DRM_NODE_PRIMARY,
 .bustype=DRM_BUS_PCI,.deviceinfo={.pci=&pci}};
static drmVersion version={.name_len=6,.name="gma500"};
static int mock_stat(int fd,struct stat *st)
{
 assert(fd!=backing && (fcntl(fd,F_GETFD)&FD_CLOEXEC));
 if(mode==19) return -1;
 memset(st,0,sizeof(*st));st->st_mode=mode==1 ? S_IFREG : S_IFCHR;
 st->st_rdev=makedev(mode==2 ? 1 : 226,0);return 0;
}
static int mock_node(int fd) {(void)fd;return mode==3 ? DRM_NODE_RENDER : DRM_NODE_PRIMARY;}
static int mock_device(int fd,uint32_t flags,drmDevicePtr *out)
{
 assert(fd>=0 && flags==0);++devices;
 if(mode==4) return -EINVAL;
 *out=&device;return 0;
}
static drmVersionPtr mock_version(int fd)
{assert(fd>=0);++versions;return mode==5 ? NULL : &version;}
static void mock_free_version(drmVersionPtr v) {assert(v==&version);++free_versions;}
static void mock_free_device(drmDevicePtr *d) {assert(*d==&device);*d=NULL;++free_devices;}
static int mock_equal(drmDevicePtr a,drmDevicePtr b)
{assert(a==&device && b==&device);return mode!=18;}
static int mock_open(const char *path,int flags)
{
 assert(!strcmp(path,"/fixture/primary") && flags==(O_RDWR|O_CLOEXEC));++opens;
 if(mode==17) return -1;
 return fcntl(backing,F_DUPFD_CLOEXEC,0);
}
#define fstat mock_stat
#define drmGetNodeTypeFromFd mock_node
#define drmGetDevice2 mock_device
#define drmGetVersion mock_version
#define drmFreeVersion mock_free_version
#define drmFreeDevice mock_free_device
#define drmDevicesEqual mock_equal
#define open mock_open
#include "remesa_primary_device.h"
#undef fstat
#undef open
static unsigned fd_count(void)
{DIR *d=opendir("/proc/self/fd");assert(d);unsigned n=0;while(readdir(d)) ++n;closedir(d);return n;}
static void reset(unsigned m)
{
 mode=m;device.bustype=DRM_BUS_PCI;device.deviceinfo.pci=&pci;
 device.available_nodes=1<<DRM_NODE_PRIMARY;device.nodes=nodes;nodes[0]="/fixture/primary";
 pci.vendor_id=0x8086;pci.device_id=0x8108;version.name="gma500";version.name_len=6;
 devices=versions=free_devices=free_versions=opens=0;
 if(m==6) device.bustype=DRM_BUS_USB;
 if(m==7) device.deviceinfo.pci=NULL;
 if(m==8) pci.vendor_id=0x1234;
 if(m==9) pci.device_id=0x8109;
 if(m==10) device.available_nodes=0;
 if(m==11) device.available_nodes|=1<<DRM_NODE_RENDER;
 if(m==12) device.nodes=NULL;
 if(m==13) nodes[0]=NULL;
 if(m==14) version.name="other!";
 if(m==15) version.name=NULL;
 if(m==16) version.name_len=7;
}
int main(void)
{
 char path[]="/tmp/remesa-egl-primary-XXXXXX";backing=mkstemp(path);assert(backing>=0);
 unsigned count=fd_count();reset(0);assert(remesa_egl_primary_fd(backing));
 assert(devices==1 && free_devices==1 && versions==1 && free_versions==1 && !opens && fd_count()==count);
 for(unsigned m=1;m<=16;m++) {
  reset(m);assert(!remesa_egl_primary_fd(backing));
  assert(free_devices==(devices && m!=4 ? 1u : 0u));
  assert(free_versions==(versions && m!=5 ? 1u : 0u));
  assert(!opens && fd_count()==count && fcntl(backing,F_GETFD)>=0);
 }
 reset(19);assert(!remesa_egl_primary_fd(backing) && fd_count()==count);
 reset(0);assert(!remesa_egl_primary_fd(-1));
 int ro=openat(AT_FDCWD,path,O_RDONLY|O_CLOEXEC);assert(ro>=0);
 assert(!remesa_egl_primary_fd(ro));close(ro);
 reset(0);assert(remesa_egl_primary_device(&device));assert(opens==1 && fd_count()==count && free_devices==2 && free_versions==1);
 reset(17);assert(!remesa_egl_primary_device(&device) && opens==1 && fd_count()==count);
 reset(18);assert(!remesa_egl_primary_device(&device) && free_devices==2 && fd_count()==count);
 reset(8);assert(!remesa_egl_primary_device(&device) && !opens && fd_count()==count);
 reset(0);assert(!remesa_egl_primary_device(NULL) && !opens);
 assert(!remesa_egl_primary_metadata(NULL));
 close(backing);unlink(path);
 puts("PASS: 26 primary-only EGL discovery cases; exact PCI/DRM identity, CLOEXEC ownership, malformed/foreign rejection and zero leaked fds; interposed fixtures, no rendering");
}
