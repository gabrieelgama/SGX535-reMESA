/* SPDX-License-Identifier: MIT */
/* INFO ioctl responses below are synthetic ownership records. No GPU results. */
#include "remesa_linux_memory.h"
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static unsigned calls,mode;
int ioctl(int fd,unsigned long request,...)
{
 va_list ap;struct remesa_memory_request *r;assert(fd==99&&request==DRM_IOCTL_REMESA_MEMORY);
 va_start(ap,request);r=va_arg(ap,void *);va_end(ap);calls++;
 assert(r->context==77);
 if(r->action==REMESA_MEM_CREATE){r->id=32;return 0;}
 if(r->action==REMESA_MEM_DESTROY)return 0;
 assert(r->action==REMESA_MEM_INFO&&r->id==32);
 struct remesa_memory_info i={.allocated_bytes=32u*1024u*1024u,
  .object_bytes=32u*1024u*1024u,.gpu_va=0x40000000,.objects=1,.pinned_objects=1,
  .mapped_pages=8192,.object_pinned=1,.object_mapped_pages=8192,.verified_pages=8192};
 switch(mode){case 1:i.gpu_va+=4096;break;case 2:--i.verified_pages;break;
 case 3:i.object_pinned=0;break;case 4:i.object_lease=1;break;
 case 5:i.quarantined=1;break;case 6:i.object_bytes-=4096;break;
 case 7:i.allocated_bytes=129u*1024u*1024u;break;
 case 8:r->context++;break;case 9:i.mapped_pages=1;break;
 case 10:r->id++;break;case 11:r->data[4095]=1;break;}
 memcpy(r->data,&i,sizeof(i));return 0;
}
/* Poisoned endpoints intentionally retain identities; forget drops host-only
 * metadata after the fixture's endpoint is discarded, not a GPU reset. */
int main(void)
{
 const struct remesa_persistent_ops *ops=remesa_linux_memory_ops();
 for(mode=0;mode<12;mode++){
  struct remesa_linux_memory *d=NULL;uint32_t id=0;
  assert(!remesa_linux_memory_attach(99,77,&d));assert(!ops->create(d,32u*1024u*1024u,&id)&&id==32);
  unsigned before=calls;
  assert(remesa_linux_memory_validate_backing(d,id,0x40000001,32u*1024u*1024u)==EINVAL);
  assert(remesa_linux_memory_validate_backing(d,id,UINT64_C(0x100000000),32u*1024u*1024u)==EINVAL&&calls==before);
  assert(remesa_linux_memory_validate_backing(d,id,UINT64_C(0xfffff000),32u*1024u*1024u)==EINVAL&&calls==before);
  assert(remesa_linux_memory_validate_backing(d,id,0x40000000,4096)==EINVAL);
  assert(remesa_linux_memory_validate_backing(d,64,0x40000000,32u*1024u*1024u)==EINVAL&&calls==before);
  int e=remesa_linux_memory_validate_backing(d,id,0x40000000,32u*1024u*1024u);
  if(!mode){assert(!e);assert(!ops->destroy(d,id));assert(!remesa_linux_memory_detach(d));}
  else {assert(e==EOWNERDEAD||e==EPROTO);before=calls;
   assert(remesa_linux_memory_validate_backing(d,id,0x40000000,32u*1024u*1024u)==EOWNERDEAD&&calls==before);
   assert(ops->destroy(d,id)==EOWNERDEAD&&calls==before);
   assert(remesa_linux_memory_detach(d)==EBUSY);remesa_linux_memory_forget(d);
  }
 }
 puts("PASS: retained Linux ownership admission; 32 MiB backing accepted; 11 stale/malformed VA/PFN/pin/lease/quarantine/size/context/count responses poisoned; no submission or GPU completion");
}
