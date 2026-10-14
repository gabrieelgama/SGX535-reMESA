/* SPDX-License-Identifier: MIT
 * Actual Gallium/NIR -> production Linux token/staging/retained submit.
 * ALL device calls are interposed. Completion/capture/pixels below are explicit
 * fixtures, never hardware evidence. Opaque retained carrier input comes only
 * from stdin and is compared, not embedded or executed by this test.
 */
#include "remesa_linux.h"
#include "remesa_linux_memory.h"
#include "remesa_screen.h"
#include "remesa_shader_program.h"
#include "nir_fixture.h"
#include "vertex_nir_fixture.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks,created,retained_calls,records,auth_calls,use_writes;
static unsigned compared_images;
static struct remesa_device *active_device;
static const char *scenario;
static struct object {size_t size;uint64_t va;unsigned char *data;bool live,pinned,leased,write;} objects[32];
static unsigned char *baseline[6];
static const size_t sizes[10]={0x20000,0x8000,0x1000,0x1000,0x1000,0x1000,0x2000,0x2000000,0x2000000,0x1000};
static const uint64_t addresses[10]={0x20000000,0x20080000,0x40000000,0x30000000,0x46000000,
 0x40001000,0x40002000,0x42000000,0x31000000,0x46001000};
#define CHECK(c) do {assert(c);++checks;} while(0)
int __wrap_ioctl(int,unsigned long,...);
int __wrap_open(const char *,int,...);
int __wrap_open64(const char *,int,...);
int __wrap_close(int);
int __wrap_open(const char *path,int flags,...)
{(void)flags;CHECK(!strcmp(path,"INTERPOSED-NO-DEVICE"));return 100;}
int __wrap_open64(const char *p,int f,...){return __wrap_open(p,f);}
int __wrap_close(int fd){CHECK(fd==100);return 0;}
int __wrap_ioctl(int fd,unsigned long command,...)
{
 va_list ap;va_start(ap,command);void *arg=va_arg(ap,void *);va_end(ap);
 if(command==DRM_IOCTL_VERSION){CHECK(fd==100);struct drm_version *v=arg;memcpy(v->name,"gma500",6);v->name_len=6;return 0;}
 CHECK(fd==99); /* No native draw ioctl or second SGX endpoint is allowed. */
 if(command==DRM_IOCTL_REMESA_RETAINED) {
  struct remesa_retained_request *r=arg;CHECK(++retained_calls==1 && r->context==1234);
  CHECK(use_writes>0 && records==1);
  for(unsigned i=0;i<10;i++)CHECK(r->bindings[i].id==32+i&&r->bindings[i].va==addresses[i]&&r->bindings[i].bytes==sizes[i]);
  CHECK(!memcmp(r->commands,baseline[4]+208,56)&&!memcmp(r->commands+56,baseline[4],208));
  for(unsigned i=0;i<6;i++){CHECK(!memcmp(objects[i].data,baseline[i],sizes[i]));compared_images++;}
  if(!strcmp(scenario,"ioctl-fail")){errno=EIO;return -1;}
  r->operation=1;r->serial=1;r->generation=1;r->state=6;r->admitted=1;
  r->observer_request_bytes=1;r->observer_request[0]=0xa1;
  r->observer_response_bytes=4140;
  /* Explicit unrelated fixture data, NOT a triangle simulation/reference. */
  memset(objects[5].data,0x7b,4096);memset(r->observer_response+44,0x7b,4096);
  if(!strcmp(scenario,"color-mismatch"))r->observer_response[44]^=1;
  return 0;
 }
 CHECK(command==DRM_IOCTL_REMESA_MEMORY);
 struct remesa_memory_request *r=arg;
 CHECK(r->version==1 && !r->reserved);
 if(r->action==REMESA_MEM_OPEN){CHECK(!r->context);r->context=1234;return 0;}
 CHECK(r->context==1234);
 if(r->action==REMESA_MEM_CREATE) {
  CHECK(created<32 && !r->id);unsigned n=created++;
  objects[n].size=(size_t)r->bytes;objects[n].data=calloc(1,(size_t)r->bytes);
  CHECK(objects[n].data);objects[n].live=true;r->id=32+n;return 0;
 }
 if(r->action==REMESA_MEM_CLOSE){for(unsigned i=0;i<created;i++)CHECK(!objects[i].live);return 0;}
 CHECK(r->id>=32 && r->id<32+created);
 unsigned slot=r->id-32;struct object *b=&objects[slot];CHECK(b->live);
 switch(r->action) {
 case REMESA_MEM_PIN:CHECK(!b->pinned);b->pinned=true;return 0;
 case REMESA_MEM_UNPIN:CHECK(b->pinned&&!b->leased);b->pinned=false;return 0;
 case REMESA_MEM_MAP:CHECK(b->pinned&&!b->va);b->va=r->va;return 0;
 case REMESA_MEM_UNMAP:CHECK(b->va&&!b->leased);b->va=0;return 0;
 case REMESA_MEM_DESTROY:CHECK(!b->pinned&&!b->va&&!b->leased);b->live=false;free(b->data);b->data=NULL;return 0;
 case REMESA_MEM_LEASE:CHECK(b->pinned&&!b->leased);b->leased=true;b->write=r->flags!=0;return 0;
 case REMESA_MEM_UNLEASE:CHECK(b->leased);b->leased=false;b->write=false;return 0;
 case REMESA_MEM_READ:case REMESA_MEM_WRITE:
  CHECK(b->leased && r->offset<=b->size && r->length<=b->size-r->offset);
  if(r->action==REMESA_MEM_READ)memcpy(r->data,b->data+(size_t)r->offset,r->length);
  else {
   CHECK(b->write);
   if(slot==1&&created>10){use_writes++;if(!strcmp(scenario,"write-fail")){errno=EIO;return -1;}}
   memcpy(b->data+(size_t)r->offset,r->data,r->length);
   if(slot==10&&!strcmp(scenario,"source-corrupt"))b->data[0]^=1;
  }
  return 0;
 case REMESA_MEM_INFO: {
  struct remesa_memory_info info={0};
  for(unsigned i=0;i<created;i++)if(objects[i].live) {
   info.objects++;info.allocated_bytes+=objects[i].size;
   info.pinned_objects+=objects[i].pinned;
   if(objects[i].va)info.mapped_pages+=(uint32_t)(objects[i].size/4096);
   info.leases+=objects[i].leased;
  }
  info.object_bytes=b->size;info.gpu_va=b->va;info.object_pinned=b->pinned;
  info.object_lease=b->leased;
  info.object_mapped_pages=info.verified_pages=b->va?(uint32_t)(b->size/4096):0;
  if(created>10&&slot==1&&!strcmp(scenario,"info-stale"))info.gpu_va+=4096;
  memcpy(r->data,&info,sizeof(info));return 0;
 }
 default:CHECK(!"unexpected memory operation");return -1;
 }
}
static int record(void *p,const struct remesa_retained_request *before,
 const struct remesa_retained_request *after,int result,int error)
{(void)p;(void)result;(void)error;CHECK(before->context==1234);records++;if(after)CHECK(records==2);return 0;}
static int authenticate(void *p,const struct remesa_retained_request *r,struct remesa_linux_fragment_evidence *out)
{
 (void)p;auth_calls++;if(!strcmp(scenario,"auth-fail"))return EACCES;
 CHECK(remesa_linux_close_checked(active_device)==EBUSY);
 memset(out,0,sizeof(*out));
 struct remesa_evidence_expected *e=&out->expected;
 struct remesa_evidence_response *s=&out->response;
 struct remesa_evidence_observer *o=&out->observer;
 struct remesa_evidence_ownership *g=&out->ownership;
 e->id=(struct remesa_evidence_id){r->context,r->operation,r->serial,r->slot,r->generation};
 memset(e->boot,1,16);memset(e->source,2,32);memset(e->request_sha256,3,32);
 e->request_bytes=500;e->resources=10;s->id=g->id=e->id;
 memcpy(s->request_sha256,e->request_sha256,32);memcpy(o->request_sha256,e->request_sha256,32);
 memset(s->response_sha256,4,32);memcpy(o->response_sha256,s->response_sha256,32);
 memcpy(o->boot,e->boot,16);memcpy(o->source,e->source,32);
 s->request_bytes=o->request_bytes=500;s->response_bytes=o->response_bytes=4140;
 s->records=s->attempted=s->ta_complete=s->end_render=s->memory_free=s->retired=s->visible=s->acknowledged=1;
 o->native_records=o->capsule_records=o->source_records=1;
 o->request_present=o->response_present=o->closed=o->syscall_exit_seen=o->source_valid=1;
 o->ta_complete=o->end_render=o->memory_free=o->retired=1;
 g->records=g->pfn_valid=g->mappings_valid=g->held_through_completion=1;
 g->visibility_ordered=g->detached=g->released=1;g->resources=10;
 return 0; /* TEST-ONLY synthetic normalized evidence. */
}
static struct pipe_resource *buffer(struct pipe_screen *s,unsigned bytes,unsigned bind)
{struct pipe_resource t={.target=PIPE_BUFFER,.format=PIPE_FORMAT_R8_UNORM,.width0=bytes,.height0=1,.depth0=1,.array_size=1,.bind=bind};return s->resource_create(s,&t);}
static void upload(struct pipe_context *c,struct pipe_resource *r,const void *p,unsigned bytes)
{struct pipe_box box={.width=(int)bytes,.height=1,.depth=1};struct pipe_transfer *t=NULL;void *m=c->buffer_map(c,r,0,PIPE_MAP_WRITE,&box,&t);CHECK(m);memcpy(m,p,bytes);c->buffer_unmap(c,t);}
static void drop(struct pipe_resource *r){if(!__atomic_sub_fetch(&r->reference.count,1,__ATOMIC_ACQ_REL))r->screen->resource_destroy(r->screen,r);}
int main(int argc,char **argv)
{
 CHECK(argc==2);scenario=argv[1];
 struct remesa_device d={0};struct remesa_linux_memory *m=NULL;
 active_device=&d;
 CHECK(!remesa_linux_open(&d,"INTERPOSED-NO-DEVICE"));CHECK(!remesa_linux_memory_begin(99,&m));
 const struct remesa_persistent_ops *ops=remesa_linux_memory_ops();
 struct remesa_retained_request job={.version=1,.context=1234};
 for(unsigned i=0;i<10;i++) {
  uint32_t id=0;CHECK(!ops->create(m,sizes[i],&id));CHECK(!ops->pin(m,id));
  CHECK(!ops->gpu_map(m,id,addresses[i],sizes[i]));
  job.bindings[i]=(struct remesa_retained_binding){id,0,addresses[i],sizes[i]};
  if(i<6) {
   baseline[i]=malloc(sizes[i]);CHECK(baseline[i]);CHECK(fread(baseline[i],1,sizes[i],stdin)==sizes[i]);
   void *ptr=NULL;CHECK(!ops->cpu_map(m,id,true,&ptr));memcpy(ptr,baseline[i],sizes[i]);
   if(i==1)memset(ptr,0xa5,8); /* Cannot pass from an untouched fixed shader. */
   CHECK(!ops->cpu_unmap(m,id,true));
  }
 }
 CHECK(fgetc(stdin)==EOF);
 /* The exact retained CONTROL layout stores raster then TA; the internal
  * command snapshot is TA then raster. No instruction bytes are defined. */
 memcpy(job.commands,baseline[4]+208,56);memcpy(job.commands+56,baseline[4],208);
 struct remesa_persistent_config cfg={.ops=ops,.priv=m,.context=1234,
  .va_start=0x23000000,.va_end=0x23010000,.byte_budget=16384,.resource_only_backend=true};
 CHECK(!remesa_linux_enable_shader_staging(&d,&cfg));
 struct remesa_linux_fragment_carrier carrier={m,&job,record,authenticate,NULL};
 struct remesa_linux_fragment_carrier invalid=carrier;invalid.authenticate=NULL;
 CHECK(remesa_linux_enable_retained_fragment(&d,&invalid)==EINVAL&&!d.retained_fragment_carrier);
 if(strcmp(scenario,"disabled")&&strcmp(scenario,"late"))CHECK(!remesa_linux_enable_retained_fragment(&d,&carrier));
 struct pipe_screen *s=remesa_screen_create(&d);CHECK(s);struct pipe_context *c=s->context_create(s,NULL,0);CHECK(c);
 if(!strcmp(scenario,"late"))CHECK(!remesa_linux_enable_retained_fragment(&d,&carrier));
 if(strcmp(scenario,"disabled"))memset(&job,0xa5,sizeof(job)); /* config owns its copy */
 if(strcmp(scenario,"disabled")&&strcmp(scenario,"late")){CHECK(strstr(s->get_name(s),"retained VS/PDS"));CHECK(!s->caps.accelerated);}
 struct pipe_resource *vb=buffer(s,48,PIPE_BIND_VERTEX_BUFFER),*ib=buffer(s,6,PIPE_BIND_INDEX_BUFFER);CHECK(vb&&ib);
 float positions[3][4]={{-.5f,-.5f,0,1},{.5f,-.5f,0,1},{-.5f,.5f,0,1}};
 if(!strcmp(scenario,"shape"))positions[0][0]=-.25f;
 const uint16_t indices[3]={0,1,2};upload(c,vb,positions,48);upload(c,ib,indices,6);
 struct pipe_resource rt={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,.width0=32,.height0=32,.depth0=1,.array_size=1,.bind=PIPE_BIND_RENDER_TARGET};
 struct pipe_resource *target=s->resource_create(s,&rt);CHECK(target);
 struct pipe_framebuffer_state fb={.width=32,.height=32,.nr_cbufs=1};fb.cbufs[0].texture=target;fb.cbufs[0].format=target->format;c->set_framebuffer_state(c,&fb);
 struct pipe_vertex_buffer binding={.buffer.resource=vb};++vb->reference.count;c->set_vertex_buffers(c,1,&binding);
 struct pipe_vertex_element element={.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT,.src_stride=16};
 void *ve=c->create_vertex_elements_state(c,1,&element);CHECK(ve);c->bind_vertex_elements_state(c,ve);
 struct pipe_shader_state state={.type=PIPE_SHADER_IR_NIR,.ir.nir=vertex_fixture()};
 void *vs=c->create_vs_state(c,&state);CHECK(vs);c->bind_vs_state(c,vs);
 state.ir.nir=fixture(!strcmp(scenario,"green")?0xff00ff00:0xffff00ff);
 void *fs=c->create_fs_state(c,&state);CHECK(fs);c->bind_fs_state(c,fs);
 struct pipe_blend_state blend={0};blend.rt[0].colormask=PIPE_MASK_RGBA;
 struct pipe_depth_stencil_alpha_state depth={0};
 struct pipe_rasterizer_state raster={.fill_front=PIPE_POLYGON_MODE_FILL,.fill_back=PIPE_POLYGON_MODE_FILL};
 void *bs=c->create_blend_state(c,&blend),*ds=c->create_depth_stencil_alpha_state(c,&depth),*rs=c->create_rasterizer_state(c,&raster);CHECK(bs&&ds&&rs);
 c->bind_blend_state(c,bs);c->bind_depth_stencil_alpha_state(c,ds);c->bind_rasterizer_state(c,rs);
 struct pipe_viewport_state viewport={.scale={16,16,.5f},.translate={16,16,.5f}};c->set_viewport_states(c,0,1,&viewport);
 struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.index_size=2,.instance_count=1};info.index.resource=ib;
 struct pipe_draw_start_count_bias draw={.count=3};CHECK(!remesa_context_error(c));
 c->draw_vbo(c,&info,0,NULL,&draw,1);
 int error=remesa_context_error(c);
 struct remesa_linux_fragment_receipt receipt={0};
 int receipt_error=remesa_linux_last_retained_fragment(&d,&receipt);
 bool good=!strcmp(scenario,"good");
 if(good) {
  CHECK(!error && retained_calls==1 && records==2 && auth_calls==1);
  CHECK(!receipt_error&&receipt.staged&&receipt.outside_fragment_unchanged&&receipt.authenticated&&!receipt.quarantined);
  CHECK(!memcmp(receipt.source,baseline[1],8));
  for(unsigned i=0;i<8;i++)CHECK(receipt.preimage[i]==0xa5);
  struct pipe_fence_handle *f=NULL;c->flush(c,&f,0);CHECK(f&&s->fence_finish(s,c,f,0));
  struct pipe_box box={.width=32,.height=32,.depth=1};struct pipe_transfer *t=NULL;
  const unsigned char *ptr=c->texture_map(c,target,0,PIPE_MAP_READ,&box,&t);CHECK(ptr);
  for(unsigned i=0;i<4096;i++)CHECK(ptr[i]==0x7b);
  c->texture_unmap(c,t);s->fence_reference(s,&f,NULL);
 } else {
  CHECK(error);
  if(!strcmp(scenario,"disabled")||!strcmp(scenario,"late")||!strcmp(scenario,"green")||!strcmp(scenario,"shape")||!strcmp(scenario,"source-corrupt"))CHECK(!retained_calls);
  if(!strcmp(scenario,"auth-fail")||!strcmp(scenario,"color-mismatch"))CHECK(retained_calls==1&&auth_calls==1&&receipt.quarantined&&!receipt.authenticated);
 }
 c->bind_fs_state(c,NULL);c->delete_fs_state(c,fs);c->bind_vs_state(c,NULL);c->delete_vs_state(c,vs);
 c->bind_vertex_elements_state(c,NULL);c->delete_vertex_elements_state(c,ve);
 c->bind_blend_state(c,NULL);c->delete_blend_state(c,bs);c->bind_depth_stencil_alpha_state(c,NULL);c->delete_depth_stencil_alpha_state(c,ds);c->bind_rasterizer_state(c,NULL);c->delete_rasterizer_state(c,rs);
 c->destroy(c);drop(vb);drop(ib);drop(target);s->destroy(s);
 if(receipt.quarantined)CHECK(remesa_linux_close_checked(&d)==EBUSY);
 else {
  CHECK(!remesa_linux_close_checked(&d));
  for(unsigned i=10;i;i--){unsigned j=i-1;CHECK(!ops->gpu_unmap(m,32+j,addresses[j],sizes[j]));CHECK(!ops->unpin(m,32+j));CHECK(!ops->destroy(m,32+j));}
  CHECK(!remesa_linux_memory_detach(m));
 }
 printf("PASS %s: %u checks; actual NIR/draw_vbo/production token -> retained ioctl calls=%u; carrier images compared=%u; fixture data only, GPU0\n",scenario,checks,retained_calls,compared_images);
 return 0;
}
