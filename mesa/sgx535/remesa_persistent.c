/* SPDX-License-Identifier: MIT */
#include "remesa_persistent.h"
#include "remesa_shader_program.h"
#include "remesa_draw.h"
#include "remesa_pds.h"
#include "remesa/shader_request.h"
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
struct shader_upload {
 struct shader_upload *next;
 void *token;
 struct remesa_shader_image *image;
 struct remesa_shader_request request;
 uint32_t handle;
 size_t allocated;
 bool pinned,leased,staged,quarantined,owner;
 /* Raw pre-VS data has a separate CPU-only allocation. Neither this object
  * nor the program backing has an SGX mapping or an execution address. */
 struct {
  struct remesa_shader_draw_snapshot *snapshot;
  uint32_t handle;
  size_t allocated,vertex_bytes,index_offset,index_bytes;
  bool pinned,leased,staged;
 } geometry;
 /* Original HOST DATA only. The single CPU backing contains both snapshots,
  * never CODE, an SGX mapping or an executable task-control contract. */
 struct {
  struct remesa_shader_storage *data[2];
  uint32_t handle;
  size_t allocated,offset[2],length[2];
  bool pinned,leased,staged;
 } pds;
};
struct persistent {
 struct remesa_persistent_config cfg;size_t allocated;
 struct remesa_slot slots[REMESA_PERSISTENT_BUFFERS];
 struct remesa_registry registry;struct remesa_provider provider;
 struct memory buffers[REMESA_PERSISTENT_BUFFERS];
 struct submission jobs[REMESA_PROVIDER_MAX_JOBS];
 struct submission *dispatching;bool failed;
 struct shader_upload *shaders;
 struct remesa_shader_cookie_pool shader_cookies;
 unsigned shader_count;
 uint64_t shader_generation;
};
static struct shader_upload **shader_link(struct persistent *d,const void *token)
{
 if(!d || !token)return NULL;
 for(struct shader_upload **p=&d->shaders;*p;p=&(*p)->next)
  if((*p)->token==token)return p;
 return NULL;
}
/* Never free uncertain GEM backing, even when no pointer was returned by a
 * failed CPU-map call. The provider may have acquired a lease before failing. */
static int shader_quarantine(struct persistent *d,struct shader_upload *s)
{s->quarantined=true;d->failed=true;return EOWNERDEAD;}
/* Check provider object identity even when CREATE reports an ordinary error.
 * A repeated live ID cannot be reclaimed as a newly created allocation. */
static bool handle_aliases(struct persistent *d,uint32_t handle,const uint32_t *field)
{
 bool alias=false;
 if(!handle)return false;
 for(struct shader_upload *s=d->shaders;s;s=s->next) {
  if((&s->handle!=field && s->handle==handle) ||
     (&s->geometry.handle!=field && s->geometry.handle==handle)) {
   s->quarantined=true;alias=true;
  }
  if(&s->pds.handle!=field && s->pds.handle==handle) {
   s->quarantined=true;alias=true;
  }
 }
 for(unsigned i=0;i<REMESA_PERSISTENT_BUFFERS;i++) {
  struct memory *b=&d->buffers[i];
  if(d->buffers[i].live && &d->buffers[i].handle!=field &&
     d->buffers[i].handle==handle) {
   b->quarantined=true;b->readable=false;alias=true;
   if(b->token.generation)remesa_resource_quarantine(&d->slots[b->token.slot].resource);
  }
 }
 if(alias)d->failed=true;
 return alias;
}
static int geometry_dispose(struct persistent *d,struct shader_upload *s)
{
 const struct remesa_persistent_ops *o=d->cfg.ops;
 if(s->quarantined || s->geometry.leased)return shader_quarantine(d,s);
 if(s->geometry.pinned) {
  int e=o->unpin(d->cfg.priv,s->geometry.handle);
  if(e)return shader_quarantine(d,s);
  s->geometry.pinned=false;
 }
 if(s->geometry.handle) {
  int e=o->destroy(d->cfg.priv,s->geometry.handle);
  if(e)return shader_quarantine(d,s);
 }
 d->allocated-=s->geometry.allocated;
 remesa_shader_draw_snapshot_destroy(s->geometry.snapshot);
 memset(&s->geometry,0,sizeof(s->geometry));return 0;
}
static int pds_dispose(struct persistent *d,struct shader_upload *s)
{
 const struct remesa_persistent_ops *o=d->cfg.ops;
 if(s->quarantined || s->pds.leased)return shader_quarantine(d,s);
 if(s->pds.pinned) {
  int e=o->unpin(d->cfg.priv,s->pds.handle);
  if(e)return shader_quarantine(d,s);
  s->pds.pinned=false;
 }
 if(s->pds.handle) {
  int e=o->destroy(d->cfg.priv,s->pds.handle);
  if(e)return shader_quarantine(d,s);
  s->pds.handle=0;
 }
 d->allocated-=s->pds.allocated;s->pds.allocated=0;
 for(unsigned i=0;i<2;i++)remesa_shader_storage_reference(&s->pds.data[i],NULL);
 memset(&s->pds,0,sizeof(s->pds));return 0;
}
static int shader_dispose(struct persistent *d,struct shader_upload **link)
{
 struct shader_upload *s=*link;
 const struct remesa_persistent_ops *o=d->cfg.ops;
 if(s->quarantined || s->leased)return shader_quarantine(d,s);
 int pds_error=pds_dispose(d,s);if(pds_error)return pds_error;
 int geometry_error=geometry_dispose(d,s);if(geometry_error)return geometry_error;
 if(s->pinned) {
  int e=o->unpin(d->cfg.priv,s->handle);
  if(e)return shader_quarantine(d,s);
  s->pinned=false;
 }
 if(s->handle) {
  int e=o->destroy(d->cfg.priv,s->handle);
  if(e)return shader_quarantine(d,s);
 }
 *link=s->next;d->allocated-=s->allocated;--d->shader_count;
 remesa_shader_image_destroy(s->image);free(s);return 0;
}
static int stage_lease(struct persistent *d,struct shader_upload *s,bool write)
{
 void *cpu=NULL;
 const struct remesa_persistent_ops *o=d->cfg.ops;
 int e=o->cpu_map(d->cfg.priv,s->handle,write,&cpu);
 if(e) {
  /* An error plus a pointer violates the ordinary atomic-error contract. */
  if(e==EOWNERDEAD || cpu)return shader_quarantine(d,s);
  return e;
 }
 s->leased=true;
 if(cpu) {
  if(write) {
   memset(cpu,0,s->allocated);
   memcpy(cpu,s->request.data,s->request.byte_count);
  } else {
   if(memcmp(cpu,s->request.data,s->request.byte_count))e=EPROTO;
   for(size_t i=s->request.byte_count;i<s->allocated;i++)
    if(((const unsigned char *)cpu)[i]) {e=EPROTO;break;}
  }
 } else e=EPROTO;
 /* A success-with-NULL still acquired a lease. Release once, without touching
  * memory, and only reclaim backing if that release demonstrably succeeds. */
 int release_error=o->cpu_unmap(d->cfg.priv,s->handle,write);
 if(release_error)return shader_quarantine(d,s);
 s->leased=false;return e;
}
static int pds_lease(struct persistent *d,struct shader_upload *s,bool write)
{
 struct remesa_shader_storage_view view[2];
 for(unsigned i=0;i<2;i++) {
  int e=remesa_shader_storage_inspect(s->pds.data[i],&view[i]);if(e)return e;
 }
 void *cpu=NULL;
 const struct remesa_persistent_ops *o=d->cfg.ops;
 int e=o->cpu_map(d->cfg.priv,s->pds.handle,write,&cpu);
 if(e) {
  if(e==EOWNERDEAD||cpu)return shader_quarantine(d,s);
  return e;
 }
 s->pds.leased=true;
 if(cpu) {
  if(write) {
   memset(cpu,0,s->pds.allocated);
   for(unsigned i=0;i<2;i++)
    memcpy((unsigned char *)cpu+s->pds.offset[i],view[i].bytes,s->pds.length[i]);
  } else {
   for(unsigned i=0;i<2;i++)
    if(memcmp((const unsigned char *)cpu+s->pds.offset[i],view[i].bytes,s->pds.length[i]))e=EPROTO;
   for(size_t i=s->pds.length[0];i<s->pds.allocated;i++)
    if((i<s->pds.offset[1]||i>=s->pds.offset[1]+s->pds.length[1])&&
       ((const unsigned char *)cpu)[i]){e=EPROTO;break;}
  }
 } else e=EPROTO;
 int release_error=o->cpu_unmap(d->cfg.priv,s->pds.handle,write);
 if(release_error)return shader_quarantine(d,s);
 s->pds.leased=false;return e;
}
static int geometry_lease(struct persistent *d,struct shader_upload *s,bool write)
{
 struct remesa_shader_draw_inputs inputs;
 struct remesa_draw_plan plan;
 int e=remesa_shader_draw_snapshot_inspect(s->geometry.snapshot,&inputs,&plan);
 if(e)return e;
 void *cpu=NULL;
 const struct remesa_persistent_ops *o=d->cfg.ops;
 e=o->cpu_map(d->cfg.priv,s->geometry.handle,write,&cpu);
 if(e) {
  if(e==EOWNERDEAD || cpu)return shader_quarantine(d,s);
  return e;
 }
 s->geometry.leased=true;
 if(cpu) {
  if(write) {
   memset(cpu,0,s->geometry.allocated);
   memcpy(cpu,inputs.vertex_data,s->geometry.vertex_bytes);
   if(s->geometry.index_bytes)
    memcpy((unsigned char *)cpu+s->geometry.index_offset,
           inputs.index_data,s->geometry.index_bytes);
  } else {
   if(memcmp(cpu,inputs.vertex_data,s->geometry.vertex_bytes) ||
      (s->geometry.index_bytes &&
       memcmp((const unsigned char *)cpu+s->geometry.index_offset,
              inputs.index_data,s->geometry.index_bytes)))e=EPROTO;
   for(size_t i=s->geometry.vertex_bytes;i<s->geometry.allocated;i++)
    if((i<s->geometry.index_offset ||
        i>=s->geometry.index_offset+s->geometry.index_bytes) &&
       ((const unsigned char *)cpu)[i]) {e=EPROTO;break;}
  }
 } else e=EPROTO;
 int release_error=o->cpu_unmap(d->cfg.priv,s->geometry.handle,write);
 if(release_error)return shader_quarantine(d,s);
 s->geometry.leased=false;return e;
}
static enum remesa_shader_request_status image_request(const struct remesa_shader_image_view *view,
 uint64_t owner,uint64_t generation,struct remesa_shader_request *out)
{
 struct remesa_shader_blob blobs[REMESA_SHADER_RESOURCE_COUNT]={{0}};
 if(view->vertex.byte_length)blobs[REMESA_SHADER_VERTEX_USSE-1]=(struct remesa_shader_blob){
  .bytes=view->vertex.bytes,.length=(uint32_t)view->vertex.byte_length,
  .alignment=(uint32_t)view->vertex.byte_alignment,.state=REMESA_SHADER_CPU_CANDIDATE
 };
 blobs[REMESA_SHADER_FRAGMENT_USSE-1]=(struct remesa_shader_blob){
  .bytes=view->fragment.bytes,.length=(uint32_t)view->fragment.byte_length,
  .alignment=(uint32_t)view->fragment.byte_alignment,.state=REMESA_SHADER_CPU_CANDIDATE
 };
 return remesa_shader_request_build(out,owner,generation,blobs);
}
static int prepare_candidate(void *data,
 const struct remesa_shader_draw_programs *programs,
 const struct remesa_usse_vertex_mapping *mapping,void **out)
{
 struct persistent *d=data;
 struct shader_upload *s;
 struct remesa_shader_image_view view;
 int e;
 if(!out)return EINVAL;
 if(d) {
  int cookie_error=remesa_shader_cookie_output_check(&d->shader_cookies,out,sizeof(*out));
  if(cookie_error)return cookie_error;
 }
 *out=NULL;
 if(!d || !programs)return EINVAL;
 if(d->failed)return EIO;
 if(d->shader_cookies.count>=REMESA_SHADER_COOKIE_LIMIT)return ENOSPC;
 if(d->shader_generation==UINT64_MAX)return EOVERFLOW;
 if(d->shader_count==REMESA_PERSISTENT_BUFFERS ||
    PAGE_BYTES>d->cfg.byte_budget-d->allocated)return ENOSPC;
 /* CPU research records must not enter the ordinary allocation path, which
  * reserves and installs an SGX VA. This path creates/pins only CPU backing. */
 s=calloc(1,sizeof(*s));if(!s)return ENOMEM;
 e=mapping?remesa_shader_image_prepare_vertex_candidate(programs,mapping,&s->image):
  remesa_shader_image_prepare(programs,&s->image);
 if(e){free(s);return e;}
 e=remesa_shader_image_inspect(s->image,&view);
 if(e){remesa_shader_image_destroy(s->image);free(s);return e;}
 if(!view.fragment.byte_length || view.fragment.byte_length>PAGE_BYTES ||
    view.fragment.byte_alignment>PAGE_BYTES || view.vertex.byte_length>PAGE_BYTES ||
    view.vertex.byte_alignment>PAGE_BYTES) {
  remesa_shader_image_destroy(s->image);free(s);return ENOTSUP;
 }
 enum remesa_shader_request_status status=image_request(&view,d->cfg.context,
  d->shader_generation+1,&s->request);
 if(status!=REMESA_SHADER_REQUEST_OK) {
  remesa_shader_image_destroy(s->image);free(s);
  return status==REMESA_SHADER_REQUEST_NOMEM?ENOMEM:EINVAL;
 }
 if(s->request.byte_count>PAGE_BYTES) {
  remesa_shader_image_destroy(s->image);free(s);return E2BIG;
 }
 s->allocated=PAGE_BYTES;s->next=d->shaders;d->shaders=s;
 ++d->shader_count;d->allocated+=s->allocated;
 e=d->cfg.ops->create(d->cfg.priv,s->allocated,&s->handle);
 /* A recycled handle would alias existing owned storage. Never attempt to
  * unpin/destroy it, since the provider has violated object identity. */
 if(handle_aliases(d,s->handle,&s->handle))return shader_quarantine(d,s);
 if(e==EOWNERDEAD){shader_quarantine(d,s);return e;}
 if(e)goto fail;
 if(!s->handle){e=EPROTO;goto fail;}
 e=d->cfg.ops->pin(d->cfg.priv,s->handle);
 if(e==EOWNERDEAD){shader_quarantine(d,s);return e;}
 if(e)goto fail;
 s->pinned=true;
 e=stage_lease(d,s,true);if(e)goto fail;
 e=stage_lease(d,s,false);if(e)goto fail;
 e=remesa_shader_cookie_create(&d->shader_cookies,&s->token);if(e)goto fail;
 s->staged=s->owner=true;d->shader_generation=s->request.generation;
 *out=s->token;
 /* Owned, byte-for-byte verified GEM staging; still no executable VS/PDS,
  * physical register mapping, qualified termination or execution address. */
 return remesa_shader_image_validate_execution(s->image);
fail:
 if(s->quarantined)return EOWNERDEAD;
 return shader_dispose(d,&d->shaders)?EOWNERDEAD:e;
}
static int prepare_shader_programs(void *data,
 const struct remesa_shader_draw_programs *programs,void **out)
{return prepare_candidate(data,programs,NULL,out);}
static int submit_shader_programs(void *data,void *target,
 const struct remesa_draw_packet *packet,const struct remesa_shader_draw_inputs *inputs,
 void *token,void **out)
{
 struct persistent *d=data;
 if(!out)return EINVAL;
 if(d) {
  int error=remesa_shader_cookie_output_check(&d->shader_cookies,out,sizeof(*out));
  if(error)return error;
 }
 *out=NULL;
 if(!d || !target || !packet || !inputs)return EINVAL;
 struct shader_upload **link=shader_link(d,token);
 if(!link || !(*link)->owner)return EINVAL;
 if(d->failed || (*link)->quarantined)return EIO;
 /* No SGX VA exists for this backing. Reject before any historical dispatch,
  * target access, job hold, cache operation or device action. */
 return remesa_shader_image_validate_execution((*link)->image);
}
static void destroy_shader_programs(void *data,void *token)
{
 struct persistent *d=data;
 struct shader_upload **link=shader_link(d,token);
 if(!link || !(*link)->owner)return;
 (*link)->owner=false;(void)shader_dispose(d,link);
}
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
 e=remesa_shader_cookie_output_check(&d->shader_cookies,out,sizeof(*out));
 if(e)return e;
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
 e=d->cfg.ops->create(d->cfg.priv,n,&b->handle);
 /* The shared provider must not recycle an identity that is still owned by
  * a CPU-only shader candidate or another resource. Check even on an error:
  * cleanup of a duplicated handle could destroy someone else's backing. */
 if(handle_aliases(d,b->handle,&b->handle)) {e=EOWNERDEAD;goto fail;}
 if(e)goto fail;
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
 e=remesa_shader_cookie_output_check(&d->shader_cookies,out,sizeof(*out));
 if(e)return e;
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
 if(out) {
  int error=remesa_shader_cookie_output_check(&d->shader_cookies,out,sizeof(*out));
  if(error)return error;
 }
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
static const struct remesa_device_ops device_ops={
 .allocate=allocate,.destroy_bo=destroy_bo,.map=map_bo,.unmap=unmap_bo,
 .submit=submit,.wait=wait_job,.destroy_fence=destroy_fence,
 .prepare_shader_programs=prepare_shader_programs,
 .submit_shader_programs=submit_shader_programs,
 .destroy_shader_programs=destroy_shader_programs,
};
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
 if(d->shaders)return EBUSY;
 remesa_shader_cookie_pool_finish(&d->shader_cookies);
 free(d);memset(device,0,sizeof(*device));return 0;
}
static bool overlaps(const void *a,size_t an,const void *b,size_t bn)
{
 uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
 if(an>UINTPTR_MAX-x || bn>UINTPTR_MAX-y)return true;
 return x<=y?y-x<an:x-y<bn;
}
static bool geometry_output_overlap(struct shader_upload *s,const void *out,size_t bytes)
{
 if(!s->geometry.snapshot)return false;
 struct remesa_shader_draw_inputs inputs;
 struct remesa_draw_plan plan;
 if(remesa_shader_draw_snapshot_inspect(s->geometry.snapshot,&inputs,&plan))return true;
 return overlaps(out,bytes,inputs.vertex_data,inputs.vertex_bytes) ||
  (inputs.index_bytes && overlaps(out,bytes,inputs.index_data,inputs.index_bytes));
}
/* Protect every retained identity and immutable payload, not only the queried
 * shader. A metadata query must not mutate another token's program/VB/IB. */
int remesa_persistent_shader_output_check(const struct remesa_device *device,
                                         const void *out,size_t bytes)
{
 if(!device||device->ops!=&device_ops||!device->priv||!out||!bytes)return EINVAL;
 struct persistent *d=device->priv;
 if(overlaps(out,bytes,device,sizeof(*device))||
    overlaps(out,bytes,d,sizeof(*d)))return EINVAL;
 int error=remesa_shader_cookie_output_check(&d->shader_cookies,out,bytes);
 if(error)return error;
 for(struct shader_upload *s=d->shaders;s;s=s->next) {
  struct remesa_shader_image_view view;
  if(overlaps(out,bytes,s,sizeof(*s)))return EINVAL;
  error=remesa_shader_image_inspect(s->image,&view);if(error)return error;
  if(overlaps(out,bytes,view.programs,sizeof(*view.programs))||
     overlaps(out,bytes,view.fragment.bytes,view.fragment.byte_length)||
     overlaps(out,bytes,view.vertex.bytes,view.vertex.byte_length)||
     geometry_output_overlap(s,out,bytes))return EINVAL;
  for(unsigned i=0;i<2;i++)if(s->pds.data[i]) {
   struct remesa_shader_storage_view data;
   error=remesa_shader_storage_inspect(s->pds.data[i],&data);if(error)return error;
   if(overlaps(out,bytes,data.bytes,data.byte_length))return EINVAL;
  }
 }
 return 0;
}
int remesa_persistent_shader_snapshot(const struct remesa_device *device,void *token,
 struct remesa_shader_request *request,struct remesa_persistent_shader_backing *backing)
{
 if(!device || device->ops!=&device_ops || !device->priv || !request || !backing)
  return EINVAL;
 struct persistent *d=device->priv;
 struct shader_upload **link=shader_link(d,token);
 if(!link || !(*link)->owner)return EINVAL;
 struct shader_upload *s=*link;
 if(d->failed || s->quarantined)return EOWNERDEAD;
 if(!s->staged || !s->pinned || !s->handle || s->leased)return EPROTO;
 struct remesa_shader_image_view view;
 int e=remesa_shader_image_inspect(s->image,&view);if(e)return e;
 if(overlaps(request,sizeof(*request),backing,sizeof(*backing)))return EINVAL;
 e=remesa_persistent_shader_output_check(device,request,sizeof(*request));if(e)return e;
 e=remesa_persistent_shader_output_check(device,backing,sizeof(*backing));if(e)return e;
 if(remesa_shader_request_validate(&s->request,d->cfg.context,s->request.generation)
    !=REMESA_SHADER_REQUEST_OK)return EPROTO;
 e=remesa_shader_image_validate_request(s->image,&s->request);if(e)return e;
 struct remesa_persistent_shader_backing v={
  .owner=d->cfg.context,.generation=s->request.generation,.handle=s->handle,
  .allocation_bytes=s->allocated,.pinned=s->pinned,.staged=s->staged,
  .quarantined=s->quarantined
 };
 *request=s->request;*backing=v;return 0;
}

int remesa_persistent_shader_bindings(const struct remesa_device *device,void *token,
 struct remesa_shader_request *request,struct remesa_shader_bindings *bindings)
{
 struct remesa_shader_request r;
 struct remesa_persistent_shader_backing backing;
 struct remesa_shader_bindings b;
 if(!request || !bindings)return EINVAL;
 int e=remesa_persistent_shader_snapshot(device,token,&r,&backing);if(e)return e;
 struct persistent *d=device->priv;
 struct shader_upload *s=*shader_link(d,token);
 struct remesa_shader_image_view view;
 e=remesa_shader_image_inspect(s->image,&view);if(e)return e;
 if(overlaps(request,sizeof(*request),bindings,sizeof(*bindings)))return EINVAL;
 e=remesa_persistent_shader_output_check(device,request,sizeof(*request));if(e)return e;
 e=remesa_persistent_shader_output_check(device,bindings,sizeof(*bindings));if(e)return e;
 if(backing.allocation_bytes>UINT32_MAX)return EOVERFLOW;
 if(remesa_shader_bindings_build(&b,&r,backing.owner,backing.generation,
    backing.handle,(uint32_t)backing.allocation_bytes)!=REMESA_SHADER_REQUEST_OK)return EPROTO;
 *request=r;*bindings=b;return 0;
}

int remesa_persistent_shader_stage_draw(struct remesa_device *device,void *token,
 const struct remesa_shader_draw_inputs *inputs)
{
 struct remesa_shader_request request;
 struct remesa_persistent_shader_backing backing;
 int e=remesa_persistent_shader_snapshot(device,token,&request,&backing);if(e)return e;
 struct persistent *d=device->priv;
 struct shader_upload *s=*shader_link(d,token);
 if(s->geometry.snapshot)return EBUSY;
 if(!inputs)return EINVAL;
 struct remesa_shader_image_view image;
 e=remesa_shader_image_inspect(s->image,&image);if(e)return e;
 if(inputs->position_input_slot!=image.programs->pipeline.position_input_slot)return EINVAL;
 if(inputs->preserve_color && device->framebuffer_mode!=REMESA_FRAMEBUFFER_PRESERVE)
  return ENOTSUP;
 struct remesa_shader_draw_snapshot *snapshot=NULL;
 e=remesa_shader_draw_snapshot_create(inputs,&snapshot);if(e)return e;
 struct remesa_shader_draw_inputs owned;
 struct remesa_draw_plan plan;
 e=remesa_shader_draw_snapshot_inspect(snapshot,&owned,&plan);
 if(e){remesa_shader_draw_snapshot_destroy(snapshot);return e;}
 /* Four-byte padding is deterministic host serialization only. Neither an
  * instruction boundary nor a physical vertex/index-fetch unit is inferred. */
 if(owned.index_bytes && owned.vertex_bytes>SIZE_MAX-3) {
  remesa_shader_draw_snapshot_destroy(snapshot);return EOVERFLOW;
 }
 size_t index_offset=owned.index_bytes?(owned.vertex_bytes+3)&~(size_t)3:0;
 if(index_offset>REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES ||
    owned.index_bytes>REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES-index_offset ||
    owned.vertex_bytes>REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES) {
  remesa_shader_draw_snapshot_destroy(snapshot);return E2BIG;
 }
 size_t length=owned.index_bytes?index_offset+owned.index_bytes:owned.vertex_bytes;
 if(length>SIZE_MAX-(PAGE_BYTES-1)) {
  remesa_shader_draw_snapshot_destroy(snapshot);return EOVERFLOW;
 }
 size_t allocated=(length+PAGE_BYTES-1)&~(size_t)(PAGE_BYTES-1);
 if(allocated>d->cfg.byte_budget-d->allocated) {
  remesa_shader_draw_snapshot_destroy(snapshot);return ENOSPC;
 }
 s->geometry.snapshot=snapshot;s->geometry.allocated=allocated;
 s->geometry.vertex_bytes=owned.vertex_bytes;s->geometry.index_offset=index_offset;
 s->geometry.index_bytes=owned.index_bytes;d->allocated+=allocated;
 e=d->cfg.ops->create(d->cfg.priv,allocated,&s->geometry.handle);
 if(handle_aliases(d,s->geometry.handle,&s->geometry.handle))return shader_quarantine(d,s);
 if(e==EOWNERDEAD)return shader_quarantine(d,s);
 if(e)goto fail;
 if(!s->geometry.handle){e=EPROTO;goto fail;}
 e=d->cfg.ops->pin(d->cfg.priv,s->geometry.handle);
 if(e==EOWNERDEAD)return shader_quarantine(d,s);
 if(e)goto fail;
 s->geometry.pinned=true;
 e=geometry_lease(d,s,true);if(e)goto fail;
 e=geometry_lease(d,s,false);if(e)goto fail;
 s->geometry.staged=true;return 0;
fail:
 if(s->quarantined)return EOWNERDEAD;
 return geometry_dispose(d,s)?EOWNERDEAD:e;
}

int remesa_persistent_shader_draw_snapshot(const struct remesa_device *device,void *token,
 struct remesa_shader_draw_inputs *out,struct remesa_draw_plan *plan)
{
 struct remesa_shader_request request;
 struct remesa_persistent_shader_backing backing;
 int error=remesa_persistent_shader_snapshot(device,token,&request,&backing);
 if(error)return error;
 if(!out||!plan||overlaps(out,sizeof(*out),plan,sizeof(*plan)))return EINVAL;
 error=remesa_persistent_shader_output_check(device,out,sizeof(*out));if(error)return error;
 error=remesa_persistent_shader_output_check(device,plan,sizeof(*plan));if(error)return error;
 struct shader_upload *s=*shader_link(device->priv,token);
 if(!s->geometry.snapshot)return EAGAIN;
 if(!s->geometry.staged||!s->geometry.pinned||!s->geometry.handle||
    s->geometry.leased)return EPROTO;
 return remesa_shader_draw_snapshot_inspect(s->geometry.snapshot,out,plan);
}
int remesa_persistent_shader_draw_backing(const struct remesa_device *device,void *token,
 struct remesa_persistent_draw_backing *out)
{
 if(!out)return EINVAL;
 struct remesa_shader_request request;
 struct remesa_persistent_shader_backing program;
 int e=remesa_persistent_shader_snapshot(device,token,&request,&program);if(e)return e;
 struct persistent *d=device->priv;
 struct shader_upload *s=*shader_link(d,token);
 if(!s->geometry.snapshot)return EAGAIN;
 if(!s->geometry.staged || !s->geometry.pinned || !s->geometry.handle ||
    s->geometry.leased)return EPROTO;
 struct remesa_shader_image_view image;
 e=remesa_shader_image_inspect(s->image,&image);if(e)return e;
 e=remesa_persistent_shader_output_check(device,out,sizeof(*out));if(e)return e;
 struct remesa_persistent_draw_backing value={
  .owner=program.owner,.generation=program.generation,.handle=s->geometry.handle,
  .allocation_bytes=s->geometry.allocated,.vertex_bytes=s->geometry.vertex_bytes,
  .index_offset=s->geometry.index_offset,.index_bytes=s->geometry.index_bytes,
  .pinned=s->geometry.pinned,.staged=s->geometry.staged
 };
 *out=value;return 0;
}

int remesa_persistent_shader_stage_pds_data(struct remesa_device *device,void *token,
 const struct remesa_pds_host_data_view *data)
{
 struct remesa_shader_request request;
 struct remesa_persistent_shader_backing program;
 int e=remesa_persistent_shader_snapshot(device,token,&request,&program);if(e)return e;
 struct persistent *d=device->priv;
 struct shader_upload *s=*shader_link(d,token);
 if(s->pds.data[0]||s->pds.data[1]||s->pds.handle)return EBUSY;
 if(!data||2*sizeof(*data)>UINTPTR_MAX-(uintptr_t)data)return EINVAL;
 for(unsigned i=0;i<2;i++) {
  const struct remesa_shader_storage_view *v=&data[i].storage;
  if(v->stage!=(i?REMESA_IR_FRAGMENT:REMESA_IR_VERTEX)||
     v->format!=REMESA_SHADER_STORAGE_HOST_PDS_DATA||!v->bytes||
     !v->byte_length||v->byte_length>1024||v->byte_length%4||
     v->byte_alignment!=16||data[i].binding.owner_cookie!=program.owner||
     data[i].binding.bytes!=v->byte_length)return EINVAL;
  e=remesa_persistent_shader_output_check(device,v->bytes,v->byte_length);if(e)return e;
 }
 size_t offset=(data[0].storage.byte_length+15)&~(size_t)15;
 size_t used=offset+data[1].storage.byte_length;
 size_t allocated=(used+PAGE_BYTES-1)&~(size_t)(PAGE_BYTES-1);
 if(allocated>d->cfg.byte_budget-d->allocated)return ENOSPC;
 for(unsigned i=0;i<2;i++) {
  const struct remesa_shader_storage_view *v=&data[i].storage;
  const struct remesa_shader_storage_request r={.stage=v->stage,.format=v->format,
   .bytes=v->bytes,.byte_length=v->byte_length,.byte_alignment=v->byte_alignment};
  e=remesa_shader_storage_create(&r,&s->pds.data[i]);if(e)goto fail;
  s->pds.length[i]=v->byte_length;
 }
 s->pds.offset[1]=offset;s->pds.allocated=allocated;d->allocated+=allocated;
 e=d->cfg.ops->create(d->cfg.priv,allocated,&s->pds.handle);
 if(handle_aliases(d,s->pds.handle,&s->pds.handle))return shader_quarantine(d,s);
 if(e==EOWNERDEAD)return shader_quarantine(d,s);
 if(e)goto fail;
 if(!s->pds.handle){e=EPROTO;goto fail;}
 e=d->cfg.ops->pin(d->cfg.priv,s->pds.handle);
 if(e==EOWNERDEAD)return shader_quarantine(d,s);
 if(e)goto fail;
 s->pds.pinned=true;
 e=pds_lease(d,s,true);if(e)goto fail;
 e=pds_lease(d,s,false);if(e)goto fail;
 s->pds.staged=true;return 0;
fail:
 if(s->quarantined)return EOWNERDEAD;
 return pds_dispose(d,s)?EOWNERDEAD:e;
}
int remesa_persistent_shader_pds_backing(const struct remesa_device *device,void *token,
 struct remesa_persistent_pds_backing *out)
{
 if(!out)return EINVAL;
 struct remesa_shader_request request;
 struct remesa_persistent_shader_backing program;
 int e=remesa_persistent_shader_snapshot(device,token,&request,&program);if(e)return e;
 struct shader_upload *s=*shader_link(device->priv,token);
 if(!s->pds.data[0]||!s->pds.data[1])return EAGAIN;
 if(!s->pds.staged||!s->pds.pinned||!s->pds.handle||s->pds.leased)return EPROTO;
 e=remesa_persistent_shader_output_check(device,out,sizeof(*out));if(e)return e;
 struct remesa_persistent_pds_backing result={.owner=program.owner,
  .generation=program.generation,.handle=s->pds.handle,.allocation_bytes=s->pds.allocated,
  .data_offset={s->pds.offset[0],s->pds.offset[1]},
  .data_bytes={s->pds.length[0],s->pds.length[1]},.pinned=true,.staged=true};
 *out=result;return 0;
}
int remesa_persistent_shader_verify(struct remesa_device *device,void *token)
{
 struct remesa_shader_request r;
 struct remesa_persistent_shader_backing backing;
 int e=remesa_persistent_shader_snapshot(device,token,&r,&backing);if(e)return e;
 struct persistent *d=device->priv;
 struct shader_upload *s=*shader_link(d,token);
 struct remesa_shader_image_view view;
 e=remesa_shader_image_inspect(s->image,&view);if(e)return e;
 e=stage_lease(d,s,false);
 if(e==EPROTO)return shader_quarantine(d,s);
 if(!e && s->geometry.snapshot) {
  if(!s->geometry.staged || !s->geometry.pinned || !s->geometry.handle ||
     s->geometry.leased)return EPROTO;
  e=geometry_lease(d,s,false);
  if(e==EPROTO)return shader_quarantine(d,s);
 }
 if(!e && s->pds.data[0]) {
  if(!s->pds.staged||!s->pds.pinned||!s->pds.handle||s->pds.leased)return EPROTO;
  e=pds_lease(d,s,false);
  if(e==EPROTO)return shader_quarantine(d,s);
 }
 return e;
}
int remesa_persistent_prepare_vertex_candidate(struct remesa_device *device,
 const struct remesa_shader_draw_programs *programs,
 const struct remesa_usse_vertex_mapping *mapping,void **out)
{
 if(!device||device->ops!=&device_ops||!device->priv||!programs||!mapping||!out)return EINVAL;
 if(overlaps(out,sizeof(*out),programs,sizeof(*programs))||
    overlaps(out,sizeof(*out),mapping,sizeof(*mapping)))return EINVAL;
 int e=remesa_persistent_shader_output_check(device,out,sizeof(*out));if(e)return e;
 if(*out)return EBUSY;
 return prepare_candidate(device->priv,programs,mapping,out);
}
