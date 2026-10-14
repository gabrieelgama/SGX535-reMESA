/* SPDX-License-Identifier: MIT */
#include "remesa_retained_readback.h"
#include "remesa_device.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h"
#include <errno.h>
#include <string.h>
#include <limits.h>
int remesa_retained_upload(struct pipe_context *c,struct pipe_resource *r,
 size_t offset,const void *data,size_t bytes)
{
 struct pipe_transfer *t=NULL;void *p;struct pipe_box box={0};
 if(!c || !r || !data || !bytes || r->screen!=c->screen ||
    r->target!=PIPE_BUFFER || offset>r->width0 || bytes>r->width0-offset ||
    offset>INT_MAX || bytes>INT_MAX)return EINVAL;
 if(!c->buffer_map || !c->buffer_unmap)return ENOTSUP;
 box.x=(int)offset;box.width=(int)bytes;box.height=box.depth=1;
 p=c->buffer_map(c,r,0,PIPE_MAP_WRITE|PIPE_MAP_DONTBLOCK,&box,&t);
 if(!p || !t){if(t)c->buffer_unmap(c,t);return EBUSY;}
 if(t->resource!=r || t->level || t->box.x!=box.x || t->box.width!=box.width ||
    t->box.y || t->box.z || t->box.height!=1 || t->box.depth!=1){
  c->buffer_unmap(c,t);return EPROTO;
 }
 memcpy(p,data,bytes);c->buffer_unmap(c,t);return remesa_context_error(c);
}
int remesa_retained_readback(struct pipe_context *c,struct pipe_resource *r,
 struct pipe_fence_handle *f,uint64_t context,uint64_t operation,
 const struct remesa_gpu_receipt *receipt,uint64_t timeout,void *out,size_t bytes)
{
 struct pipe_transfer *t=NULL;const unsigned char *p;size_t end;
 struct pipe_box box={.width=32,.height=32,.depth=1};
 if(!c || !c->screen || !r || !f || !receipt || !out || bytes!=4096 ||
    !context || !operation || r->screen!=c->screen)return EINVAL;
 if(receipt->context!=context || receipt->operation!=operation)return ESTALE;
 if(receipt->failed)return EIO;
 if(!receipt->retired || !receipt->published)return EAGAIN;
 if(r->target!=PIPE_TEXTURE_2D || r->format!=PIPE_FORMAT_B8G8R8A8_UNORM ||
    r->width0!=32 || r->height0!=32 || r->depth0!=1 || r->array_size!=1 ||
    r->last_level || r->nr_samples>1 || r->nr_storage_samples>1)return ENOTSUP;
 if(!c->screen->fence_finish || !c->texture_map || !c->texture_unmap)return ENOTSUP;
 if(!c->screen->fence_finish(c->screen,c,f,timeout))return EAGAIN;
 /* Even after a fence finishes, map checks the resource's current writer. */
 p=c->texture_map(c,r,0,PIPE_MAP_READ|PIPE_MAP_DONTBLOCK,&box,&t);
 if(!p || !t){if(t)c->texture_unmap(c,t);return EBUSY;}
 end=(size_t)t->stride*31;
 if(end/31!=t->stride || end>SIZE_MAX-128 || t->resource!=r || t->level || t->stride<128 ||
    t->box.x || t->box.y || t->box.z ||
    t->box.width!=32 || t->box.height!=32 || t->box.depth!=1){
  c->texture_unmap(c,t);return EPROTO;
 }
 for(unsigned y=0;y<32;y++)memcpy((unsigned char *)out+y*128,p+(size_t)y*t->stride,128);
 c->texture_unmap(c,t);return 0;
}
int remesa_retained_readback_verified(struct pipe_context *c,struct pipe_resource *r,
 struct pipe_fence_handle *f,const struct remesa_evidence_expected *expected,
 const struct remesa_evidence_response *response,const struct remesa_evidence_observer *observer,
 const struct remesa_evidence_ownership *ownership,uint64_t timeout,void *out,size_t bytes)
{
 struct remesa_gpu_receipt receipt;
 if(remesa_retained_completion_verify(expected,response,observer,ownership)!=REMESA_EVIDENCE_OK)return EPROTO;
 receipt=(struct remesa_gpu_receipt){expected->id.context,expected->id.operation,true,true,false};
 return remesa_retained_readback(c,r,f,expected->id.context,expected->id.operation,&receipt,timeout,out,bytes);
}
int remesa_retained_compare(const void *image,size_t bytes,const void *reference,
 size_t ref_bytes,unsigned *mismatches)
{
 const unsigned char *a=image,*b=reference;unsigned n=0;
 if(!a || !b || !mismatches || bytes!=4096 || ref_bytes!=4096)return EINVAL;
 for(unsigned i=0;i<4096;i++)n+=a[i]!=b[i];
 *mismatches=n;return n?EILSEQ:0;
}
