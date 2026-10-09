/* SPDX-License-Identifier: MIT */
#include "remesa_persistent.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#define PAGE_BYTES 4096u
struct memory {
 struct remesa_token token;uint32_t handle;uint64_t va;
 size_t bytes,allocated;unsigned refs;
 bool live,owner,pinned,gpu_mapped,reserved,cpu_mapped,readable,quarantined;
};
struct submission {
 struct remesa_fence fence;struct memory *target;
 struct remesa_draw_packet packet;uint64_t operation;
 bool live,detached,released;int error;
};
struct persistent {
 struct remesa_persistent_config cfg;size_t allocated;
 struct remesa_slot slots[REMESA_PERSISTENT_BUFFERS];
 struct remesa_registry registry;struct remesa_provider provider;
 struct memory buffers[REMESA_PERSISTENT_BUFFERS];
 struct submission jobs[REMESA_PROVIDER_MAX_JOBS];
 struct submission *dispatching;bool failed;
};
static bool owns_memory(struct persistent *d,const void *object)
{ for(unsigned i=0;i<REMESA_PERSISTENT_BUFFERS;i++) if(object==&d->buffers[i])return true;return false; }
static bool owns_job(struct persistent *d,const void *object)
{ for(unsigned i=0;i<REMESA_PROVIDER_MAX_JOBS;i++) if(object==&d->jobs[i])return true;return false; }
static int status(int s)
{ return s==REMESA_OK?0:s==REMESA_BUSY?EBUSY:s==REMESA_UNSUPPORTED?EOPNOTSUPP:s==REMESA_AMBIGUOUS?EIO:EINVAL; }
static bool token_equal(struct remesa_token a,struct remesa_token b)
{ return a.slot==b.slot && a.generation==b.generation; }
static void quarantine(struct persistent *d,struct memory *b)
{
 b->quarantined=true;b->readable=false;
 if(b->token.generation) remesa_resource_quarantine(&d->slots[b->token.slot].resource);
}
static void memory_error(struct persistent *d,struct memory *b,int error)
{ quarantine(d,b);if(error==EOWNERDEAD)d->failed=true; }
static void dispose(struct persistent *d,struct memory *b)
{
 const struct remesa_persistent_ops *o=d->cfg.ops;int e;
 if(!b->live || b->refs || b->quarantined) return;
 if(b->token.generation) {
  struct remesa_slot *slot=&d->slots[b->token.slot];
  if(slot->holds || (slot->resource.state!=REMESA_NEW && slot->resource.state!=REMESA_MAPPED)) {
   quarantine(d,b);return;
  }
 }
 if(b->gpu_mapped) {
  e=o->gpu_unmap(d->cfg.priv,b->handle,b->va,b->allocated);
  if(e) {memory_error(d,b,e);return;} b->gpu_mapped=false;
 }
 if(b->pinned) {
  e=o->unpin(d->cfg.priv,b->handle);if(e) {memory_error(d,b,e);return;} b->pinned=false;
 }
 if(b->handle) {
  e=o->destroy(d->cfg.priv,b->handle);if(e) {memory_error(d,b,e);return;} b->handle=0;
 }
 if(b->token.generation && remesa_registry_remove(&d->registry,b->token)!=REMESA_OK) {
  quarantine(d,b);return;
 }
 d->allocated-=b->allocated;memset(b,0,sizeof(*b));
}
static void drop_memory(struct persistent *d,struct memory *b)
{ if(b->refs) --b->refs;dispose(d,b); }
static int reserve_va(struct persistent *d,struct memory *b)
{
 uint64_t va=d->cfg.va_start;
 for(unsigned attempt=0;attempt<=REMESA_PERSISTENT_BUFFERS;attempt++) {
  bool overlap=false;
  if(va>d->cfg.va_end || b->allocated>d->cfg.va_end-va) return ENOSPC;
  for(unsigned i=0;i<REMESA_PERSISTENT_BUFFERS;i++) {
   struct memory *other=&d->buffers[i];
   if(other==b || !other->live || !other->reserved) continue;
   if(va<other->va+other->allocated && other->va<va+b->allocated) {
    va=other->va+other->allocated;overlap=true;break;
   }
  }
  if(!overlap) {b->va=va;b->reserved=true;return 0;}
 }
 return ENOSPC;
}
static int allocate(void *data,size_t bytes,void **out)
{
 struct persistent *d=data;struct memory *b=NULL;int e;
 if(!out) return EINVAL;
 *out=NULL;
 if(d->failed) return EIO;
 if(!bytes || bytes>SIZE_MAX-(PAGE_BYTES-1)) return EINVAL;
 size_t n=(bytes+PAGE_BYTES-1)&~(size_t)(PAGE_BYTES-1);
 if(n>d->cfg.byte_budget-d->allocated) return ENOSPC;
 for(unsigned i=0;i<REMESA_PERSISTENT_BUFFERS;i++) if(!d->buffers[i].live) {b=&d->buffers[i];break;}
 if(!b) return ENOSPC;
 *b=(struct memory){.live=true,.bytes=bytes,.allocated=n};d->allocated+=n;
 e=status(remesa_registry_allocate(&d->registry,n,&b->token));if(e) goto fail;
 e=reserve_va(d,b);if(e) goto fail;
 e=d->cfg.ops->create(d->cfg.priv,n,&b->handle);if(e) goto fail;
 if(!b->handle) {e=EPROTO;goto fail;}
 e=d->cfg.ops->pin(d->cfg.priv,b->handle);if(e) goto fail;b->pinned=true;
 e=d->cfg.ops->gpu_map(d->cfg.priv,b->handle,b->va,n);if(e) goto fail;b->gpu_mapped=true;
 e=status(remesa_registry_map(&d->registry,b->token,b->va,PAGE_BYTES));if(e) goto fail;
 b->refs=1;b->owner=true;*out=b;return 0;
fail:
 if(e==EOWNERDEAD) {quarantine(d,b);d->failed=true;} else dispose(d,b);
 return e;
}
static void destroy_bo(void *data,void *object)
{
 struct persistent *d=data;struct memory *b=object;
 if(!owns_memory(d,b) || !b->live || !b->owner) return;
 b->owner=false;drop_memory(d,b);
}
static int map_bo(void *data,void *object,bool write,void **out)
{
 struct persistent *d=data;struct memory *b=object;struct remesa_resource r;int e;
 if(!out) return EINVAL;
 *out=NULL;
 if(!owns_memory(d,b)) return EINVAL;
 if(!b->live || b->quarantined) return EIO;
 if(!b->owner) return EINVAL;
 if(b->cpu_mapped || b->refs==UINT32_MAX) return EBUSY;
 if(remesa_registry_snapshot(&d->registry,b->token,&r)!=REMESA_OK) return EINVAL;
 if(r.state!=REMESA_MAPPED) return EBUSY;
 if(!write && !b->readable) return EAGAIN;
 e=d->cfg.ops->cpu_map(d->cfg.priv,b->handle,write,out);
 if(e==EOWNERDEAD) {quarantine(d,b);d->failed=true;}
 if(e) {*out=NULL;return e;}
 if(!*out) {quarantine(d,b);return EPROTO;}
 b->cpu_mapped=true;++b->refs;return 0;
}
static void unmap_bo(void *data,void *object,bool write)
{
 struct persistent *d=data;struct memory *b=object;int e;
 if(!owns_memory(d,b) || !b->live || !b->cpu_mapped) return;
 e=d->cfg.ops->cpu_unmap(d->cfg.priv,b->handle,write);
 b->cpu_mapped=false;
 if(!e && write) e=d->cfg.ops->sync_for_device(d->cfg.priv,b->handle);
 if(e) {quarantine(d,b);if(e==EOWNERDEAD)d->failed=true;}
 else if(write) b->readable=true;
 drop_memory(d,b);
}
static enum remesa_dispatch dispatch(void *data,const struct remesa_job *job)
{
 struct persistent *d=data;struct submission *j=d->dispatching;
 if(!j || job->count!=1 || !token_equal(job->uses[0].token,j->target->token)) return REMESA_NOT_ISSUED;
 j->operation=job->operation;
 struct remesa_gpu_submit s={job->context,job->operation,j->target->va,j->target->bytes,j->target->handle,&j->packet};
 return d->cfg.ops->dispatch(d->cfg.priv,&s);
}
static int release(void *data,const struct remesa_job *job)
{
 struct persistent *d=data;
 /* Visibility is separate from retirement, and the mapping remains pinned. */
 if(job->state!=REMESA_FENCE_PUBLISHED) return REMESA_BUSY;
 int e=d->cfg.ops->release(d->cfg.priv,job->context,job->operation);
 return e==EAGAIN?REMESA_RETRY:e?REMESA_BUSY:REMESA_OK;
}
static int submit(void *data,void *object,const struct remesa_draw_packet *packet,void **out)
{
 struct persistent *d=data;
 if(d->cfg.resource_only_backend) {if(out)*out=NULL;return EOPNOTSUPP;}
 struct memory *b=object;struct submission *j=NULL;int s;
 if(!out) return EINVAL;
 *out=NULL;
 if(!owns_memory(d,b)) return EINVAL;
 if(d->failed || !b->live || b->quarantined) return EIO;
 if(!b->owner) return EINVAL;
 if(b->cpu_mapped || b->refs==UINT32_MAX) return EBUSY;
 if(b->bytes!=4096 || remesa_draw_packet_validate(packet)!=REMESA_PACKET_OK) return EINVAL;
 for(unsigned i=0;i<REMESA_PROVIDER_MAX_JOBS;i++) if(!d->jobs[i].live) {j=&d->jobs[i];break;}
 if(!j) return EBUSY;
 *j=(struct submission){.target=b,.live=true};j->packet=*packet;++b->refs;
 struct remesa_use use={b->token,0,b->bytes,REMESA_ACCESS_WRITE};
 d->dispatching=j;s=remesa_provider_submit(&d->provider,&use,1,&j->fence);d->dispatching=NULL;
 if(s!=REMESA_OK && s!=REMESA_AMBIGUOUS) {memset(j,0,sizeof(*j));drop_memory(d,b);return status(s);}
 b->readable=false;*out=j;
 if(s==REMESA_AMBIGUOUS) {j->error=EIO;quarantine(d,b);d->failed=true;}
 return status(s);
}
static int fault(struct persistent *d,struct submission *j)
{
 remesa_provider_fault(&d->provider,j->fence,d->cfg.context,j->operation);
 quarantine(d,j->target);j->error=EIO;d->failed=true;return EIO;
}
static int wait_job(void *data,void *object,uint64_t timeout,bool visible)
{
 struct persistent *d=data;struct submission *j=object;struct remesa_gpu_receipt r={0};struct remesa_job job;int e;
 (void)visible; /* Always require publication before permitting backing reuse. */
 if(!owns_job(d,j) || !j->live) return EINVAL;
 if(j->error) return j->error;
 if(j->released) return 0;
 e=d->cfg.ops->poll(d->cfg.priv,d->cfg.context,j->operation,timeout,&r);
 if(e==EAGAIN || e==ETIMEDOUT || e==EBUSY) return e;
 if(e || r.context!=d->cfg.context || r.operation!=j->operation || r.failed || (r.published && !r.retired)) return fault(d,j);
 if(remesa_provider_query(&d->provider,j->fence,&job)!=REMESA_OK) return fault(d,j);
 if((job.state==REMESA_FENCE_RETIRED || job.state==REMESA_FENCE_PUBLISHED) && !r.retired) return fault(d,j);
 if(job.state==REMESA_FENCE_PUBLISHED && !r.published) return fault(d,j);
 if(r.retired && job.state==REMESA_FENCE_PENDING &&
    remesa_provider_complete(&d->provider,j->fence,r.context,r.operation,1)!=REMESA_OK) return fault(d,j);
 if(!r.published) return EAGAIN;
 /* Completion may be reported out of order. Publication/acknowledgement must
  * stay in endpoint operation order, including jobs detached by a destroyed
  * context. Do not release a newer GEM writer while an older receipt remains
  * unacknowledged. Drain older dependencies with nonblocking polls; a caller
  * waiting on the newest Gallium fence need not keep every earlier handle.
  * Recursion strictly decreases operation IDs and is bounded by job slots. */
 for(unsigned i=0;i<REMESA_PROVIDER_MAX_JOBS;i++) {
  struct submission *earlier=&d->jobs[i];
  if(earlier->live && !earlier->released && earlier->operation<j->operation) {
   int older=wait_job(d,earlier,0,true);
   if(older==EAGAIN || older==ETIMEDOUT || older==EBUSY)return EAGAIN;
   if(older)return fault(d,j);
  }
 }
 if(job.state!=REMESA_FENCE_PUBLISHED &&
    remesa_provider_publish(&d->provider,j->fence,r.context,r.operation)!=REMESA_OK) return fault(d,j);
 int released=remesa_provider_release(&d->provider,j->fence);
 if(released==REMESA_RETRY) return EAGAIN;
 if(released!=REMESA_OK) return fault(d,j);
 j->target->readable=true;drop_memory(d,j->target);j->target=NULL;j->released=true;
 return 0;
}
static void destroy_fence(void *data,void *object)
{
 struct persistent *d=data;struct submission *j=object;
 if(!owns_job(d,j) || !j->live) return;
 j->detached=true;
 if(j->released) memset(j,0,sizeof(*j));
 else {int e=wait_job(d,j,0,true);if(!e)memset(j,0,sizeof(*j));}
}
static const struct remesa_device_ops device_ops={allocate,destroy_bo,map_bo,unmap_bo,submit,wait_job,destroy_fence};
int remesa_persistent_open(struct remesa_device *device,const struct remesa_persistent_config *cfg)
{
 struct persistent *d;
 if(device && (device->ops || device->priv)) return EBUSY;
 if(!device || !cfg || !cfg->ops || !cfg->context ||
    ((unsigned)cfg->offline_test_backend+(unsigned)cfg->qualified_hardware_backend+
     (unsigned)cfg->resource_only_backend)!=1 || !cfg->byte_budget ||
    !cfg->va_start || cfg->va_start%PAGE_BYTES || cfg->va_end%PAGE_BYTES || cfg->va_end<=cfg->va_start) return EINVAL;
 const struct remesa_persistent_ops *o=cfg->ops;
 if(!o->create||!o->pin||!o->gpu_map||!o->gpu_unmap||!o->unpin||!o->destroy||!o->cpu_map||!o->cpu_unmap||!o->sync_for_device||!o->dispatch||!o->poll||!o->release) return EOPNOTSUPP;
 d=calloc(1,sizeof(*d));if(!d)return ENOMEM;d->cfg=*cfg;
 struct remesa_backend b={dispatch,release,d};
 if(remesa_registry_init(&d->registry,d->slots,REMESA_PERSISTENT_BUFFERS)!=REMESA_OK ||
    remesa_provider_init(&d->provider,&d->registry,cfg->context,&b)!=REMESA_OK) {free(d);return EINVAL;}
 struct remesa_resource_limits limits=cfg->resource_limits;
 uint64_t capacity=cfg->byte_budget;
 if(capacity>cfg->va_end-cfg->va_start)capacity=cfg->va_end-cfg->va_start;
 if(!limits.max_resource_bytes || limits.max_resource_bytes>capacity)limits.max_resource_bytes=capacity;
 *device=(struct remesa_device){.ops=&device_ops,.priv=d,.offline_test_backend=cfg->offline_test_backend,.qualified_hardware_backend=cfg->qualified_hardware_backend,.resource_only_backend=cfg->resource_only_backend,.framebuffer_mode=REMESA_FRAMEBUFFER_REPLACE,.resource_limits=limits};return 0;
}
int remesa_persistent_collect(struct remesa_device *device)
{
 if(!device || device->ops!=&device_ops || !device->priv)return EINVAL;
 struct persistent *d=device->priv;
 for(unsigned attempt=0;attempt<REMESA_PROVIDER_MAX_JOBS;attempt++) {
  struct submission *oldest=NULL;
  for(unsigned i=0;i<REMESA_PROVIDER_MAX_JOBS;i++) {
   struct submission *j=&d->jobs[i];
   if(j->live && j->detached && (!oldest || j->operation<oldest->operation))oldest=j;
  }
  if(!oldest)return 0;
  int e=wait_job(d,oldest,0,true);if(e)return e;
  memset(oldest,0,sizeof(*oldest));
 }
 return 0;
}
int remesa_persistent_close(struct remesa_device *device)
{
 if(!device || device->ops!=&device_ops || !device->priv)return EINVAL;
 struct persistent *d=device->priv;remesa_persistent_collect(device);
 for(unsigned i=0;i<REMESA_PROVIDER_MAX_JOBS;i++) if(d->jobs[i].live)return EBUSY;
 for(unsigned i=0;i<REMESA_PERSISTENT_BUFFERS;i++) if(d->buffers[i].live)return EBUSY;
 free(d);memset(device,0,sizeof(*device));return 0;
}
