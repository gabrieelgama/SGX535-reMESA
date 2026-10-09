/* SPDX-License-Identifier: MIT */
/* SIMULATED transport responses, never hardware completion evidence. */
#include "remesa/native_uapi.h"
#define CHECK(condition) do { if (!(condition)) abort(); } while (0)
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int __real_open64(const char *,int,...);
int __real_close(int);
int __wrap_open64(const char *,int,...);
int __wrap_close(int);
int __wrap_ioctl(int,unsigned long,...);
static unsigned submissions;
int __wrap_open64(const char *path,int flags,...)
{
 if(!strcmp(path,"SIMULATED-NO-DEVICE"))return 12345;
 mode_t mode=0;if(flags&O_CREAT){va_list ap;va_start(ap,flags);mode=va_arg(ap,int);va_end(ap);}
 return __real_open64(path,flags,mode);
}
int __wrap_close(int fd){return fd==12345?0:__real_close(fd);}
int __wrap_ioctl(int fd,unsigned long op,...)
{
 CHECK(fd==12345);va_list ap;va_start(ap,op);void *arg=va_arg(ap,void *);va_end(ap);
 if(op==DRM_IOCTL_VERSION){struct drm_version *v=arg;memcpy(v->name,"gma500",6);v->name_len=6;return 0;}
 CHECK(op==DRM_IOCTL_REMESA_NATIVE_DRAW && ++submissions==1);
 struct remesa_native_draw *r=arg;
 CHECK(r->argb==0xffff00ff && r->vertex_count==3 && r->index_count==3);
 const char *mode=getenv("REMESA_SIMULATED_OUTCOME");CHECK(mode);
 fprintf(stderr,"SIMULATED_TRANSPORT_ONLY: submissions=%u outcome=%s; NO GPU\n",submissions,mode);
 if(!strcmp(mode,"interrupted")){errno=EINTR;return -1;}
 r->operation=1;r->phase=9;r->events=7;r->issued=1;r->published=1;r->status=REMESA_NATIVE_STATUS_COMPLETED;
 int ref=__real_open64(getenv("REMESA_SIMULATED_REFERENCE"),O_RDONLY);CHECK(ref>=0);
 CHECK(read(ref,r->color,sizeof(r->color))==(ssize_t)sizeof(r->color));__real_close(ref);
 if(!strcmp(mode,"pixel-mismatch"))r->color[0]^=1;
 if(!strcmp(mode,"missing-completion"))r->events=3;
 if(!strcmp(mode,"wrong-operation"))r->operation=2;
 return !strcmp(mode,"positive-ioctl")?1:0;
}
