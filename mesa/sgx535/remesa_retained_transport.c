/* SPDX-License-Identifier: MIT */
#include "remesa_retained_transport.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
int remesa_retained_request_validate(const struct remesa_retained_request *r)
{
 if(!r)return EINVAL;
 bool seeded=r->version==REMESA_RETAINED_SEEDED_VERSION && r->flags==REMESA_RETAINED_SEEDED_FLAG;
 if((!seeded && (r->version!=REMESA_RETAINED_VERSION || r->flags)) || !r->context ||
    r->reserved[0] || r->reserved[1] || r->operation || r->serial || r->slot ||
    r->generation || r->state || r->error || r->admitted || r->attached ||
    r->observer_request_bytes || r->observer_response_bytes)return EINVAL;
 for(unsigned i=0;i<sizeof(r->observer_request);i++)if(r->observer_request[i])return EINVAL;
 for(unsigned i=0;i<sizeof(r->observer_response);i++)if(r->observer_response[i])return EINVAL;
 for(unsigned i=0;i<10;i++){
  if(!r->bindings[i].id || !(r->bindings[i].id>>5) || r->bindings[i].reserved ||
     !r->bindings[i].bytes || r->bindings[i].bytes%4096 || r->bindings[i].va%4096 ||
     !r->bindings[i].va || r->bindings[i].va>=UINT64_C(0x100000000) ||
     r->bindings[i].bytes>UINT64_C(0x100000000)-r->bindings[i].va)return EINVAL;
  for(unsigned j=0;j<i;j++)if(r->bindings[j].id==r->bindings[i].id ||
    (r->bindings[j].va<r->bindings[i].va+r->bindings[i].bytes &&
     r->bindings[i].va<r->bindings[j].va+r->bindings[j].bytes))return EINVAL;
 }
 return 0;
}
int remesa_retained_submit_once(int fd,struct remesa_retained_request *r,bool *attempted,
 remesa_retained_recorder record,void *context)
{
 struct remesa_retained_request before;int rc,e;
 if(!r || !attempted || !record || fd<0)return EINVAL;
 if(*attempted)return EALREADY;
 e=remesa_retained_request_validate(r);if(e)return e;
 before=*r;if(record(context,&before,NULL,0,0))return EIO;
 *attempted=true;rc=ioctl(fd,DRM_IOCTL_REMESA_RETAINED,r);e=rc<0?errno:0;
 if(record(context,&before,r,rc,e))return EOWNERDEAD;
 if(rc)return EOWNERDEAD; /* Never retry, including EINTR or unknown admission. */
 if(memcmp(r,&before,offsetof(struct remesa_retained_request,operation)))return EOWNERDEAD;
 if(!r->operation || !r->serial || !r->generation || r->slot>=8 || r->state!=6 ||
    r->error || r->admitted!=1 || r->attached || !r->observer_request_bytes ||
    !r->observer_response_bytes || r->observer_request_bytes>sizeof(r->observer_request) ||
    r->observer_response_bytes>sizeof(r->observer_response))return EOWNERDEAD;
 for(unsigned i=r->observer_request_bytes;i<sizeof(r->observer_request);i++)
  if(r->observer_request[i])return EOWNERDEAD;
 for(unsigned i=r->observer_response_bytes;i<sizeof(r->observer_response);i++)
  if(r->observer_response[i])return EOWNERDEAD;
 return 0;
}
