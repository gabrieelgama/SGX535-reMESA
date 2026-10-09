/* SPDX-License-Identifier: MIT */
#include "remesa_screen.h"
#include <errno.h>
#ifdef REMESA_WITH_MESA_HEADERS
#include "remesa_resource_ref.h"
#include "remesa_device.h"
#include "remesa_resource_layout.h"
#include "remesa_draw.h"
#include "remesa_fixed_state.h"
#include "remesa_render_pass.h"
#include "remesa_sampler_state.h"
#include "remesa_texture_binding.h"
#include "remesa_nir_uniform.h"
#ifdef REMESA_WITH_NIR
#include "remesa_nir.h"
#include "nir.h"
#include "util/ralloc.h"
#endif
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* Serialized API entry; children retain the backend until final teardown. */
struct screen {
 struct pipe_screen base; struct remesa_device device; unsigned refs; bool closing;
 int (*release_device)(struct remesa_device *);int release_error;bool release_pending;
 uint64_t resource_cookie;
};
static void screen_hold(struct screen *sc) { ++sc->refs; }
static void screen_drop(struct screen *sc)
{
 if(!--sc->refs) {
  int error=sc->release_device ? sc->release_device(&sc->device) : 0;
  if(error) {
   /* Backend still owns pending/quarantined resources. Retain the owner. */
   sc->refs=1;sc->release_pending=true;sc->release_error=error;return;
  }
  free(sc);
 }
}
struct resource {
 struct pipe_resource base; void *bo; size_t bytes; unsigned maps;unsigned stride;
 struct pipe_fence_handle *writer; int write_error; bool has_contents;
 struct remesa_render_pass pass;struct remesa_pass_ticket ticket;
};
enum fixed_kind { FIXED_BLEND,FIXED_DSA,FIXED_RASTER,FIXED_COUNT };
struct fixed { struct fixed *next; enum fixed_kind kind; bool deleted,scissor,depth_clip,full_clip; };
struct elements { struct elements *next; struct pipe_vertex_element position; bool deleted; };
struct vertex { struct vertex *next; unsigned input_slot; bool deleted; };
struct fragment { struct fragment *next; uint32_t argb; bool deleted,uses_uniform;
 struct remesa_uniform_color uniform; };
struct context {
 struct pipe_context base; struct pipe_vertex_buffer vb; struct pipe_vertex_element position;
 struct pipe_resource *target; bool position_bound,pipeline_bound,diagnostic_discard; uint32_t argb;
 int error; struct pipe_fence_handle *last; uint64_t cpu_clears,cpu_copies;
 struct fragment *fragments,*bound_fragment;
 bool invalid_viewport,invalid_scissor,invalid_constants,sample_enabled; uint32_t constant_rejections[2];
 struct vertex *vertices,*bound_vertex; bool vertex_required,viewport_set;
 struct elements *elements,*bound_elements;bool invalid_elements;
 struct pipe_viewport_state viewport;
 struct fixed *fixed_states,*bound_fixed[FIXED_COUNT];bool invalid_fixed[FIXED_COUNT],scissor_set;
 struct remesa_sampler_bank *samplers;
 struct remesa_texture_binding_bank *textures;
 struct pipe_resource *fragment_constants;unsigned constant_offset,constant_size;
 void *constant_snapshot;
};
struct pipe_fence_handle { unsigned refs; struct screen *screen; void *backend; };
struct transfer { struct pipe_transfer base; bool write; };
static struct screen *s(struct pipe_screen *p) { return (struct screen *)p; }
static struct context *c(struct pipe_context *p) { return (struct context *)p; }
static struct resource *r(struct pipe_resource *p) { return (struct resource *)p; }
static void fence_ref(struct pipe_screen *,struct pipe_fence_handle **,struct pipe_fence_handle *);
static void set_capabilities(struct screen *sc)
{
 struct pipe_caps caps={0}; struct pipe_shader_caps shader={0};
 /* Narrow Gallium contract only; this does not advertise an OpenGL version. */
 caps.graphics=!sc->device.resource_only_backend;
 caps.accelerated=!sc->device.resource_only_backend&&sc->device.qualified_hardware_backend ? 1 : 0;
 caps.max_render_targets=1; caps.max_texture_2d_size=32;
 caps.max_vertex_buffers=1; caps.max_vertex_attrib_stride=16;
 caps.max_viewports=1; caps.supported_prim_modes=1u<<MESA_PRIM_TRIANGLES;
 /* Position is a built-in VS output, not an interpolated user varying.
  * Fragment coordinates/interpolation are rejected by the NIR recognizer;
  * their convention flags must not advertise unqualified GPU semantics. */
 caps.max_varyings=0; caps.vendor_id=0x8086; caps.device_id=0x8108;
 caps.viewport_transform_lowered=true;
#ifdef REMESA_WITH_NIR
 if(!sc->device.resource_only_backend) {
  shader.max_instructions=1; shader.max_inputs=1; shader.max_outputs=1;
  shader.supported_irs=1u<<PIPE_SHADER_IR_NIR;
 }
#endif
 /* Gallium places tessellation and geometry stages between VS and FS.
  * Advertising those stages makes Mesa call unsupported lifecycle hooks. */
 memcpy((void *)&sc->base.shader_caps[PIPE_SHADER_VERTEX],&shader,sizeof(shader));
 shader.max_inputs=0;
 memcpy((void *)&sc->base.shader_caps[PIPE_SHADER_FRAGMENT],&shader,sizeof(shader));
 memcpy((void *)&sc->base.caps,&caps,sizeof(caps));
}
static const char *name(struct pipe_screen *p) { return s(p)->device.resource_only_backend ? "reMESA GEM resources (DRAW DISABLED)" : s(p)->device.offline_test_backend ? "reMESA recording backend (NO SGX)" : "reMESA SGX535"; }
static const char *vendor(struct pipe_screen *p) { (void)p;return "reMESA"; }
#ifdef REMESA_WITH_NIR
static const struct nir_shader_compiler_options *compiler_options(struct pipe_screen *p,enum pipe_shader_type stage)
{
 /* No general opcode lowering or shader capability is claimed. Keep NIR in
  * its original form for the strict semantic recognizers; unsupported IR is
  * rejected by create_vs/fs_state, not silently rewritten into a profile. */
 static const nir_shader_compiler_options options={0};
 if(s(p)->closing||(stage!=PIPE_SHADER_VERTEX&&stage!=PIPE_SHADER_FRAGMENT))return NULL;
 return &options;
}
#endif

static bool format(struct pipe_screen *p,enum pipe_format f,enum pipe_texture_target t,unsigned samples,unsigned storage,unsigned bind)
{
 (void)p;if(samples>1 || storage>1) return false;
 if(t==PIPE_BUFFER) return !(bind & ~(PIPE_BIND_VERTEX_BUFFER|PIPE_BIND_INDEX_BUFFER|PIPE_BIND_CONSTANT_BUFFER)) &&
  (f==PIPE_FORMAT_R8_UNORM || (!(bind&PIPE_BIND_CONSTANT_BUFFER)&&remesa_position_format_bytes(f)));
 if(t!=PIPE_TEXTURE_2D)return false;
 /* Transfer-only storage is not sampler or render capability. Real SGX render
  * attachments remain the exact BGRA32 profile; sampler binds are unsupported. */
 if(!bind)return f==PIPE_FORMAT_B8G8R8A8_UNORM||f==PIPE_FORMAT_R8G8B8A8_UNORM;
 return f==PIPE_FORMAT_B8G8R8A8_UNORM && bind==PIPE_BIND_RENDER_TARGET;
}
static struct pipe_resource *create_resource(struct pipe_screen *p,const struct pipe_resource *t)
{
 struct resource *b;size_t bytes;unsigned stride=0;
 if(s(p)->closing || !t || !format(p,t->format,t->target,t->nr_samples,t->nr_storage_samples,t->bind) || t->last_level ||
    t->array_size!=1 || t->depth0!=1 || t->flags) return NULL;
 if(t->target==PIPE_BUFFER) {
  uint64_t budget=s(p)->device.resource_limits.max_resource_bytes;
  if(!budget)budget=UINT64_C(64)*1024*1024;
  if(!t->width0 || t->height0!=1 || t->width0>budget) return NULL;
  bytes=t->width0;
 }
 else {
  if(t->bind) { if(t->width0!=32 || t->height0!=32)return NULL; }
  struct remesa_resource_layout layout;
  /* The exact physical profile requires tight rows; transfer-only storage
   * follows explicit backend alignment and independently bounded capacity. */
  struct remesa_resource_limits limits=s(p)->device.resource_limits;
  if(t->bind&&limits.row_alignment>1) {
   if((limits.row_alignment&(limits.row_alignment-1))||
      (128u&(limits.row_alignment-1)))return NULL;
  }
  if(remesa_resource_layout(&limits,t->width0,t->height0,4,&layout))return NULL;
  bytes=layout.bytes;stride=layout.stride;
 }
 if(s(p)->resource_cookie==UINT64_MAX)return NULL;
 b=calloc(1,sizeof(*b));if(!b) return NULL;b->base=*t;b->base.screen=p;
 remesa_render_pass_init(&b->pass,++s(p)->resource_cookie);
 b->base.reference.count=1;b->bytes=bytes;b->stride=stride;
 if(s(p)->device.ops->allocate(s(p)->device.priv,bytes,&b->bo) || !b->bo) {
  if(b->bo) s(p)->device.ops->destroy_bo(s(p)->device.priv,b->bo);
  free(b);return NULL;
 }
 screen_hold(s(p));return &b->base;
}
static void destroy_resource(struct pipe_screen *p,struct pipe_resource *b)
{
 fence_ref(p,&r(b)->writer,NULL);
 s(p)->device.ops->destroy_bo(s(p)->device.priv,r(b)->bo);free(b);screen_drop(s(p));
}
static void fence_ref(struct pipe_screen *p,struct pipe_fence_handle **dst,struct pipe_fence_handle *src)
{
 struct pipe_fence_handle *old=*dst;(void)p;
 if(src) __atomic_add_fetch(&src->refs,1,__ATOMIC_RELAXED);
 *dst=src;
 if(old && !__atomic_sub_fetch(&old->refs,1,__ATOMIC_ACQ_REL)) {
  struct screen *owner=old->screen;
  owner->device.ops->destroy_fence(owner->device.priv,old->backend);free(old);screen_drop(owner);
 }
}
static bool finish(struct pipe_screen *p,struct pipe_context *ctx,struct pipe_fence_handle *f,uint64_t ns)
{ (void)ctx;return f && f->screen==s(p) && !s(p)->device.ops->wait(s(p)->device.priv,f->backend,ns,true); }
/* Contexts sharing a screen/resource must be externally serialized. A pending
 * writer belongs to the resource, not whichever context currently binds it.
 * Conservatively require CPU publication for both host access and reuse; no
 * inferred GPU-only visibility or implicit failure recovery is permitted. */
static int resource_sync(struct pipe_screen *p,struct resource *b,uint64_t timeout)
{
 int error;
 if(b->write_error) return b->write_error;
 if(!b->writer) return 0;
 error=s(p)->device.ops->wait(s(p)->device.priv,b->writer->backend,timeout,true);
 if(!error) {
  /* wait(cpu_visible=true) must authenticate this backend fence and attest
   * retirement/publication. These are not independently synthesized events. */
  const struct remesa_pass_completion proof={true,true,true};
  error=remesa_render_pass_finish(&b->pass,&b->ticket,&proof);
  if(!error)fence_ref(p,&b->writer,NULL);
 }
 if(error && error!=EAGAIN && error!=ETIMEDOUT && error!=EBUSY&&error!=EINTR) {
  b->write_error=error;remesa_render_pass_fail(&b->pass,&b->ticket,error);
 }
 return error;
}
static void *map(struct pipe_context *p,struct pipe_resource *b,unsigned level,unsigned usage,const struct pipe_box *box,struct pipe_transfer **out)
{
 struct transfer *t;void *ptr=NULL;size_t offset;struct screen *sc=s(p->screen);
 if(out) *out=NULL;
 if(!b || b->screen!=p->screen || !box || !out || level || !(usage & (PIPE_MAP_READ|PIPE_MAP_WRITE)) ||
    usage & ~(PIPE_MAP_READ|PIPE_MAP_WRITE|PIPE_MAP_DONTBLOCK) || box->x<0 || box->width<=0 || r(b)->maps) return NULL;
 if(b->target==PIPE_BUFFER) {
  if(box->y || box->z || box->height!=1 || box->depth!=1 ||
     (uint64_t)box->x+box->width>b->width0) return NULL;
  offset=box->x;
 } else {
  if(box->y<0 || box->z || box->height<=0 || box->depth!=1 ||
     (uint64_t)box->x+box->width>b->width0 || (uint64_t)box->y+box->height>b->height0) return NULL;
  offset=(size_t)box->y*r(b)->stride+(size_t)box->x*4;
 }
 if(resource_sync(p->screen,r(b),(usage & PIPE_MAP_DONTBLOCK) ? 0 : UINT64_MAX)) return NULL;
 t=calloc(1,sizeof(*t));if(!t) return NULL;
 int mapped=sc->device.ops->map(sc->device.priv,r(b)->bo,(usage & PIPE_MAP_WRITE)!=0,&ptr);
 if(mapped || !ptr) {
  if(!mapped||ptr)sc->device.ops->unmap(sc->device.priv,r(b)->bo,(usage & PIPE_MAP_WRITE)!=0);
  free(t);return NULL;
 }
 remesa_resource_reference(&t->base.resource,b);t->base.level=level;t->base.usage=usage;t->base.box=*box;
 t->base.stride=r(b)->stride;t->base.layer_stride=r(b)->bytes;
 t->write=(usage & PIPE_MAP_WRITE)!=0;++r(b)->maps;*out=&t->base;return (unsigned char *)ptr+offset;
}
static void unmap(struct pipe_context *p,struct pipe_transfer *pt)
{
 struct transfer *t=(struct transfer *)pt;struct screen *sc=s(p->screen);
 sc->device.ops->unmap(sc->device.priv,r(pt->resource)->bo,t->write);
 if(t->write){
  int error=remesa_render_pass_cpu_write(&r(pt->resource)->pass);
  if(error)r(pt->resource)->write_error=error;
  else r(pt->resource)->has_contents=true;
 }
 --r(pt->resource)->maps;
 remesa_resource_reference(&pt->resource,NULL);free(t);
}
/* CPU upload into owned backing, not GPU texture sampling or a clear. These
 * entry points use exactly the same writer/lease gate as normal transfers. */
static void buffer_subdata(struct pipe_context *p,struct pipe_resource *b,unsigned usage,
                          unsigned offset,unsigned size,const void *data)
{
 struct context *ctx=c(p);struct pipe_transfer *t=NULL;void *dst;
 if(s(p->screen)->closing){ctx->error=ESHUTDOWN;return;}
 if(!b||b->screen!=p->screen||b->target!=PIPE_BUFFER||offset>b->width0||size>b->width0-offset||
    offset>INT32_MAX||size>INT32_MAX||(size&&!data)||usage&~(PIPE_MAP_WRITE|PIPE_MAP_DONTBLOCK)) {ctx->error=EINVAL;return;}
 if(!size){ctx->error=0;return;}
 ctx->error=resource_sync(p->screen,r(b),usage&PIPE_MAP_DONTBLOCK?0:UINT64_MAX);if(ctx->error)return;
 struct pipe_box box={.x=offset,.width=size,.height=1,.depth=1};
 dst=map(p,b,0,usage|PIPE_MAP_WRITE,&box,&t);if(!dst){ctx->error=EIO;return;}
 memmove(dst,data,size);unmap(p,t);ctx->error=0;
}
static void texture_subdata(struct pipe_context *p,struct pipe_resource *b,unsigned level,unsigned usage,
                           const struct pipe_box *box,const void *data,unsigned stride,uintptr_t layer_stride)
{
 struct context *ctx=c(p);struct pipe_transfer *t=NULL;void *dst;size_t row,span;
 if(s(p->screen)->closing){ctx->error=ESHUTDOWN;return;}
 if(!b||b->screen!=p->screen||b->target!=PIPE_TEXTURE_2D||level||!box||box->x<0||box->y<0||box->z||
    box->width<0||box->height<0||box->depth!=1||
    (uint64_t)box->x+box->width>b->width0||(uint64_t)box->y+box->height>b->height0||
    usage&~(PIPE_MAP_WRITE|PIPE_MAP_DONTBLOCK)) {ctx->error=EINVAL;return;}
 if(!box->width||!box->height){ctx->error=0;return;}
 row=(size_t)box->width*4;
 if(!data||stride<row||(box->height>1&&stride>(SIZE_MAX-row)/(box->height-1))){ctx->error=EINVAL;return;}
 span=(size_t)(box->height-1)*stride+row;
 if(layer_stride&&layer_stride<span){ctx->error=EINVAL;return;}
 ctx->error=resource_sync(p->screen,r(b),usage&PIPE_MAP_DONTBLOCK?0:UINT64_MAX);if(ctx->error)return;
 dst=map(p,b,0,usage|PIPE_MAP_WRITE,box,&t);if(!dst){ctx->error=EIO;return;}
 for(unsigned y=0;y<(unsigned)box->height;y++)memcpy((unsigned char *)dst+y*t->stride,(const unsigned char *)data+y*stride,row);
 unmap(p,t);ctx->error=0;
}
/* Gallium permits CPU transfer implementations of this byte-copy contract.
 * No rasterization, format conversion, resolve or GPU work occurs here.
 * Snapshot first so aliasing and overlapping regions are deterministic. */
static void copy_region(struct pipe_context *p,struct pipe_resource *dst,unsigned dst_level,
 unsigned dstx,unsigned dsty,unsigned dstz,struct pipe_resource *src,unsigned src_level,
 const struct pipe_box *box)
{
 struct context *ctx=c(p);struct pipe_transfer *transfer=NULL;
 struct pipe_box dest;void *ptr;unsigned char *snapshot=NULL;size_t row,bytes;
 int error=EINVAL;
 if(s(p->screen)->closing){ctx->error=ESHUTDOWN;return;}
 if(!dst||!src||dst->screen!=p->screen||src->screen!=p->screen||
    dst_level||src_level||dst->format!=src->format||dst->target!=src->target||
    dst->nr_samples>1||src->nr_samples>1||!box||box->x<0||box->y<0||box->z||
    box->width<0||box->height<0||box->depth!=1||dstz||
    dstx>INT32_MAX||dsty>INT32_MAX)goto done;
 if(src->target==PIPE_BUFFER){
  if(box->y||box->height!=1||dsty)goto done;
  row=(size_t)box->width;
 }else if(src->target==PIPE_TEXTURE_2D){
  if(src->format!=PIPE_FORMAT_B8G8R8A8_UNORM&&src->format!=PIPE_FORMAT_R8G8B8A8_UNORM)goto done;
  if((size_t)box->width>SIZE_MAX/4)goto done;
  row=(size_t)box->width*4;
 }else goto done;
 if((uint64_t)box->x+box->width>src->width0||
    (uint64_t)box->y+box->height>src->height0||
    (uint64_t)dstx+box->width>dst->width0||
    (uint64_t)dsty+box->height>dst->height0)goto done;
 if(!box->width||!box->height){error=0;goto done;}
 if((size_t)box->height>SIZE_MAX/row){error=EOVERFLOW;goto done;}
 bytes=row*(size_t)box->height;
 if(r(src)->maps||r(dst)->maps){error=EBUSY;goto done;}
 error=resource_sync(p->screen,r(src),UINT64_MAX);if(error)goto done;
 error=resource_sync(p->screen,r(dst),UINT64_MAX);if(error)goto done;
 snapshot=malloc(bytes);if(!snapshot){error=ENOMEM;goto done;}
 ptr=map(p,src,0,PIPE_MAP_READ,box,&transfer);if(!ptr){error=EIO;goto done;}
 for(unsigned y=0;y<(unsigned)box->height;y++)
  memcpy(snapshot+(size_t)y*row,(unsigned char *)ptr+(size_t)y*transfer->stride,row);
 unmap(p,transfer);transfer=NULL;
 dest=*box;dest.x=dstx;dest.y=dsty;
 ptr=map(p,dst,0,PIPE_MAP_WRITE,&dest,&transfer);if(!ptr){error=EIO;goto done;}
 for(unsigned y=0;y<(unsigned)box->height;y++)
  memcpy((unsigned char *)ptr+(size_t)y*transfer->stride,snapshot+(size_t)y*row,row);
 unmap(p,transfer);transfer=NULL;++ctx->cpu_copies;error=0;
 done:
 free(snapshot);ctx->error=error;
}
/* Raw texel CPU clear for transfer storage, explicitly not an SGX clear. */
static void clear_texture(struct pipe_context *p,struct pipe_resource *res,unsigned level,
 const struct pipe_box *box,const void *data)
{
 struct context *ctx=c(p);struct pipe_transfer *transfer=NULL;void *ptr;
 unsigned char pixel[4];int error=EINVAL;
 if(s(p->screen)->closing){ctx->error=ESHUTDOWN;return;}
 if(!res||res->screen!=p->screen||res->target!=PIPE_TEXTURE_2D||level||!box||!data||
    (res->format!=PIPE_FORMAT_B8G8R8A8_UNORM&&res->format!=PIPE_FORMAT_R8G8B8A8_UNORM)||
    box->x<0||box->y<0||box->z||box->width<0||box->height<0||box->depth!=1||
    (uint64_t)box->x+box->width>res->width0||(uint64_t)box->y+box->height>res->height0)goto done;
 if(!box->width||!box->height){error=0;goto done;}
 if(r(res)->maps){error=EBUSY;goto done;}
 error=resource_sync(p->screen,r(res),UINT64_MAX);if(error)goto done;
 memcpy(pixel,data,sizeof(pixel));
 ptr=map(p,res,0,PIPE_MAP_WRITE,box,&transfer);if(!ptr){error=EIO;goto done;}
 for(unsigned y=0;y<(unsigned)box->height;y++)
  for(unsigned x=0;x<(unsigned)box->width;x++)
   memcpy((unsigned char *)ptr+(size_t)y*transfer->stride+(size_t)x*4,pixel,4);
 unmap(p,transfer);++ctx->cpu_clears;error=0;
 done:ctx->error=error;
}
static void set_vb(struct pipe_context *p,unsigned count,const struct pipe_vertex_buffer *v)
{
 struct context *ctx=c(p);remesa_resource_reference(&ctx->vb.buffer.resource,NULL);memset(&ctx->vb,0,sizeof(ctx->vb));
 if(!count) return;
 /* Mesa 25.2 transfers these resource references to the callback. */
 if(count!=1 || !v || v[0].is_user_buffer) {
  ctx->error=ENOTSUP;if(v) for(unsigned i=0;i<count;i++) if(!v[i].is_user_buffer) {
   struct pipe_resource *owned=v[i].buffer.resource;remesa_resource_reference(&owned,NULL);
  }
  return;
 }
 ctx->vb=v[0];if(ctx->vb.buffer.resource && (ctx->vb.buffer.resource->screen!=p->screen||
    ctx->vb.buffer.resource->target!=PIPE_BUFFER||!(ctx->vb.buffer.resource->bind&PIPE_BIND_VERTEX_BUFFER))) {
  remesa_resource_reference(&ctx->vb.buffer.resource,NULL);ctx->error=EINVAL;
 }
}
static void *create_elements(struct pipe_context *p,unsigned count,const struct pipe_vertex_element *e)
{
 struct context *ctx=c(p);struct elements *out;
 if(count!=1 || !e || !remesa_position_format_bytes(e->src_format) || e->vertex_buffer_index ||
    e->instance_divisor || e->dual_slot || !e->src_stride) {
  ctx->invalid_elements=true;ctx->error=ENOTSUP;return NULL;
 }
 out=calloc(1,sizeof(*out));if(!out){ctx->invalid_elements=true;ctx->error=ENOMEM;return NULL;}
 out->position=*e;out->next=ctx->elements;ctx->elements=out;ctx->error=0;return out;
}
static struct elements *find_elements(struct context *ctx,void *handle)
{
 for(struct elements *e=ctx->elements;e;e=e->next)if(e==handle&&!e->deleted)return e;
 return NULL;
}
static void bind_elements(struct pipe_context *p,void *handle)
{
 struct context *ctx=c(p);struct elements *e=find_elements(ctx,handle);
 ctx->bound_elements=e;ctx->position_bound=e!=NULL;ctx->invalid_elements=handle&&!e;
 if(e)ctx->position=e->position;
 ctx->error=ctx->invalid_elements?EINVAL:0;
}
static void delete_elements(struct pipe_context *p,void *handle)
{
 struct context *ctx=c(p);struct elements *e=find_elements(ctx,handle);
 if(!e){ctx->error=EINVAL;return;}
 if(ctx->bound_elements==e){ctx->bound_elements=NULL;ctx->position_bound=false;}
 e->deleted=true;
}
static void framebuffer(struct pipe_context *p,const struct pipe_framebuffer_state *f)
{
 struct context *ctx=c(p);remesa_resource_reference(&ctx->target,NULL);
 if(f&&!f->nr_cbufs&&!f->zsbuf.texture&&!f->resolve&&!f->viewmask){ctx->error=0;return;}
 if(!f || f->nr_cbufs!=1 || f->width!=32 || f->height!=32 || f->samples>1 || f->zsbuf.texture ||
    f->resolve || f->viewmask || f->cbufs[0].level || f->cbufs[0].first_layer || f->cbufs[0].last_layer ||
    !f->cbufs[0].texture || f->cbufs[0].format!=PIPE_FORMAT_B8G8R8A8_UNORM ||
    f->cbufs[0].texture->screen!=p->screen || f->cbufs[0].texture->target!=PIPE_TEXTURE_2D ||
    !(f->cbufs[0].texture->bind&PIPE_BIND_RENDER_TARGET) ||
    f->cbufs[0].texture->format!=PIPE_FORMAT_B8G8R8A8_UNORM || f->cbufs[0].texture->width0!=32 || f->cbufs[0].texture->height0!=32 ||
    f->cbufs[0].texture->depth0!=1 || f->cbufs[0].texture->array_size!=1 || f->cbufs[0].texture->nr_samples>1) { ctx->error=ENOTSUP;return; }
 remesa_resource_reference(&ctx->target,f->cbufs[0].texture);
}
static struct pipe_surface *create_surface(struct pipe_context *p,struct pipe_resource *b,const struct pipe_surface *t)
{
 struct pipe_surface *out;
 if(!b || b->screen!=p->screen || b->target!=PIPE_TEXTURE_2D || !(b->bind&PIPE_BIND_RENDER_TARGET)||
    b->format!=PIPE_FORMAT_B8G8R8A8_UNORM||!t ||
    t->format!=PIPE_FORMAT_B8G8R8A8_UNORM || t->level || t->first_layer || t->last_layer || t->nr_samples>1) return NULL;
 out=calloc(1,sizeof(*out));if(!out) return NULL;*out=*t;
 out->reference.count=1;out->texture=NULL;remesa_resource_reference(&out->texture,b);
 out->context=p;return out;
}
static void surface_destroy(struct pipe_context *p,struct pipe_surface *surface)
{ (void)p;remesa_resource_reference(&surface->texture,NULL);free(surface); }
static void *unsupported_shader(struct pipe_context *p,const struct pipe_shader_state *state)
{
#ifdef REMESA_WITH_NIR
 if(state && state->type==PIPE_SHADER_IR_NIR) ralloc_free(state->ir.nir);
#else
 (void)state;
#endif
 c(p)->error=ENOTSUP;return NULL;
}
#ifdef REMESA_WITH_NIR
static void *create_vertex(struct pipe_context *p,const struct pipe_shader_state *state)
{
 struct vertex *v=NULL;unsigned slot;int err=ENOTSUP;
 if(state&&state->type==PIPE_SHADER_IR_NIR) {
  /* Transform feedback is a separate, unsupported API contract. */
  err=state->stream_output.num_outputs ? ENOTSUP : remesa_nir_position_vertex(state->ir.nir,&slot);
  ralloc_free(state->ir.nir);
  if(!err) {v=calloc(1,sizeof(*v));if(!v)err=ENOMEM;else {v->input_slot=slot;v->next=c(p)->vertices;c(p)->vertices=v;}}
 }
 c(p)->error=err;return v;
}
static struct vertex *find_vertex(struct context *ctx,void *handle)
{
 for(struct vertex *v=ctx->vertices;v;v=v->next)if(v==handle&&!v->deleted)return v;
 return NULL;
}
static void bind_vertex(struct pipe_context *p,void *handle)
{
 struct context *ctx=c(p);ctx->diagnostic_discard=false;ctx->vertex_required=true;ctx->bound_vertex=NULL;
 if(!handle){ctx->error=0;return;}
 struct vertex *v=find_vertex(ctx,handle);if(!v){ctx->error=EINVAL;return;}
 ctx->bound_vertex=v;ctx->error=0;
}
static void delete_vertex(struct pipe_context *p,void *handle)
{
 struct context *ctx=c(p);struct vertex *v=find_vertex(ctx,handle);
 if(!v){ctx->error=EINVAL;return;}
 if(ctx->bound_vertex==v)ctx->bound_vertex=NULL;
 v->deleted=true;
}
static void *create_fragment(struct pipe_context *p,const struct pipe_shader_state *state)
{
 struct fragment *f=NULL;uint32_t argb=0;int err=ENOTSUP;bool uniform=false;
 struct remesa_uniform_color descriptor={0};
 if(state && state->type==PIPE_SHADER_IR_NIR) {
  err=state->stream_output.num_outputs ? ENOTSUP : remesa_nir_constant_fragment(state->ir.nir,&argb);
  if(err==ENOTSUP&&!state->stream_output.num_outputs) {
   err=remesa_nir_uniform_fragment(state->ir.nir,&descriptor);uniform=!err;
  }
  /* Gallium transfers NIR ownership even if compilation is rejected. */
  ralloc_free(state->ir.nir);
  if(!err) {
   f=calloc(1,sizeof(*f));
   if(!f) err=ENOMEM;
   else {f->argb=argb;f->uses_uniform=uniform;f->uniform=descriptor;f->next=c(p)->fragments;c(p)->fragments=f;}
  }
 }
 c(p)->error=err;return f;
}
static struct fragment *find_fragment(struct context *ctx,void *handle)
{
 for(struct fragment *f=ctx->fragments;f;f=f->next)
  if(f==handle && !f->deleted)return f;
 return NULL;
}
static void bind_fragment(struct pipe_context *p,void *handle)
{
 struct context *ctx=c(p);ctx->diagnostic_discard=false;struct fragment *f=find_fragment(ctx,handle);
 ctx->pipeline_bound=false;ctx->bound_fragment=NULL;
 if(!handle){ctx->error=0;return;}
 if(!f){ctx->error=EINVAL;return;}
 ctx->bound_fragment=f;ctx->argb=f->argb;ctx->pipeline_bound=true;ctx->error=0;
}
static void delete_fragment(struct pipe_context *p,void *handle)
{
 struct context *ctx=c(p);struct fragment *f=find_fragment(ctx,handle);
 if(!f){ctx->error=EINVAL;return;}
 if(ctx->bound_fragment==f){ctx->pipeline_bound=false;ctx->bound_fragment=NULL;}
 /* Keep a tombstone until context teardown so a stale handle cannot alias a
  * freshly allocated shader. No NIR/program allocation is retained. */
 f->deleted=true;
}
#endif
/* Pure state admission does not generate new SGX state bytes. These immutable
 * descriptors require semantics equivalent to the unchanged restricted profile.
 * Invalid creates/binds remain latched independently of transient API errors. */
static void *create_fixed(struct pipe_context *p,enum fixed_kind kind,int error,
                         bool scissor,bool depth_clip)
{
 struct context *ctx=c(p);struct fixed *state=NULL;
 if(!error) {
  state=calloc(1,sizeof(*state));if(!state)error=ENOMEM;
  else {state->kind=kind;state->scissor=scissor;state->depth_clip=depth_clip;state->next=ctx->fixed_states;ctx->fixed_states=state;}
 }
 if(error)ctx->invalid_fixed[kind]=true;
 ctx->error=error;return state;
}
static struct fixed *find_fixed(struct context *ctx,void *handle,enum fixed_kind kind)
{
 for(struct fixed *state=ctx->fixed_states;state;state=state->next)
  if(state==handle&&!state->deleted&&state->kind==kind)return state;
 return NULL;
}
static void bind_fixed(struct pipe_context *p,void *handle,enum fixed_kind kind)
{
 struct context *ctx=c(p);struct fixed *state=find_fixed(ctx,handle,kind);
 ctx->bound_fixed[kind]=state;ctx->invalid_fixed[kind]=!state;
 ctx->error=state?0:EINVAL;
}
static void delete_fixed(struct pipe_context *p,void *handle,enum fixed_kind kind)
{
 struct context *ctx=c(p);struct fixed *state=find_fixed(ctx,handle,kind);
 if(!state){ctx->error=EINVAL;return;}
 if(ctx->bound_fixed[kind]==state){ctx->bound_fixed[kind]=NULL;ctx->invalid_fixed[kind]=true;}
 state->deleted=true;
}
static void *create_blend(struct pipe_context *p,const struct pipe_blend_state *state)
{ return create_fixed(p,FIXED_BLEND,remesa_validate_blend(state),false,false); }
static void bind_blend(struct pipe_context *p,void *state){bind_fixed(p,state,FIXED_BLEND);}
static void delete_blend(struct pipe_context *p,void *state){delete_fixed(p,state,FIXED_BLEND);}
static void *create_dsa(struct pipe_context *p,const struct pipe_depth_stencil_alpha_state *state)
{ return create_fixed(p,FIXED_DSA,remesa_validate_dsa(state),false,false); }
static void bind_dsa(struct pipe_context *p,void *state){bind_fixed(p,state,FIXED_DSA);}
static void delete_dsa(struct pipe_context *p,void *state){delete_fixed(p,state,FIXED_DSA);}
static void *create_raster(struct pipe_context *p,const struct pipe_rasterizer_state *state)
{ struct fixed *out=create_fixed(p,FIXED_RASTER,remesa_validate_rasterizer(state),state&&state->scissor,state&&(state->depth_clip_near||state->depth_clip_far));
 if(out)out->full_clip=state->depth_clip_near&&state->depth_clip_far;
 return out; }
static void bind_raster(struct pipe_context *p,void *state){bind_fixed(p,state,FIXED_RASTER);}
static void delete_raster(struct pipe_context *p,void *state){delete_fixed(p,state,FIXED_RASTER);}
static void viewport_state(struct pipe_context *p,unsigned start,unsigned count,const struct pipe_viewport_state *v)
{
 struct context *ctx=c(p);ctx->invalid_viewport=true;
 if(start || count!=1 || !v){ctx->error=ENOTSUP;return;}
 if(v->swizzle_x||v->swizzle_y||v->swizzle_z||v->swizzle_w){ctx->error=ENOTSUP;return;}
 bool identity=true,canonical=true;
 for(unsigned i=0;i<3;i++) {
  if(v->scale[i]!=1||v->translate[i]!=0)identity=false;
  float required=i==2?.5f:16;
  if(v->scale[i]!=required||v->translate[i]!=required)canonical=false;
 }
 if(!identity&&!canonical){ctx->error=ENOTSUP;return;}
 ctx->viewport=*v;ctx->viewport_set=true;ctx->invalid_viewport=false;ctx->error=0;
}
static void scissor_state(struct pipe_context *p,unsigned start,unsigned count,const struct pipe_scissor_state *v)
{
 struct context *ctx=c(p);ctx->invalid_scissor=true;
 if(start || count!=1 || !v || v->minx || v->miny || v->maxx!=32 || v->maxy!=32){ctx->error=ENOTSUP;return;}
 ctx->invalid_scissor=false;ctx->scissor_set=true;ctx->error=0;
}
/* Mesa state tracker/CSO owns these host descriptors. Creating a nearest
 * descriptor does not admit GPU texture sampling. Nonempty bindings remain
 * draw-blocking independently of transient errors on unrelated callbacks. */
static void *create_sampler(struct pipe_context *p,const struct pipe_sampler_state *state)
{
 void *handle=NULL;
 c(p)->error=remesa_sampler_create(c(p)->samplers,state,&handle);
 return handle;
}
static void bind_samplers(struct pipe_context *p,enum pipe_shader_type stage,
 unsigned start,unsigned count,void **states)
{ c(p)->error=remesa_sampler_bind(c(p)->samplers,stage,start,count,states); }
static void delete_sampler(struct pipe_context *p,void *state)
{ c(p)->error=state?remesa_sampler_delete(c(p)->samplers,state):0; }
/* Query only resources from this screen. Backing metadata is not a kernel
 * generation or proof of GPU visibility; quarantined backing is inadmissible. */
static int texture_storage(void *owner,struct pipe_resource *resource,
 struct remesa_texture_storage *out)
{
 struct pipe_screen *screen=owner;
 if(!resource || resource->screen!=screen || !out) return EINVAL;
 if(s(screen)->closing) return ESHUTDOWN;
 if(r(resource)->write_error) return r(resource)->write_error;
 if(r(resource)->maps) return EBUSY;
 if(!r(resource)->bo || !r(resource)->pass.resource_cookie) return ESTALE;
 *out=(struct remesa_texture_storage){r(resource)->pass.resource_cookie,
     r(resource)->bytes,r(resource)->stride};return 0;
}
static struct pipe_sampler_view *create_sampler_view(struct pipe_context *p,
 struct pipe_resource *resource,const struct pipe_sampler_view *view)
{
 struct pipe_sampler_view *out=NULL;
 c(p)->error=remesa_texture_view_create(c(p)->textures,resource,view,&out);
 return out;
}
static void destroy_sampler_view(struct pipe_context *p,struct pipe_sampler_view *view)
{
 c(p)->error=view?remesa_texture_view_destroy(c(p)->textures,view):0;
}
static void set_sampler_views(struct pipe_context *p,enum pipe_shader_type stage,
 unsigned start,unsigned count,unsigned trailing,struct pipe_sampler_view **views)
{ c(p)->error=remesa_texture_views_bind(c(p)->textures,stage,start,count,trailing,views); }
/* Mesa CSO cleanup invokes these even with zero shader/texture caps. */
static void constant_buffer(struct pipe_context *p,enum pipe_shader_type shader,unsigned slot,
 bool take_ownership,const struct pipe_constant_buffer *buffer)
{
 struct context *ctx=c(p);struct pipe_resource *owned=NULL,*incoming=NULL;
 if(buffer&&take_ownership)owned=buffer->buffer;
 /* A borrowed rebind may name the buffer held solely by the old binding.
  * Keep it alive while validation replaces that binding, including failures. */
 if(buffer&&buffer->buffer&&shader==PIPE_SHADER_FRAGMENT&&!slot)
  remesa_resource_reference(&incoming,buffer->buffer);
 bool stage=shader==PIPE_SHADER_VERTEX||shader==PIPE_SHADER_FRAGMENT;
 bool empty=!buffer||(!buffer->buffer&&!buffer->user_buffer&&!buffer->buffer_size&&!buffer->buffer_offset);
 if(!stage||slot>=32){ctx->invalid_constants=true;ctx->error=ENOTSUP;}
 else {
  unsigned index=shader==PIPE_SHADER_VERTEX?0:1;
  if(shader==PIPE_SHADER_FRAGMENT&&!slot) {
   remesa_resource_reference(&ctx->fragment_constants,NULL);
   free(ctx->constant_snapshot);ctx->constant_snapshot=NULL;ctx->constant_size=0;ctx->constant_offset=0;
  }
#ifdef REMESA_WITH_NIR
  if(!empty&&shader==PIPE_SHADER_FRAGMENT&&!slot) {
   int error=ENOTSUP;
   if(buffer->buffer_size>=16&&buffer->buffer_size<=REMESA_UNIFORM_BYTES_MAX) {
    if(buffer->buffer&&!buffer->user_buffer) {
     struct pipe_resource *b=buffer->buffer;
     if(b->screen==p->screen&&b->target==PIPE_BUFFER&&(b->bind&PIPE_BIND_CONSTANT_BUFFER)&&
        buffer->buffer_offset<=r(b)->bytes&&buffer->buffer_size<=r(b)->bytes-buffer->buffer_offset&&
        !(buffer->buffer_offset&15u)) {
      remesa_resource_reference(&ctx->fragment_constants,b);
      ctx->constant_offset=buffer->buffer_offset;error=0;
     }
    } else if(buffer->user_buffer&&!buffer->buffer&&!buffer->buffer_offset) {
     ctx->constant_snapshot=malloc(buffer->buffer_size);
     if(!ctx->constant_snapshot)error=ENOMEM;
     else {memcpy(ctx->constant_snapshot,buffer->user_buffer,buffer->buffer_size);error=0;}
    }
   }
   if(error)ctx->constant_rejections[index]|=1u<<slot;
   else {ctx->constant_size=buffer->buffer_size;ctx->constant_rejections[index]&=~(1u<<slot);}
   ctx->error=error;
  } else
#endif
  if(!empty){ctx->constant_rejections[index]|=1u<<slot;ctx->error=ENOTSUP;}
  else {ctx->constant_rejections[index]&=~(1u<<slot);ctx->error=0;}
 }
 /* Rejected bindings still consume an explicitly transferred reference. */
 remesa_resource_reference(&owned,NULL);
 remesa_resource_reference(&incoming,NULL);
}
#ifdef REMESA_WITH_NIR
/* Specialize the existing constant fragment profile from synchronized CPU
 * bytes. This is not a GPU UBO instruction or a general shader compiler. */
static int fragment_color(struct pipe_context *p,uint32_t *argb)
{
 struct context *ctx=c(p);struct fragment *f=ctx->bound_fragment;
 if(!f||!f->uses_uniform){*argb=ctx->argb;return 0;}
 if(!ctx->constant_size)return ENODATA;
 if(!ctx->fragment_constants)
  return remesa_uniform_color_resolve(&f->uniform,ctx->constant_snapshot,ctx->constant_size,argb);
 struct resource *b=r(ctx->fragment_constants);struct screen *sc=s(p->screen);void *bytes=NULL;
 if(b->maps)return EBUSY;
 int error=resource_sync(p->screen,b,UINT64_MAX);if(error)return error;
 error=sc->device.ops->map(sc->device.priv,b->bo,false,&bytes);
 if(error){if(bytes)sc->device.ops->unmap(sc->device.priv,b->bo,false);return error;}
 error=bytes ? remesa_uniform_color_resolve(&f->uniform,(unsigned char *)bytes+ctx->constant_offset,
                                          ctx->constant_size,argb) : EPROTO;
 sc->device.ops->unmap(sc->device.priv,b->bo,false);return error;
}
#endif
static void stencil_ref(struct pipe_context *p,const struct pipe_stencil_ref reference)
{ (void)reference;c(p)->error=0; /* All admitted DSA states disable stencil. */ }
static void blend_color(struct pipe_context *p,const struct pipe_blend_color *color)
{
 if(!color){c(p)->error=EINVAL;return;}
 for(unsigned i=0;i<4;i++)if(!isfinite(color->color[i])){c(p)->error=EINVAL;return;}
 c(p)->error=0; /* All admitted blend states disable blending. */
}
static void sample_mask(struct pipe_context *p,unsigned mask)
{ c(p)->sample_enabled=(mask&1)!=0;c(p)->error=0; /* Single sample only. */ }
/* Explicit CPU transfer fallback, not a GPU clear. It cannot establish GL
 * conformance: the native draw profile still clears/replaces its whole target.
 * Sync the resource writer before mapping; never synthesize a completion. */
static int cpu_clear(struct pipe_context *p,struct pipe_resource *target,
 const union pipe_color_union *color,unsigned x,unsigned y,unsigned width,unsigned height)
{
 struct context *ctx=c(p);struct screen *sc=s(p->screen);struct remesa_color_clear region;void *ptr=NULL;int error;
 if(sc->closing)return ESHUTDOWN;
 if(!target||target->screen!=p->screen||target->target!=PIPE_TEXTURE_2D||
    !color)return ENOTSUP;
 error=remesa_prepare_color_clear(target->format,target->width0,target->height0,r(target)->stride,
   r(target)->bytes,color,x,y,width,height,&region);if(error)return error;
 if(!width||!height)return 0;
 if(r(target)->maps)return EBUSY;
 error=resource_sync(p->screen,r(target),UINT64_MAX);if(error)return error;
 error=sc->device.ops->map(sc->device.priv,r(target)->bo,true,&ptr);
 if(error)return error;
 if(!ptr){sc->device.ops->unmap(sc->device.priv,r(target)->bo,true);return EPROTO;}
 for(unsigned row=0;row<region.rows;row++)
  for(size_t byte=0;byte<region.row_bytes;byte+=4)
   memcpy((unsigned char *)ptr+region.offset+(size_t)row*region.stride+byte,region.pixel,4);
 sc->device.ops->unmap(sc->device.priv,r(target)->bo,true);
 error=remesa_render_pass_cpu_write(&r(target)->pass);if(error){r(target)->write_error=error;return error;}
 r(target)->has_contents=true;++ctx->cpu_clears;return 0;
}
static void clear(struct pipe_context *p,unsigned buffers,const struct pipe_scissor_state *scissor,
 const union pipe_color_union *color,double depth,unsigned stencil)
{
 (void)depth;(void)stencil;
 if(!buffers){c(p)->error=0;return;}
 if(buffers!=PIPE_CLEAR_COLOR0){c(p)->error=ENOTSUP;return;}
 if(scissor&&(scissor->minx>scissor->maxx||scissor->miny>scissor->maxy||scissor->maxx>32||scissor->maxy>32)) {
  c(p)->error=EINVAL;return;
 }
 c(p)->error=cpu_clear(p,c(p)->target,color,scissor?scissor->minx:0,scissor?scissor->miny:0,
                      scissor?scissor->maxx-scissor->minx:32,scissor?scissor->maxy-scissor->miny:32);
}
static void clear_target(struct pipe_context *p,struct pipe_surface *surface,
 const union pipe_color_union *color,unsigned x,unsigned y,unsigned width,unsigned height,bool conditional)
{
 if(conditional||!surface||surface->context!=p||surface->format!=PIPE_FORMAT_B8G8R8A8_UNORM||
    surface->level||surface->first_layer||surface->last_layer||surface->nr_samples>1) {
  c(p)->error=ENOTSUP;return;
 }
 c(p)->error=cpu_clear(p,surface->texture,color,x,y,width,height);
}
static void draw(struct pipe_context *p,const struct pipe_draw_info *info,unsigned drawid,
 const struct pipe_draw_indirect_info *indirect,const struct pipe_draw_start_count_bias *draws,unsigned count)
{
 struct context *ctx=c(p);struct screen *sc=s(p->screen);
 struct pipe_draw_info validated_info;struct pipe_resource *owned_index=NULL;
 void *ip=NULL,*vp=NULL,*backend=NULL;struct remesa_draw_packet *packet=NULL;
 enum remesa_draw_status packed;int err=ENOTSUP;uint32_t draw_argb=ctx->argb;bool submitted=false,v_mapped=false,i_mapped=false;struct pipe_fence_handle *f=NULL;
 if(!info) goto done;
 /* Nonindexed draws do not initialize the index union. */
 if(info->index_size && info->take_index_buffer_ownership && !info->has_user_indices)
  owned_index=info->index.resource;
 if(sc->closing) { err=ESHUTDOWN;goto done; }
 if(sc->device.resource_only_backend) goto done;
 if(!draws || count!=1 || indirect || drawid) goto done;
 if(!draws->count || !info->instance_count || !ctx->sample_enabled) { err=0;goto done; }
 int sampling=remesa_sampler_draw_admit(ctx->samplers);if(sampling){err=sampling;goto done;}
 sampling=remesa_texture_draw_admit(ctx->textures);if(sampling){err=sampling;goto done;}
 if(ctx->invalid_viewport || ctx->invalid_scissor || ctx->invalid_elements || ctx->invalid_constants || ctx->constant_rejections[0] || ctx->constant_rejections[1]) goto done;
 for(unsigned kind=0;kind<FIXED_COUNT;kind++)
  if(ctx->invalid_fixed[kind]||(ctx->vertex_required&&!ctx->bound_fixed[kind]))goto done;
 struct fixed *raster=ctx->bound_fixed[FIXED_RASTER];
 if(raster&&((raster->scissor&&!ctx->scissor_set)||(raster->depth_clip&&!ctx->bound_vertex)))goto done;
 if(ctx->vertex_required) {
  if(!ctx->bound_vertex||!ctx->viewport_set||ctx->viewport.scale[0]!=16||ctx->viewport.scale[1]!=16||
     ctx->viewport.scale[2]!=.5f||ctx->viewport.translate[0]!=16||ctx->viewport.translate[1]!=16||ctx->viewport.translate[2]!=.5f)goto done;
 } else if(ctx->viewport_set) {
  for(unsigned axis=0;axis<3;axis++)if(ctx->viewport.scale[axis]!=1||ctx->viewport.translate[axis]!=0)goto done;
 }
 if((info->index_size && info->has_user_indices) || !ctx->target || !ctx->pipeline_bound ||
    !ctx->position_bound || !ctx->vb.buffer.resource ||
    (info->index_size && (!info->index.resource || info->index.resource->screen!=p->screen||
     info->index.resource->target!=PIPE_BUFFER||!(info->index.resource->bind&PIPE_BIND_INDEX_BUFFER)))) goto done;
#ifdef REMESA_WITH_NIR
 int color_error=fragment_color(p,&draw_argb);if(color_error){err=color_error;goto done;}
#endif
 /* The live native profile still admits magenta only. Other observed colors
  * may be translated offline, but do not expand kernel execution admission. */
 if(ctx->bound_fragment && sc->device.qualified_hardware_backend && draw_argb!=0xffff00ff) goto done;
 if(r(ctx->target)->maps || r(ctx->vb.buffer.resource)->maps ||
    (info->index_size && r(info->index.resource)->maps)) { err=EBUSY;goto done; }
 /* A retained handle is not proof of preserved pixels. The current native
  * profile initializes COLOR to zero and replaces the whole render resource.
  * Fail before dispatch rather than erase a clear/prior ordinary draw. */
 if(r(ctx->target)->has_contents&&!ctx->diagnostic_discard&&
    sc->device.framebuffer_mode!=REMESA_FRAMEBUFFER_PRESERVE)goto done;
 err=resource_sync(p->screen,r(ctx->target),UINT64_MAX);if(err) goto done;
 err=resource_sync(p->screen,r(ctx->vb.buffer.resource),UINT64_MAX);if(err)goto done;
 if(info->index_size) {err=resource_sync(p->screen,r(info->index.resource),UINT64_MAX);if(err)goto done;}
 if(info->index_size) {
  err=sc->device.ops->map(sc->device.priv,r(info->index.resource)->bo,false,&ip);if(err) goto done;
  i_mapped=true;if(!ip){err=EPROTO;goto done;}
 }
 validated_info=*info;validated_info.take_index_buffer_ownership=false;
 err=sc->device.ops->map(sc->device.priv,r(ctx->vb.buffer.resource)->bo,false,&vp);if(err) goto done;
 v_mapped=true;if(!vp){err=EPROTO;goto done;}
 packet=malloc(sizeof(*packet));f=calloc(1,sizeof(*f));if(!packet || !f) { err=ENOMEM;goto done; }
 /* CPU primitive clipping is connected to the real callback only for the
  * recording backend and explicit depth-clip state. This does not qualify
  * GPU edge rules or expand the live executor's historical profile. */
 if(sc->device.offline_test_backend && !sc->device.qualified_hardware_backend &&
    ctx->bound_vertex && raster && raster->full_clip) {
  struct remesa_prepared_triangles *prepared=malloc(sizeof(*prepared));
  if(!prepared){err=ENOMEM;goto done;}
  packed=remesa_prepare_constant_draw(&validated_info,draws,&ctx->vb,&ctx->position,
      vp,r(ctx->vb.buffer.resource)->bytes,ip,info->index_size?r(info->index.resource)->bytes:0,
      draw_argb,&ctx->viewport,ctx->target->width0,ctx->target->height0,prepared);
  if(packed==REMESA_DRAW_VALIDATED)packed=remesa_pack_prepared_triangles(prepared,packet);
  free(prepared);
  if(packed==REMESA_DRAW_EMPTY){err=0;goto done;}
 } else {
 packed=remesa_pack_triangle_draw_vertex(&validated_info,draws,&ctx->vb,&ctx->position,
     vp,r(ctx->vb.buffer.resource)->bytes,ip,info->index_size ? r(info->index.resource)->bytes : 0,draw_argb,ctx->bound_vertex ? &ctx->viewport : NULL,packet);
 }
 if(packed!=REMESA_DRAW_VALIDATED) { err=packed==REMESA_DRAW_NOMEM ? ENOMEM :
     packed==REMESA_DRAW_UNSUPPORTED ? ENOTSUP : EINVAL;goto done; }
 enum remesa_pass_load load=r(ctx->target)->has_contents&&!ctx->diagnostic_discard ? REMESA_PASS_PRESERVE : REMESA_PASS_DISCARD;
 err=remesa_render_pass_begin(&r(ctx->target)->pass,load,REMESA_PASS_STORE,
    sc->device.framebuffer_mode==REMESA_FRAMEBUFFER_PRESERVE,&r(ctx->target)->ticket);if(err)goto done;
 err=remesa_render_pass_submitted(&r(ctx->target)->pass,&r(ctx->target)->ticket);if(err)goto done;
 submitted=true;
 err=sc->device.ops->submit(sc->device.priv,r(ctx->target)->bo,packet,&backend);if(!err && !backend) err=EPROTO;
 if(!err) {
  r(ctx->target)->has_contents=true;
  f->refs=1;f->screen=sc;f->backend=backend;screen_hold(sc);
  fence_ref(p->screen,&r(ctx->target)->writer,f);
  fence_ref(p->screen,&ctx->last,f);fence_ref(p->screen,&f,NULL);
 }
 done:
 if(v_mapped||vp) sc->device.ops->unmap(sc->device.priv,r(ctx->vb.buffer.resource)->bo,false);
 if(i_mapped||ip) sc->device.ops->unmap(sc->device.priv,r(info->index.resource)->bo,false);
 /* A backend-returned fence is owned even on failure. Its destructor must
  * retain uncertain device backing; frontend cleanup cannot imply retirement. */
 if(err && submitted) {
  /* A returned error fence may represent uncertain issuance. Never let a CPU
   * overwrite or another draw turn it into an apparently valid resource. */
  r(ctx->target)->write_error=err;
  remesa_render_pass_fail(&r(ctx->target)->pass,&r(ctx->target)->ticket,err);
  if(backend) sc->device.ops->destroy_fence(sc->device.priv,backend);
 }
 remesa_resource_reference(&owned_index,NULL);
 free(packet);free(f);ctx->error=err;
}
static void flush(struct pipe_context *p,struct pipe_fence_handle **f,unsigned flags)
{ (void)flags;if(f) fence_ref(p->screen,f,c(p)->error ? NULL : c(p)->last); }
static void destroy_context(struct pipe_context *p)
{ struct context *ctx=c(p);struct screen *sc=s(p->screen);remesa_resource_reference(&ctx->vb.buffer.resource,NULL);remesa_resource_reference(&ctx->target,NULL);remesa_resource_reference(&ctx->fragment_constants,NULL);free(ctx->constant_snapshot);fence_ref(p->screen,&ctx->last,NULL);while(ctx->fragments){struct fragment *f=ctx->fragments;ctx->fragments=f->next;free(f);}while(ctx->vertices){struct vertex *v=ctx->vertices;ctx->vertices=v->next;free(v);}while(ctx->fixed_states){struct fixed *state=ctx->fixed_states;ctx->fixed_states=state->next;free(state);}while(ctx->elements){struct elements *e=ctx->elements;ctx->elements=e->next;free(e);}remesa_sampler_bank_destroy(ctx->samplers);remesa_texture_binding_close(ctx->textures);free(ctx);screen_drop(sc); }
static struct pipe_context *create_context(struct pipe_screen *p,void *priv,unsigned flags)
{
 /* PREFER_THREADED is a hint; NO_LOD_BIAS adds no unsupported sampler state.
  * Other semantic/context-security flags remain unavailable. */
 struct context *ctx;if((flags & ~(PIPE_CONTEXT_PREFER_THREADED|PIPE_CONTEXT_NO_LOD_BIAS)) || s(p)->closing) return NULL;ctx=calloc(1,sizeof(*ctx));if(!ctx) return NULL;
 ctx->samplers=remesa_sampler_bank_create(NULL,NULL,NULL);if(!ctx->samplers){free(ctx);return NULL;}
 ctx->textures=remesa_texture_binding_create(p,texture_storage,p,NULL,NULL,NULL);
 if(!ctx->textures){remesa_sampler_bank_destroy(ctx->samplers);free(ctx);return NULL;}
 ctx->sample_enabled=true;ctx->base.screen=p;ctx->base.priv=priv;ctx->base.destroy=destroy_context;
 ctx->base.buffer_map=map;ctx->base.texture_map=map;ctx->base.buffer_unmap=unmap;ctx->base.texture_unmap=unmap;
 ctx->base.buffer_subdata=buffer_subdata;ctx->base.texture_subdata=texture_subdata;
 ctx->base.resource_copy_region=copy_region;
 ctx->base.clear_texture=clear_texture;
 ctx->base.set_vertex_buffers=set_vb;ctx->base.create_vertex_elements_state=create_elements;
 ctx->base.bind_vertex_elements_state=bind_elements;ctx->base.delete_vertex_elements_state=delete_elements;
 ctx->base.create_surface=create_surface;ctx->base.surface_destroy=surface_destroy;
 ctx->base.create_fs_state=unsupported_shader;ctx->base.create_vs_state=unsupported_shader;ctx->base.clear=clear;ctx->base.clear_render_target=clear_target;
#ifdef REMESA_WITH_NIR
 ctx->base.create_vs_state=create_vertex;ctx->base.bind_vs_state=bind_vertex;ctx->base.delete_vs_state=delete_vertex;
 ctx->base.create_fs_state=create_fragment;ctx->base.bind_fs_state=bind_fragment;ctx->base.delete_fs_state=delete_fragment;
#endif
 ctx->base.create_blend_state=create_blend;ctx->base.bind_blend_state=bind_blend;ctx->base.delete_blend_state=delete_blend;
 ctx->base.create_depth_stencil_alpha_state=create_dsa;ctx->base.bind_depth_stencil_alpha_state=bind_dsa;ctx->base.delete_depth_stencil_alpha_state=delete_dsa;
 ctx->base.create_rasterizer_state=create_raster;ctx->base.bind_rasterizer_state=bind_raster;ctx->base.delete_rasterizer_state=delete_raster;
 ctx->base.set_stencil_ref=stencil_ref;ctx->base.set_blend_color=blend_color;
 ctx->base.set_constant_buffer=constant_buffer;ctx->base.set_sample_mask=sample_mask;
 ctx->base.create_sampler_state=create_sampler;ctx->base.bind_sampler_states=bind_samplers;ctx->base.delete_sampler_state=delete_sampler;
 ctx->base.create_sampler_view=create_sampler_view;ctx->base.sampler_view_destroy=destroy_sampler_view;ctx->base.set_sampler_views=set_sampler_views;
 ctx->base.set_viewport_states=viewport_state;ctx->base.set_scissor_states=scissor_state;
 ctx->base.set_framebuffer_state=framebuffer;ctx->base.draw_vbo=draw;ctx->base.flush=flush;screen_hold(s(p));return &ctx->base;
}
static void destroy_screen(struct pipe_screen *p)
{ if(!s(p)->closing) { s(p)->closing=true;screen_drop(s(p)); } }
struct pipe_screen *remesa_screen_create_owned(struct remesa_device *device,int (*release)(struct remesa_device *))
{
 struct screen *sc;
 if(!device || (device->framebuffer_mode!=REMESA_FRAMEBUFFER_REPLACE&&device->framebuffer_mode!=REMESA_FRAMEBUFFER_PRESERVE) ||
    (device->framebuffer_mode==REMESA_FRAMEBUFFER_PRESERVE&&(!device->offline_test_backend||device->qualified_hardware_backend)) || (!device->offline_test_backend && !device->qualified_hardware_backend && !device->resource_only_backend) || !device->ops ||
    !device->ops->allocate || !device->ops->destroy_bo || !device->ops->map || !device->ops->unmap ||
    !device->ops->submit || !device->ops->wait || !device->ops->destroy_fence) return NULL;
 sc=calloc(1,sizeof(*sc));if(!sc) return NULL;sc->device=*device;sc->refs=1;sc->release_device=release;
 sc->base.destroy=destroy_screen;sc->base.get_name=name;sc->base.get_vendor=vendor;sc->base.context_create=create_context;
 set_capabilities(sc);
 #ifdef REMESA_WITH_NIR
 sc->base.get_compiler_options=compiler_options;
#endif
 sc->base.is_format_supported=format;sc->base.resource_create=create_resource;sc->base.resource_destroy=destroy_resource;
 sc->base.fence_reference=fence_ref;sc->base.fence_finish=finish;
 if(release) memset(device,0,sizeof(*device));
 return &sc->base;
}
int remesa_screen_collect(struct pipe_screen *p)
{
 struct screen *sc;
 if(!p || p->destroy!=destroy_screen) return EINVAL;
 sc=s(p);if(!sc->closing || !sc->release_pending || sc->refs!=1) return EBUSY;
 if(sc->release_error!=EBUSY && sc->release_error!=EAGAIN) return sc->release_error;
 /* Only backend collection/release may retry. No draw can be redispatched. */
 int error=sc->release_device(&sc->device);
 if(error) { sc->release_error=error;return error; }
 free(sc);return 0;
}
struct pipe_screen *remesa_screen_create(struct remesa_device *device)
{ return remesa_screen_create_owned(device,NULL); }
int remesa_resource_wait_writer(struct pipe_context *ctx,struct pipe_resource *res,uint64_t timeout)
{
 if(!ctx || ctx->draw_vbo!=draw || !res || res->screen!=ctx->screen) return EINVAL;
 if(r(res)->maps) return EBUSY;
 return resource_sync(ctx->screen,r(res),timeout);
}
int remesa_resource_attach_writer(struct pipe_context *ctx,struct pipe_resource *res,void *backend,struct pipe_fence_handle **out)
{
 struct pipe_fence_handle *f;
 if(!ctx || ctx->draw_vbo!=draw || s(ctx->screen)->closing || !s(ctx->screen)->device.resource_only_backend || !res || res->screen!=ctx->screen || !backend || !out || *out ||
    r(res)->maps || r(res)->writer || r(res)->write_error)return EINVAL;
 f=calloc(1,sizeof(*f));if(!f)return ENOMEM;
 int error=remesa_render_pass_import_writer(&r(res)->pass,&r(res)->ticket);
 if(error){free(f);return error;}
 f->refs=1;f->screen=s(ctx->screen);f->backend=backend;screen_hold(f->screen);
 r(res)->has_contents=true;fence_ref(ctx->screen,&r(res)->writer,f);*out=f;return 0;
}
int remesa_bind_diagnostic_pipeline(struct pipe_context *p,uint32_t argb)
{ if(!p || p->draw_vbo!=draw) return EINVAL;c(p)->argb=argb;c(p)->pipeline_bound=true;c(p)->diagnostic_discard=true;return 0; }
int remesa_context_set_framebuffer_load(struct pipe_context *p,bool preserve)
{
 if(!p||p->draw_vbo!=draw)return EINVAL;
 if(s(p->screen)->closing)return ESHUTDOWN;
 /* The qualified hardware profile is explicitly REPLACE-only.  Reject a
  * preserve request at admission instead of accepting state that can only
  * fail later, after a caller has assumed load semantics.  Offline recording
  * fixtures may still exercise the state machine with an explicit PRESERVE
  * device mode. */
 if(preserve && s(p->screen)->device.qualified_hardware_backend &&
    s(p->screen)->device.framebuffer_mode!=REMESA_FRAMEBUFFER_PRESERVE)
  return ENOTSUP;
 c(p)->diagnostic_discard=!preserve;return 0;
}
uint64_t remesa_context_cpu_clear_count(const struct pipe_context *p) { return p && p->draw_vbo==draw ? ((const struct context *)p)->cpu_clears : 0; }
uint64_t remesa_context_cpu_copy_count(const struct pipe_context *p) { return p && p->draw_vbo==draw ? ((const struct context *)p)->cpu_copies : 0; }
int remesa_context_error(const struct pipe_context *p) { return p && p->draw_vbo==draw ? ((const struct context *)p)->error : EINVAL; }
#else
int remesa_context_set_framebuffer_load(struct pipe_context *p,bool preserve) { (void)p;(void)preserve;return ENOTSUP; }
uint64_t remesa_context_cpu_clear_count(const struct pipe_context *p) { (void)p;return 0; }
uint64_t remesa_context_cpu_copy_count(const struct pipe_context *p) { (void)p;return 0; }
int remesa_screen_collect(struct pipe_screen *screen) { (void)screen;return ENOTSUP; }
struct pipe_screen *remesa_screen_create_owned(struct remesa_device *device,int (*release)(struct remesa_device *)) { (void)device;(void)release;return 0; }
struct pipe_screen *remesa_screen_create(struct remesa_device *device) { (void)device;return 0; }
#endif
const char *remesa_screen_unavailable_reason(void) { return "Qualified clean hardware backend required; recording tests do not render"; }
