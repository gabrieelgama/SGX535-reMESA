/* SPDX-License-Identifier: MIT */
/* Pinned loader selection with interposed identity; zero physical operations. */
#include "remesa_dri.h"
#include "remesa_loader.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "remesa/native_uapi.h"
#include <assert.h>
#include <errno.h>
#include <dirent.h>
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
#ifdef REMESA_DRI_TEST_ALLOC_FAILURE
/* Linker interception applies only to this test executable. Failure records
 * remain simulated; no production allocation override is compiled. */
static unsigned allocation_failure;
void *__real_calloc(size_t,size_t);
void *__wrap_calloc(size_t count,size_t size)
{
 if(allocation_failure && !--allocation_failure) {errno=ENOMEM;return NULL;}
 return __real_calloc(count,size);
}
#endif
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
 if(mode==5) {errno=EIO;return -1;}
 if(mode==6) return 1;
 memcpy(v->name,mode==4 ? "other!" : "gma500",6);v->name_len=6;return 0;
}
static unsigned fd_count(void)
{
 DIR *dir=opendir("/proc/self/fd");assert(dir);unsigned count=0;
 while(readdir(dir)) ++count;
 assert(!closedir(dir));return count;
}
static void rejected(int fd,enum remesa_dri_stage stage,int error,
                     bool identity,bool transport,bool screen)
{
 unsigned before=fd_count();struct remesa_dri_report report;
 memset(&report,0xff,sizeof(report));
 assert(!remesa_dri_create_screen(fd,NULL,&report));
 assert(errno==error && report.error==error && report.stage==stage);
 assert(report.identity_validated==identity && report.transport_validated==transport && report.screen_created==screen);
 assert(report.missing_contracts==(stage==REMESA_DRI_OPENGL_ADMISSION ? remesa_dri_missing_contracts() : 0));
 if(identity) assert(report.identity.vendor==0x8086 && report.identity.device==0x8108);
 assert(before==fd_count());
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
 /* Attribute the first failing function, not a generic EGL failure or global
  * last error. All fd identities here are interposed; none prove hardware. */
 rejected(-1,REMESA_DRI_RETAIN_FD,EBADF,false,false,false);
 assert(!remesa_dri_create_screen(fd,NULL,NULL) && errno==EINVAL);
 for(mode=1;mode<=3;mode++)
  rejected(fd,REMESA_DRI_PCI_IDENTITY,mode==1 ? EBADF : ENODEV,false,false,false);
 mode=0;file("device","0x8108x\n");
 rejected(fd,REMESA_DRI_PCI_IDENTITY,EPROTO,false,false,false);
 file("device","0x8109\n");rejected(fd,REMESA_DRI_PCI_IDENTITY,ENODEV,false,false,false);
 file("device","0x8108\n");
 for(mode=4;mode<=6;mode++)
  rejected(fd,REMESA_DRI_DRM_TRANSPORT,mode==4 ? ENODEV : mode==5 ? EIO : EPROTO,true,false,false);
 mode=0;
 char readonly[]="/tmp/remesa-dri-readonly-XXXXXX";int rw=mkstemp(readonly);assert(rw>=0);close(rw);
 int ro=__real_open(readonly,O_RDONLY|O_CLOEXEC);assert(ro>=0);unlink(readonly);
 rejected(ro,REMESA_DRI_DRM_TRANSPORT,EACCES,true,false,false);assert(fcntl(ro,F_GETFD)>=0);close(ro);
 struct remesa_dri_report first,second;struct pipe_screen_config config={0};
 assert(!remesa_dri_create_screen(fd,&config,&first) && errno==ENOTSUP);
 assert(first.stage==REMESA_DRI_OPENGL_ADMISSION && first.screen_created && first.transport_validated && first.identity_validated);
 assert(first.missing_contracts & REMESA_DRI_MISSING_TEXTURE_SAMPLING);
 assert(first.missing_contracts & REMESA_DRI_MISSING_COLOR_PRESERVATION);
 assert(!remesa_dri_create_screen(-1,NULL,&second));
 assert(first.stage==REMESA_DRI_OPENGL_ADMISSION && first.error==ENOTSUP);
 assert(second.stage==REMESA_DRI_RETAIN_FD && second.error==EBADF);
 assert(!strcmp(remesa_dri_stage_name(first.stage),"opengl-admission"));
 assert(!strcmp(remesa_dri_stage_name((enum remesa_dri_stage)99),"invalid-stage"));
#ifdef REMESA_DRI_TEST_ALLOC_FAILURE
 allocation_failure=1;
 rejected(fd,REMESA_DRI_DRM_TRANSPORT,ENOMEM,true,false,false);
 assert(!allocation_failure && fcntl(fd,F_GETFD)>=0);
 allocation_failure=2;
 rejected(fd,REMESA_DRI_GALLIUM_SCREEN,ENOMEM,true,true,false);
 assert(!allocation_failure && fcntl(fd,F_GETFD)>=0);
#endif
 for(unsigned i=0;i<10;i++) rejected(fd,REMESA_DRI_OPENGL_ADMISSION,ENOTSUP,true,true,true);
 assert(!setenv("REMESA_DRI_TRACE","1",1));
 for(unsigned i=0;i<10;i++) {
  assert(!driver->create_screen(fd,NULL)&&errno==ENOTSUP);
  assert(fcntl(duplicated,F_GETFD)==-1&&errno==EBADF&&fcntl(fd,F_GETFD)>=0);
 }
 mode=4;assert(!driver->create_screen(fd,NULL)&&errno==ENODEV);mode=0;
 assert(!unsetenv("REMESA_DRI_TRACE"));
 assert(strstr(remesa_dri_unavailable_reason(),"shader"));
 /* Real Gallium factory accepts optional state-tracker hints, not semantic
  * protected/compute/robust flags. This is not a real OpenGL context test. */
 int error;struct pipe_screen *s=remesa_loader_diagnostic_screen(fd,&error);assert(s);
 struct pipe_context *c=s->context_create(s,NULL,PIPE_CONTEXT_PREFER_THREADED|PIPE_CONTEXT_NO_LOD_BIAS);assert(c);c->destroy(c);
 assert(!s->context_create(s,NULL,PIPE_CONTEXT_PROTECTED));
 assert(!s->context_create(s,NULL,PIPE_CONTEXT_ROBUST_BUFFER_ACCESS));s->destroy(s);
 assert(versions==26
#ifdef REMESA_DRI_TEST_ALLOC_FAILURE
        +2
#endif
 );close(fd);unlinkat(dir,"vendor",0);unlinkat(dir,"device",0);close(dir);rmdir(directory);
 puts("PASS: actual descriptor diagnostics distinguish retain/PCI/DRM/OpenGL admission; 20 admission cleanup cycles, zero fd leaks, no software fallback/draw; OpenGL remains unsupported");
}
