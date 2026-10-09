/* SPDX-License-Identifier: MIT */
/* Interposed ioctl only. Mock response is NOT physical completion evidence. */
#include "remesa_retained_transport.h"
#include <assert.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
static unsigned calls,records,scenario;
int __wrap_ioctl(int fd,unsigned long request,...)
{
 va_list ap;va_start(ap,request);struct remesa_retained_request *r=va_arg(ap,void *);va_end(ap);
 assert(fd==23 && request==DRM_IOCTL_REMESA_RETAINED);calls++;
 if(scenario==1){errno=EINTR;return -1;}
 r->operation=11;r->serial=12;r->generation=1;r->state=6;r->admitted=1;r->observer_request_bytes=1;r->observer_response_bytes=1;
 r->observer_request[0]=0xa1;r->observer_response[0]=0xb1;
 if(scenario==2)r->context++;
 if(scenario==3)r->attached=1;
 if(scenario==4)r->state=5;
 if(scenario==5)r->generation=0;
 if(scenario==6)r->error=-EIO;
 if(scenario==8)r->observer_request_bytes=sizeof(r->observer_request)+1;
 if(scenario==9)r->observer_response[1]=1;
 if(scenario==10)r->observer_response_bytes=0;
 return 0;
}
static int capture(void *c,const struct remesa_retained_request *before,
 const struct remesa_retained_request *after,int rc,int error)
{(void)c;assert(before->context==7);records++;if(after){assert(records==2);if(scenario==1)assert(rc==-1 && error==EINTR);}return scenario==7 && after?EIO:0;}
int main(void)
{
 for(unsigned profile=0;profile<2;profile++)for(scenario=0;scenario<11;scenario++){
  struct remesa_retained_request r={0};r.version=profile?2:1;r.flags=profile?1:0;r.context=7;bool attempted=false;calls=records=0;
  for(unsigned i=0;i<10;i++){r.bindings[i].id=32+i;r.bindings[i].bytes=4096;r.bindings[i].va=0x40000000+i*4096;}
  struct remesa_retained_request bad=r;bad.bindings[1].id=bad.bindings[0].id;
  assert(remesa_retained_submit_once(23,&bad,&attempted,capture,NULL)==EINVAL && !attempted && !calls);
  bad=r;bad.flags^=1;
  assert(remesa_retained_submit_once(23,&bad,&attempted,capture,NULL)==EINVAL && !attempted && !calls);
  bad=r;bad.version=3;
  assert(remesa_retained_submit_once(23,&bad,&attempted,capture,NULL)==EINVAL && !attempted && !calls);
  int e=remesa_retained_submit_once(23,&r,&attempted,capture,NULL);
  assert(e==(scenario?EOWNERDEAD:0) && attempted && calls==1 && records==2);
  assert(remesa_retained_submit_once(23,&r,&attempted,capture,NULL)==EALREADY && calls==1);
 }
 puts("PASS retained transport: 22 V1/V2 interposed responses, strict profile flags, exact records, no retry; no hardware");return 0;
}
