/* SPDX-License-Identifier: MIT */
/* Pinned loader selection with interposed identity; zero physical operations. */
#include "remesa_dri.h"
#include "remesa_loader.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "remesa/native_uapi.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
const struct drm_driver_descriptor *remesa_test_find_driver(const char *);
static char directory[128];static unsigned mode,versions;static int duplicated=-1;
int __wrap_fstat(int fd,struct stat *s)
{
 (void)fd;memset(s,0,sizeof(*s));
 if(mode==1) {errno=EBADF;return -1;}
 s->st_mode=mode==2 ? S_IFREG : S_IFCHR;s->st_rdev=makedev(mode==3 ? 1 : 226,0);return 0;
}
int __real_open(const char *,int,...);
int __wrap_open(const char *path,int flags,...)
{ assert(!strcmp(path,"/sys/dev/char/226:0/device"));return __real_open(directory,flags); }
int __wrap_ioctl(int fd,unsigned long request,...)
{
 va_list ap;va_start(ap,request);struct drm_version *v=va_arg(ap,void *);va_end(ap);
 assert(request==DRM_IOCTL_VERSION);assert(fcntl(fd,F_GETFD)&FD_CLOEXEC);duplicated=fd;++versions;
 memcpy(v->name,mode==4 ? "other!" : "gma500",6);v->name_len=6;return 0;
}
static void file(const char *name,const char *contents)
{
 int dir=__real_open(directory,O_RDONLY|O_DIRECTORY);assert(dir>=0);
 int fd=openat(dir,name,O_CREAT|O_TRUNC|O_WRONLY,0600);assert(fd>=0);
 assert(write(fd,contents,strlen(contents))==(ssize_t)strlen(contents));close(fd);close(dir);
}
int main(void)
{
 strcpy(directory,"/tmp/remesa-dri-XXXXXX");assert(mkdtemp(directory));
 file("vendor","0x8086\n");file("device","0x8108\n");
 char temporary[]="/tmp/remesa-dri-fd-XXXXXX";int fd=mkstemp(temporary);assert(fd>=0);unlink(temporary);
 struct remesa_dri_identity id={123,456};
 assert(!remesa_dri_probe_fd(fd,&id)&&id.vendor==0x8086&&id.device==0x8108);
 for(mode=1;mode<=3;mode++) {
  id.vendor=123;assert(remesa_dri_probe_fd(fd,&id)==(mode==1 ? EBADF : ENODEV)&&id.vendor==123);
 }
 mode=0;file("device","0x8109\n");assert(remesa_dri_probe_fd(fd,&id)==ENODEV);
 file("device","0x8108x\n");assert(remesa_dri_probe_fd(fd,&id)==EPROTO);
 file("device","0x10000\n");assert(remesa_dri_probe_fd(fd,&id)==EPROTO);
 file("device","+0x8108\n");assert(remesa_dri_probe_fd(fd,&id)==EPROTO);
 file("device","");assert(remesa_dri_probe_fd(fd,&id)==EPROTO);file("device","0x8108\n");
 int dir=__real_open(directory,O_RDONLY|O_DIRECTORY);unlinkat(dir,"vendor",0);
 assert(remesa_dri_probe_fd(fd,&id)==ENOENT);file("vendor","0x8086\n");
 const struct drm_driver_descriptor *driver=remesa_test_find_driver("gma500");
 assert(remesa_test_find_driver("llvmpipe")!=driver);
 assert(driver==&gma500_driver_descriptor&&!strcmp(driver->driver_name,"gma500"));
 assert(!driver->driconf&&!driver->driconf_count&&!driver->probe_nctx);
 for(unsigned i=0;i<10;i++) {
  assert(!driver->create_screen(fd,NULL)&&errno==ENOTSUP);
  assert(fcntl(duplicated,F_GETFD)==-1&&errno==EBADF&&fcntl(fd,F_GETFD)>=0);
 }
 mode=4;assert(!driver->create_screen(fd,NULL)&&errno==ENODEV);mode=0;
 assert(strstr(remesa_dri_unavailable_reason(),"shader"));
 /* Real Gallium factory accepts optional state-tracker hints, not semantic
  * protected/compute/robust flags. This is not a real OpenGL context test. */
 int error;struct pipe_screen *s=remesa_loader_diagnostic_screen(fd,&error);assert(s);
 struct pipe_context *c=s->context_create(s,NULL,PIPE_CONTEXT_PREFER_THREADED|PIPE_CONTEXT_NO_LOD_BIAS);assert(c);c->destroy(c);
 assert(!s->context_create(s,NULL,PIPE_CONTEXT_PROTECTED));
 assert(!s->context_create(s,NULL,PIPE_CONTEXT_ROBUST_BUFFER_ACCESS));s->destroy(s);
 assert(versions==12);close(fd);unlinkat(dir,"vendor",0);unlinkat(dir,"device",0);close(dir);rmdir(directory);
 puts("PASS: official descriptor selection, PCI/sysfs rejection, 10 GL-admission cleanup cycles, no software fallback/draw, optional Gallium context hints; OpenGL remains unsupported");
}
