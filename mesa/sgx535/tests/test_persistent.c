/* SPDX-License-Identifier: MIT */
/* GEM/MMU operations and completion receipts below are offline fixtures.
 * No registers, ioctl, CPU rasterizer or GPU-output claims. */
#include "remesa_persistent.h"
#include "remesa_screen.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct gem {void *bytes;size_t length;uint64_t va;bool live,pinned,mapped,cpu;};
struct event {struct remesa_gpu_submit submitted;bool retired,published,failed,released;};
struct fixture {
 struct gem gems[128];struct event events[256];
 unsigned creates,destroys,pins,unpins,maps,unmaps,syncs,dispatches,releases;
 unsigned fail,invalid,release_retries;enum remesa_dispatch dispatch_result;
};
static struct gem *gem(struct fixture *f,uint32_t h) {assert(h && h<=128 && f->gems[h-1].live);return &f->gems[h-1];}
static int create(void *data,size_t n,uint32_t *h)
{struct fixture *f=data;assert(f->creates<128);struct gem *b=&f->gems[f->creates++];b->bytes=calloc(1,n);assert(b->bytes);b->length=n;b->live=true;*h=f->creates;return f->fail==1?ENOMEM:0;}
static int pin(void *data,uint32_t h)
{struct fixture *f=data;struct gem *b=gem(f,h);if(f->fail==2)return ENOMEM;assert(!b->pinned);b->pinned=true;++f->pins;return 0;}
static int gpu_map(void *data,uint32_t h,uint64_t va,size_t n)
{struct fixture *f=data;struct gem *b=gem(f,h);if(f->fail==3)return ENOMEM;assert(b->pinned&&!b->mapped&&n%4096==0);
 for(unsigned i=0;i<f->creates;i++) if(f->gems[i].mapped)assert(!(va<f->gems[i].va+f->gems[i].length && f->gems[i].va<va+n));
 b->mapped=true;b->va=va;++f->maps;return 0;}
static int gpu_unmap(void *data,uint32_t h,uint64_t va,size_t n)
{struct fixture *f=data;struct gem *b=gem(f,h);(void)n;if(f->fail==4)return EOWNERDEAD;assert(b->mapped&&b->va==va);b->mapped=false;++f->unmaps;return 0;}
static int unpin(void *data,uint32_t h)
{struct fixture *f=data;struct gem *b=gem(f,h);assert(b->pinned&&!b->mapped);b->pinned=false;++f->unpins;return 0;}
static int destroy(void *data,uint32_t h)
{struct fixture *f=data;struct gem *b=gem(f,h);assert(!b->mapped&&!b->pinned&&!b->cpu);free(b->bytes);memset(b,0,sizeof(*b));++f->destroys;return 0;}
static int cpu_map(void *data,uint32_t h,bool write,void **out)
{struct fixture *f=data;struct gem *b=gem(f,h);(void)write;if(f->fail==5)return ENOMEM;assert(!b->cpu);b->cpu=true;*out=b->bytes;return 0;}
static int cpu_unmap(void *data,uint32_t h,bool write)
{struct fixture *f=data;struct gem *b=gem(f,h);(void)write;assert(b->cpu);b->cpu=false;return 0;}
static int sync_device(void *data,uint32_t h)
{struct fixture *f=data;gem(f,h);++f->syncs;return 0;}
static enum remesa_dispatch dispatch(void *data,const struct remesa_gpu_submit *s)
{struct fixture *f=data;struct gem *b=gem(f,s->gem_handle);assert(b->mapped&&b->va==s->gpu_va);
 assert(s->operation&&s->context&&s->bytes==4096&&remesa_draw_packet_validate(s->packet)==REMESA_PACKET_OK);
 assert(f->dispatches<256);f->events[f->dispatches++].submitted=*s;return f->dispatch_result;}
static struct event *event(struct fixture *f,uint64_t op)
{for(unsigned i=0;i<f->dispatches;i++)if(f->events[i].submitted.operation==op)return &f->events[i];assert(0);return NULL;}
static int poll(void *data,uint64_t ctx,uint64_t op,uint64_t ns,struct remesa_gpu_receipt *r)
{struct fixture *f=data;struct event *e=event(f,op);(void)ns;assert(ctx==e->submitted.context);
 *r=(struct remesa_gpu_receipt){ctx,op,e->retired,e->published,e->failed};if(f->invalid==1)++r->operation;if(f->invalid==2){r->published=true;r->retired=false;}if(f->invalid==3)r->published=false;return 0;}
static int release(void *data,uint64_t ctx,uint64_t op)
{struct fixture *f=data;struct event *e=event(f,op);assert(ctx==e->submitted.context&&e->retired&&e->published&&!e->released);
 if(f->fail==6)return EIO;
 if(f->release_retries){--f->release_retries;return EAGAIN;}
 e->released=true;++f->releases;return 0;}
static const struct remesa_persistent_ops ops={create,pin,gpu_map,gpu_unmap,unpin,destroy,cpu_map,cpu_unmap,sync_device,dispatch,poll,release};
static void open_device(struct fixture *f,struct remesa_device *d,uint64_t context)
{struct remesa_persistent_config c={.ops=&ops,.priv=f,.context=context,.va_start=0x100000,.va_end=0x200000,.byte_budget=65536,.offline_test_backend=true};assert(!remesa_persistent_open(d,&c));}
static void published(struct fixture *f,unsigned i) {f->events[i].retired=f->events[i].published=true;}
static void packet(struct remesa_draw_packet *p)
{const float v[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};const uint16_t i[]={0,1,2};assert(!remesa_draw_packet_build(p,v,3,i,3,0xffff00ff));}
static void drop(struct pipe_resource *p) {if(!--p->reference.count)p->screen->resource_destroy(p->screen,p);}
static struct pipe_resource *buffer(struct pipe_screen *s,unsigned n,unsigned bind)
{struct pipe_resource t={0};t.target=PIPE_BUFFER;t.format=PIPE_FORMAT_R8_UNORM;t.width0=n;t.height0=t.depth0=t.array_size=1;t.bind=bind;return s->resource_create(s,&t);}
static void bind(struct pipe_context *c,struct pipe_resource *v,struct pipe_resource *t)
{struct pipe_vertex_buffer vb={0};vb.buffer.resource=v;++v->reference.count;c->set_vertex_buffers(c,1,&vb);
 struct pipe_vertex_element e={.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT,.src_stride=16};void *ve=c->create_vertex_elements_state(c,1,&e);assert(ve);c->bind_vertex_elements_state(c,ve);
 /* Keep the layout alive through draws. The context-owned registry tears it
  * down with this fixture context; deleting a bound layout invalidates draw. */
 struct pipe_framebuffer_state fb={0};fb.width=fb.height=32;fb.nr_cbufs=1;fb.cbufs[0].texture=t;fb.cbufs[0].format=t->format;c->set_framebuffer_state(c,&fb);assert(!remesa_bind_diagnostic_pipeline(c,0xffff00ff));}
static void draw(struct pipe_context *c,struct pipe_resource *i)
{struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1,.index_size=2,.index.resource=i};struct pipe_draw_start_count_bias dc={.count=3};c->draw_vbo(c,&info,0,NULL,&dc,1);}
static void gallium_reuse(void)
{
 struct fixture f={.dispatch_result=REMESA_ISSUED};struct remesa_device d={0};open_device(&f,&d,101);
 struct pipe_screen *s=remesa_screen_create(&d);assert(s&&strstr(s->get_name(s),"NO SGX"));
 struct pipe_context *a=s->context_create(s,NULL,0),*b=s->context_create(s,NULL,0);
 struct pipe_resource *v=buffer(s,48,PIPE_BIND_VERTEX_BUFFER),*i=buffer(s,6,PIPE_BIND_INDEX_BUFFER);
 struct pipe_resource rt={0};rt.target=PIPE_TEXTURE_2D;rt.format=PIPE_FORMAT_B8G8R8A8_UNORM;rt.width0=rt.height0=32;rt.depth0=rt.array_size=1;rt.bind=PIPE_BIND_RENDER_TARGET;
 struct pipe_resource *t=s->resource_create(s,&rt);assert(v&&i&&t&&f.creates==3&&f.maps==3);
 const float vert[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};const uint16_t ix[]={0,1,2};struct pipe_transfer *tr=NULL;struct pipe_box box={.width=48,.height=1,.depth=1};
 void *p=a->buffer_map(a,v,0,PIPE_MAP_WRITE,&box,&tr);assert(p);memcpy(p,vert,48);a->buffer_unmap(a,tr);
 box.width=6;p=a->buffer_map(a,i,0,PIPE_MAP_WRITE,&box,&tr);assert(p);memcpy(p,ix,6);a->buffer_unmap(a,tr);bind(a,v,t);bind(b,v,t);
 draw(a,i);assert(!remesa_context_error(a));a->destroy(a);
 box.width=box.height=32;assert(!b->texture_map(b,t,0,PIPE_MAP_READ|PIPE_MAP_DONTBLOCK,&box,&tr));
 f.events[0].retired=true;assert(!b->texture_map(b,t,0,PIPE_MAP_READ,&box,&tr)&&f.releases==0);
 published(&f,0);p=b->texture_map(b,t,0,PIPE_MAP_READ,&box,&tr);assert(p);b->texture_unmap(b,tr);
 uint32_t handle=f.events[0].submitted.gem_handle;uint64_t va=f.events[0].submitted.gpu_va;
 struct pipe_fence_handle *old=NULL;
 for(unsigned frame=1;frame<=4;frame++) {
  box.width=48;box.height=1;p=b->buffer_map(b,v,0,PIPE_MAP_WRITE,&box,&tr);assert(p);((float *)p)[0]=8+(float)frame;b->buffer_unmap(b,tr);
  draw(b,i);assert(!remesa_context_error(b));assert(f.events[frame].submitted.operation==frame+1);
  assert(f.events[frame].submitted.gem_handle==handle&&f.events[frame].submitted.gpu_va==va);
  assert(f.events[frame].submitted.packet->positions[0][0]==8+(float)frame);
  if(frame==1)b->flush(b,&old,0);
  published(&f,frame);struct pipe_fence_handle *now=NULL;b->flush(b,&now,0);assert(now&&s->fence_finish(s,b,now,0));s->fence_reference(s,&now,NULL);
  if(frame>1)assert(s->fence_finish(s,b,old,0));
 }
 assert(f.creates==3&&f.pins==3&&f.maps==3&&f.releases==5);
 s->fence_reference(s,&old,NULL);b->destroy(b);drop(v);drop(i);drop(t);s->destroy(s);
 assert(f.creates==f.destroys&&f.maps==f.unmaps&&f.pins==f.unpins);assert(!remesa_persistent_close(&d));
}
static void memory_and_detach(void)
{
 struct fixture f={.dispatch_result=REMESA_ISSUED};struct remesa_device d={0};open_device(&f,&d,202);
 void *b,*p,*fence=NULL;struct remesa_draw_packet packet_data;packet(&packet_data);
 for(unsigned stage=1;stage<=3;stage++) {f.fail=stage;assert(d.ops->allocate(d.priv,4096,&b)==ENOMEM&&!b);assert(f.creates==f.destroys);}
 f.fail=0;assert(!d.ops->allocate(d.priv,4096,&b));assert(!d.ops->submit(d.priv,b,&packet_data,&fence));
 packet_data.positions[0][0]=99;assert(f.events[0].submitted.packet->positions[0][0]==8);
 d.ops->destroy_bo(d.priv,b);d.ops->destroy_fence(d.priv,fence);assert(remesa_persistent_close(&d)==EBUSY);
 assert(f.destroys==3);published(&f,0);assert(!remesa_persistent_collect(&d));assert(f.destroys==4);
 void *bos[16];for(unsigned i=0;i<16;i++)assert(!d.ops->allocate(d.priv,4096,&bos[i]));
 assert(d.ops->allocate(d.priv,1,&b)==ENOSPC&&!b);assert(d.ops->allocate(d.priv,SIZE_MAX,&b)==EINVAL);
 f.fail=5;assert(d.ops->map(d.priv,bos[0],true,&p)==ENOMEM&&!p);f.fail=0;
 assert(!d.ops->map(d.priv,bos[0],true,&p));d.ops->destroy_bo(d.priv,bos[0]);assert(remesa_persistent_close(&d)==EBUSY);d.ops->unmap(d.priv,bos[0],true);
 for(unsigned i=1;i<16;i++)d.ops->destroy_bo(d.priv,bos[i]);
 for(unsigned i=0;i<40;i++){assert(!d.ops->allocate(d.priv,3,&b));d.ops->destroy_bo(d.priv,b);}
 assert(f.creates==f.destroys&&f.maps==f.unmaps&&f.pins==f.unpins);assert(!remesa_persistent_close(&d));
}
static void ranges_and_ordering(void)
{
 struct fixture f={.dispatch_result=REMESA_ISSUED},other={.dispatch_result=REMESA_ISSUED};struct remesa_device d={0},foreign={0};
 struct remesa_persistent_config c={.ops=&ops,.priv=&f,.context=203,.va_start=0x100000,.va_end=0x102000,.byte_budget=65536,.offline_test_backend=true};
 assert(!remesa_persistent_open(&d,&c));assert(remesa_persistent_open(&d,&c)==EBUSY);
 open_device(&other,&foreign,204);void *bo,*another,*ptr=NULL;
 assert(!foreign.ops->allocate(foreign.priv,4096,&bo));assert(d.ops->map(d.priv,bo,true,&ptr)==EINVAL&&!ptr);
 assert(d.ops->wait(d.priv,(void *)1,0,true)==EINVAL);d.ops->destroy_bo(d.priv,bo);
 foreign.ops->destroy_bo(foreign.priv,bo);assert(!remesa_persistent_close(&foreign));
 assert(!d.ops->allocate(d.priv,4096,&bo)&&!d.ops->allocate(d.priv,4096,&another));
 void *third=NULL;unsigned created=f.creates;assert(d.ops->allocate(d.priv,1,&third)==ENOSPC&&!third&&f.creates==created);
 d.ops->destroy_bo(d.priv,bo);assert(d.ops->allocate(d.priv,4097,&third)==ENOSPC&&!third);d.ops->destroy_bo(d.priv,another);
 assert(!d.ops->allocate(d.priv,4097,&third));assert(f.gems[f.creates-1].va==0x100000&&f.gems[f.creates-1].length==8192);
 d.ops->destroy_bo(d.priv,third);assert(!remesa_persistent_close(&d));
 memset(&f,0,sizeof(f));f.dispatch_result=REMESA_NOT_ISSUED;open_device(&f,&d,205);
 struct remesa_draw_packet p;packet(&p);void *fence=(void *)1;
 assert(!d.ops->allocate(d.priv,4096,&bo));assert(d.ops->submit(d.priv,bo,&p,&fence)==EOPNOTSUPP&&!fence);
 assert(!d.ops->map(d.priv,bo,true,&ptr));d.ops->unmap(d.priv,bo,true);d.ops->destroy_bo(d.priv,bo);assert(!remesa_persistent_close(&d));
 memset(&f,0,sizeof(f));f.dispatch_result=REMESA_ISSUED;open_device(&f,&d,206);
 void *buffers[8],*fences[8];
 for(unsigned i=0;i<8;i++){assert(!d.ops->allocate(d.priv,4096,&buffers[i]));assert(!d.ops->submit(d.priv,buffers[i],&p,&fences[i]));}
 fence=(void *)1;assert(d.ops->submit(d.priv,buffers[0],&p,&fence)==EBUSY&&!fence&&f.dispatches==8);
 published(&f,7);assert(d.ops->wait(d.priv,fences[7],0,true)==EAGAIN&&f.releases==0);
 for(unsigned i=0;i<7;i++){published(&f,i);assert(!d.ops->wait(d.priv,fences[i],0,true));d.ops->destroy_fence(d.priv,fences[i]);}
 assert(!d.ops->wait(d.priv,fences[7],0,true));d.ops->destroy_fence(d.priv,fences[7]);
 assert(!d.ops->submit(d.priv,buffers[7],&p,&fences[7])&&f.events[8].submitted.operation==9);
 published(&f,8);assert(!d.ops->wait(d.priv,fences[7],0,true));d.ops->destroy_fence(d.priv,fences[7]);
 for(unsigned i=0;i<8;i++)d.ops->destroy_bo(d.priv,buffers[i]);
 assert(f.creates==f.destroys&&f.releases==9);assert(!remesa_persistent_close(&d));
}
/* Exercise ordered publication through real pinned Mesa callbacks. Completion
 * receipts are fixtures, never pixel output. A destroyed context's earlier
 * fence still blocks acknowledgement of a newer surface writer. */
static void gallium_ordered_surfaces(void)
{
 struct fixture f={.dispatch_result=REMESA_ISSUED};struct remesa_device d={0};open_device(&f,&d,207);
 struct pipe_screen *s=remesa_screen_create(&d);assert(s);
 struct pipe_context *a=s->context_create(s,NULL,0),*b=s->context_create(s,NULL,0);
 struct pipe_resource *v=buffer(s,48,PIPE_BIND_VERTEX_BUFFER),*i=buffer(s,6,PIPE_BIND_INDEX_BUFFER);
 struct pipe_resource rt={0};rt.target=PIPE_TEXTURE_2D;rt.format=PIPE_FORMAT_B8G8R8A8_UNORM;rt.width0=rt.height0=32;rt.depth0=rt.array_size=1;rt.bind=PIPE_BIND_RENDER_TARGET;
 struct pipe_resource *x=s->resource_create(s,&rt),*y=s->resource_create(s,&rt);assert(v&&i&&x&&y);
 const float vert[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};const uint16_t ix[]={0,1,2};
 struct pipe_transfer *tr=NULL;struct pipe_box box={.width=48,.height=1,.depth=1};
 void *p=a->buffer_map(a,v,0,PIPE_MAP_WRITE,&box,&tr);assert(p);memcpy(p,vert,48);a->buffer_unmap(a,tr);
 box.width=6;p=a->buffer_map(a,i,0,PIPE_MAP_WRITE,&box,&tr);assert(p);memcpy(p,ix,6);a->buffer_unmap(a,tr);
 bind(a,v,x);bind(b,v,y);draw(a,i);draw(b,i);assert(!remesa_context_error(a)&&!remesa_context_error(b));
 struct pipe_fence_handle *old=NULL,*newer=NULL;a->flush(a,&old,0);b->flush(b,&newer,0);a->destroy(a);
 assert(f.dispatches==2&&f.events[0].submitted.operation==1&&f.events[1].submitted.operation==2);
 published(&f,1);assert(!s->fence_finish(s,b,newer,0)&&f.releases==0);
 box.width=box.height=32;assert(!b->texture_map(b,y,0,PIPE_MAP_READ|PIPE_MAP_DONTBLOCK,&box,&tr));
 assert(f.releases==0&&f.destroys==0);
 published(&f,0);assert(s->fence_finish(s,b,newer,0)&&f.releases==2);
 assert(s->fence_finish(s,b,old,0)&&f.releases==2);
 assert(s->fence_finish(s,b,newer,0)&&f.releases==2); /* idempotent fence, no duplicate ack */
 p=b->texture_map(b,y,0,PIPE_MAP_READ,&box,&tr);assert(p);b->texture_unmap(b,tr);
 s->fence_reference(s,&old,NULL);s->fence_reference(s,&newer,NULL);b->destroy(b);
 drop(v);drop(i);drop(x);drop(y);s->destroy(s);
 assert(f.creates==f.destroys&&f.maps==f.unmaps&&f.pins==f.unpins);assert(!remesa_persistent_close(&d));
}
static void detached_slot_reuse(void)
{
 struct fixture f={.dispatch_result=REMESA_ISSUED};struct remesa_device d={0};open_device(&f,&d,208);
 struct remesa_draw_packet p;packet(&p);void *bo[2],*fence[2];
 for(unsigned i=0;i<2;i++){assert(!d.ops->allocate(d.priv,4096,&bo[i]));assert(!d.ops->submit(d.priv,bo[i],&p,&fence[i]));}
 published(&f,0);assert(!d.ops->wait(d.priv,fence[0],0,true));d.ops->destroy_fence(d.priv,fence[0]);
 assert(!d.ops->submit(d.priv,bo[0],&p,&fence[0])&&f.events[2].submitted.operation==3);
 published(&f,2);d.ops->destroy_fence(d.priv,fence[1]);d.ops->destroy_fence(d.priv,fence[0]);
 assert(f.releases==1);published(&f,1);assert(!remesa_persistent_collect(&d)&&f.releases==3);
 for(unsigned i=0;i<2;i++)d.ops->destroy_bo(d.priv,bo[i]);
 assert(!remesa_persistent_close(&d));
 assert(f.pins==f.unpins&&f.maps==f.unmaps&&f.creates==f.destroys);
}
/* Failure storage is intentionally retained globally: no qualified reset
 * contract exists. Testing EBUSY/quarantine is not a license to free it. */
static struct remesa_device retained[7];static struct fixture failures[7];
static void failures_retained(void)
{
 for(unsigned n=0;n<5;n++) {
  struct fixture *f=&failures[n];f->dispatch_result=n==3?REMESA_ISSUANCE_AMBIGUOUS:REMESA_ISSUED;struct remesa_device *d=&retained[n];open_device(f,d,300+n);
  void *bo,*fence=NULL,*p=NULL;struct remesa_draw_packet request;packet(&request);assert(!d->ops->allocate(d->priv,4096,&bo));
  if(n==4) {f->fail=4;d->ops->destroy_bo(d->priv,bo);assert(remesa_persistent_close(d)==EBUSY&&f->unmaps==0&&f->destroys==0);continue;}
  int e=d->ops->submit(d->priv,bo,&request,&fence);assert(fence);
  if(n==3)assert(e==EIO);else {
   assert(!e);published(f,0);
   if(n==0)f->invalid=1;
   if(n==1)f->invalid=2;
   if(n==2)f->fail=6;
   assert(d->ops->wait(d->priv,fence,0,true)==EIO);
  }
  assert(d->ops->map(d->priv,bo,true,&p)==EIO&&!p);
  unsigned calls=f->dispatches;void *next=NULL;assert(d->ops->submit(d->priv,bo,&request,&next)==EIO&&!next&&f->dispatches==calls);
  d->ops->destroy_fence(d->priv,fence);d->ops->destroy_bo(d->priv,bo);
  assert(remesa_persistent_close(d)==EBUSY&&f->destroys==0&&f->unmaps==0);
 }
}
static void failed_dependency(void)
{
 struct fixture *f=&failures[5];f->dispatch_result=REMESA_ISSUED;
 struct remesa_device *d=&retained[5];open_device(f,d,305);
 struct remesa_draw_packet p;packet(&p);void *bo[2],*fence[2],*map=NULL;
 for(unsigned i=0;i<2;i++){assert(!d->ops->allocate(d->priv,4096,&bo[i]));assert(!d->ops->submit(d->priv,bo[i],&p,&fence[i]));}
 published(f,0);f->invalid=1;assert(d->ops->wait(d->priv,fence[0],0,true)==EIO);f->invalid=0;
 published(f,1);assert(d->ops->wait(d->priv,fence[1],0,true)==EIO&&f->releases==0);
 assert(d->ops->map(d->priv,bo[1],false,&map)==EIO&&!map);
 for(unsigned i=0;i<2;i++){d->ops->destroy_fence(d->priv,fence[i]);d->ops->destroy_bo(d->priv,bo[i]);}
 assert(remesa_persistent_close(d)==EBUSY&&!f->destroys&&!f->unmaps&&!f->unpins);
}

static void release_visibility_regression(void)
{
 struct fixture *f=&failures[6];struct remesa_device *d=&retained[6];
 f->dispatch_result=REMESA_ISSUED;open_device(f,d,306);
 void *bo=NULL,*fence=NULL,*map=NULL;struct remesa_draw_packet request;packet(&request);
 assert(!d->ops->allocate(d->priv,4096,&bo));assert(!d->ops->submit(d->priv,bo,&request,&fence));
 published(f,0);f->release_retries=1;assert(d->ops->wait(d->priv,fence,0,true)==EAGAIN);
 f->invalid=3;assert(d->ops->wait(d->priv,fence,0,true)==EIO);
 assert(!f->releases&&d->ops->map(d->priv,bo,false,&map)==EIO&&!map);
 d->ops->destroy_fence(d->priv,fence);d->ops->destroy_bo(d->priv,bo);
 assert(remesa_persistent_close(d)==EBUSY&&!f->destroys&&!f->unpins&&!f->unmaps);
}

static void bounded_sequences(void)
{
 const unsigned lengths[]={2,10,100};
 for(unsigned n=0;n<3;n++) {
  struct fixture f={0};struct remesa_device d={0};f.dispatch_result=REMESA_ISSUED;open_device(&f,&d,500+n);
  void *bo=NULL;assert(!d.ops->allocate(d.priv,4096,&bo));struct remesa_draw_packet request;packet(&request);
  uint64_t previous=0;
  for(unsigned i=0;i<lengths[n];i++) {
   void *fence=NULL,*map=NULL;assert(!d.ops->submit(d.priv,bo,&request,&fence));
   assert(f.events[i].submitted.operation>previous);previous=f.events[i].submitted.operation;
   assert(d.ops->wait(d.priv,fence,0,true)==EAGAIN);assert(d.ops->map(d.priv,bo,false,&map)==EBUSY&&!map);
   published(&f,i);f.release_retries=2;
   for(unsigned retry=0;retry<2;retry++) {
    assert(d.ops->wait(d.priv,fence,0,true)==EAGAIN);assert(f.releases==i);
    assert(d.ops->map(d.priv,bo,true,&map)==EBUSY&&!map);
    void *blocked=NULL;assert(d.ops->submit(d.priv,bo,&request,&blocked)==EBUSY&&!blocked&&f.dispatches==i+1);
   }
   assert(!d.ops->wait(d.priv,fence,0,true));assert(!d.ops->wait(d.priv,fence,0,true));
   assert(f.releases==i+1);d.ops->destroy_fence(d.priv,fence);
  }
  assert(f.creates==1&&f.pins==1&&f.maps==1&&f.dispatches==lengths[n]);
  d.ops->destroy_bo(d.priv,bo);assert(f.destroys==1&&f.unpins==1&&f.unmaps==1);assert(!remesa_persistent_close(&d));
 }
}

int main(void)
{
 struct remesa_device d={0};struct remesa_persistent_config bad={0};assert(remesa_persistent_open(&d,&bad)==EINVAL);
 bounded_sequences();release_visibility_regression();gallium_reuse();gallium_ordered_surfaces();detached_slot_reuse();memory_and_detach();ranges_and_ordering();failures_retained();failed_dependency();
 puts("PASS: bounded 2/10/100 mock operations, acknowledgement backpressure, no redispatch; Gallium persistent handle/VA/pins across 5 oracle submissions; rollback, CPU-map failure, byte/VA/job exhaustion, cross-device rejection, 40 allocation cycles, detached retirement, ordered cross-context publication of out-of-order mock receipts dependency-failure quarantine, slot-reuse drain and 6 retained failure cases; NO SGX");
}
