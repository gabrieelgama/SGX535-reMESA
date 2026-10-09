/* SPDX-License-Identifier: MIT
 * Real Gallium callbacks, packet recording and explicitly SIMULATED retirement.
 * CPU uniform specialization only; zero pixels, shader ISA or SGX execution. */
#include "uniform_nir_fixture.h"
#include "vertex_nir_fixture.h"
#define main inherited_driver_main
#include "test_driver.c"
#undef main
#ifdef NDEBUG
#error Callback tests require assertions enabled
#endif
static bool fail_next_malloc;
void *__real_malloc(size_t);
void *__wrap_malloc(size_t bytes)
{ if(fail_next_malloc){fail_next_malloc=false;return NULL;}return __real_malloc(bytes); }
static void simulate_finish(struct recorder *rec,struct pipe_screen *s,struct pipe_context *ctx)
{
 struct pipe_fence_handle *f=NULL;ctx->flush(ctx,&f,0);assert(f&&!s->fence_finish(s,ctx,f,0));
 rec->last->retired=true;rec->last->target->published=true; /* SIMULATED fixture. */
 assert(s->fence_finish(s,ctx,f,0));s->fence_reference(s,&f,NULL);
}
int main(void)
{
 struct recorder rec={0};struct remesa_device dev={.ops=&ops,.priv=&rec,.offline_test_backend=true};
 struct pipe_screen *s=remesa_screen_create(&dev);assert(s);struct pipe_context *ctx=s->context_create(s,NULL,0);assert(ctx);
 struct pipe_resource *vb=buffer(s,48,PIPE_BIND_VERTEX_BUFFER),*cb=buffer(s,64,PIPE_BIND_CONSTANT_BUFFER);assert(vb&&cb);
 const float clip[3][4]={{-.5f,-.5f,0,1},{.5f,-.5f,0,1},{-.5f,.5f,0,1}};ctx->buffer_subdata(ctx,vb,0,0,48,clip);assert(!remesa_context_error(ctx));
 struct pipe_vertex_buffer v={.buffer.resource=vb};++vb->reference.count;ctx->set_vertex_buffers(ctx,1,&v);
 struct pipe_vertex_element e={.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT,.src_stride=16};void *ve=ctx->create_vertex_elements_state(ctx,1,&e);assert(ve);ctx->bind_vertex_elements_state(ctx,ve);
 struct pipe_shader_state sh={.type=PIPE_SHADER_IR_NIR,.ir.nir=vertex_fixture()};void *vs=ctx->create_vs_state(ctx,&sh);assert(vs);ctx->bind_vs_state(ctx,vs);
 struct fixture fs_nir=make(16);sh.ir.nir=fs_nir.s;void *fs=ctx->create_fs_state(ctx,&sh);assert(fs);ctx->bind_fs_state(ctx,fs);
 struct pipe_blend_state blend={0};blend.rt[0].colormask=PIPE_MASK_RGBA;struct pipe_depth_stencil_alpha_state depth={0};struct pipe_rasterizer_state raster={0};raster.fill_front=raster.fill_back=PIPE_POLYGON_MODE_FILL;
 void *bs=ctx->create_blend_state(ctx,&blend),*ds=ctx->create_depth_stencil_alpha_state(ctx,&depth),*rs=ctx->create_rasterizer_state(ctx,&raster);assert(bs&&ds&&rs);
 ctx->bind_blend_state(ctx,bs);ctx->bind_depth_stencil_alpha_state(ctx,ds);ctx->bind_rasterizer_state(ctx,rs);
 struct pipe_viewport_state viewport={.scale={16,16,.5f},.translate={16,16,.5f}};ctx->set_viewport_states(ctx,0,1,&viewport);
 struct pipe_resource rt={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,.width0=32,.height0=32,.depth0=1,.array_size=1,.bind=PIPE_BIND_RENDER_TARGET};
 struct pipe_resource *target=s->resource_create(s,&rt);assert(target);struct pipe_framebuffer_state fb={.width=32,.height=32,.nr_cbufs=1};fb.cbufs[0].texture=target;fb.cbufs[0].format=target->format;ctx->set_framebuffer_state(ctx,&fb);
 struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1};struct pipe_draw_start_count_bias draw={.count=3};
 ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(remesa_context_error(ctx)==ENODATA&&!rec.draws);
 uint32_t bits[8]={0,0,0,0,0x3f800000,0,0x3f800000,0x3f800000};
 struct pipe_constant_buffer user={.user_buffer=bits,.buffer_size=32};
 fail_next_malloc=true;ctx->set_constant_buffer(ctx,PIPE_SHADER_FRAGMENT,0,false,&user);
 assert(remesa_context_error(ctx)==ENOMEM&&!fail_next_malloc&&!rec.draws);
 ctx->bind_fs_state(ctx,fs);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);
 assert(remesa_context_error(ctx)==ENOTSUP&&!rec.draws); /* Rejection is sticky. */
 ctx->set_constant_buffer(ctx,PIPE_SHADER_FRAGMENT,0,false,&user);assert(!remesa_context_error(ctx));
 memset(bits,0,sizeof(bits)); /* Snapshot owned by the context, not this pointer. */
 ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!remesa_context_error(ctx)&&rec.draws==1&&rec.packet.argb==0xffff00ff);simulate_finish(&rec,s,ctx);
 const uint32_t colors[2][4]={{0,0x3f800000,0,0x3f800000},{0x3f800000,0x3f800000,0,0x3f800000}};
 const uint32_t expected[2]={0xff00ff00,0xffffff00};
 /* Rebinding a borrowed handle remains safe when its only reference is the
  * existing context binding. The input resource must be retained before the
  * old binding is dropped. No external reference rescues a buggy swap. */
 {
  struct pipe_context *lease=s->context_create(s,NULL,0);assert(lease);
  struct pipe_resource *alias=buffer(s,64,PIPE_BIND_CONSTANT_BUFFER);assert(alias);
  struct pipe_constant_buffer binding={.buffer=alias,.buffer_offset=16,.buffer_size=32};
  lease->set_constant_buffer(lease,PIPE_SHADER_FRAGMENT,0,false,&binding);assert(alias->reference.count==2);
  resource_drop(alias);assert(alias->reference.count==1);unsigned freed=rec.frees;
  lease->set_constant_buffer(lease,PIPE_SHADER_FRAGMENT,0,false,&binding);
  assert(!remesa_context_error(lease)&&rec.frees==freed&&alias->reference.count==1);
  /* An invalid replacement fails closed, drops its temporary incoming ref,
   * releases the sole old binding, and leaves no stale backing retained. */
  binding.buffer_size=65;
  lease->set_constant_buffer(lease,PIPE_SHADER_FRAGMENT,0,false,&binding);
  assert(remesa_context_error(lease)==ENOTSUP&&rec.frees==freed+1);
  lease->set_constant_buffer(lease,PIPE_SHADER_FRAGMENT,0,false,NULL);assert(!remesa_context_error(lease));
  struct pipe_resource *rejected=buffer(s,32,PIPE_BIND_VERTEX_BUFFER);assert(rejected);
  binding=(struct pipe_constant_buffer){.buffer=rejected,.buffer_size=32};freed=rec.frees;
  lease->set_constant_buffer(lease,PIPE_SHADER_FRAGMENT,0,true,&binding);
  assert(remesa_context_error(lease)==ENOTSUP&&rec.frees==freed+1);
  lease->destroy(lease);
 }
 struct pipe_constant_buffer resource={.buffer=cb,.buffer_offset=16,.buffer_size=32};
 ctx->set_constant_buffer(ctx,PIPE_SHADER_FRAGMENT,0,false,&resource);assert(!remesa_context_error(ctx)&&cb->reference.count==2);
 for(unsigned i=0;i<2;i++) {
  /* Persistent constant GEM-style backing reused; fresh target avoids claiming
   * unqualified framebuffer preservation from this mock transport. */
  struct pipe_resource *next=s->resource_create(s,&rt);assert(next);fb.cbufs[0].texture=next;ctx->set_framebuffer_state(ctx,&fb);resource_drop(target);target=next;
  ctx->buffer_subdata(ctx,cb,0,32,16,colors[i]);assert(!remesa_context_error(ctx));
  ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!remesa_context_error(ctx)&&rec.draws==2+i&&rec.packet.argb==expected[i]);simulate_finish(&rec,s,ctx);
 }
 struct pipe_resource *next=s->resource_create(s,&rt);assert(next);fb.cbufs[0].texture=next;ctx->set_framebuffer_state(ctx,&fb);resource_drop(target);target=next;
 struct pipe_transfer *transfer=NULL;struct pipe_box box={.width=16,.height=1,.depth=1};
 void *mapped=ctx->buffer_map(ctx,cb,0,PIPE_MAP_READ,&box,&transfer);assert(mapped&&transfer);
 ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(remesa_context_error(ctx)==EBUSY&&rec.draws==3);ctx->buffer_unmap(ctx,transfer);
 for(unsigned mode=1;mode<=2;mode++) {
  rec.map_mode=mode;ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(remesa_context_error(ctx)==(mode==1?EPROTO:EIO)&&rec.draws==3&&rec.maps==rec.unmaps);
 }
 rec.map_mode=0;
 const uint32_t unknown[4]={0x3f000000,0,0x3f800000,0x3f800000};ctx->buffer_subdata(ctx,cb,0,32,16,unknown);
 ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(remesa_context_error(ctx)==ENOTSUP&&rec.draws==3);
 ctx->set_constant_buffer(ctx,PIPE_SHADER_FRAGMENT,0,false,NULL);assert(cb->reference.count==1);
 for(unsigned mode=0;mode<5;mode++) {
  struct pipe_constant_buffer bad={.user_buffer=bits,.buffer_size=32};unsigned stage=PIPE_SHADER_FRAGMENT,slot=0;
  if(mode==0)bad.buffer_size=15;if(mode==1)bad.buffer_offset=16;if(mode==2)slot=1;if(mode==3)stage=PIPE_SHADER_VERTEX;
  if(mode==4){bad.buffer=vb;bad.user_buffer=NULL;bad.buffer_offset=0;}
  ctx->set_constant_buffer(ctx,stage,slot,false,&bad);assert(remesa_context_error(ctx));
  ctx->bind_fs_state(ctx,fs);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(rec.draws==3&&remesa_context_error(ctx));
  ctx->set_constant_buffer(ctx,stage,slot,false,NULL);
 }
 /* Explicitly transferred binding reference is consumed exactly once; the
  * context keeps its own reference until teardown. */
 ++cb->reference.count;ctx->set_constant_buffer(ctx,PIPE_SHADER_FRAGMENT,0,true,&resource);assert(cb->reference.count==2);
 ctx->delete_fs_state(ctx,fs);ctx->bind_fs_state(ctx,fs);assert(remesa_context_error(ctx)==EINVAL);
 ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(rec.draws==3&&remesa_context_error(ctx));
 ctx->destroy(ctx);assert(cb->reference.count==1);resource_drop(cb);resource_drop(vb);resource_drop(target);s->destroy(s);
 assert(rec.allocs==rec.frees&&rec.maps==rec.unmaps&&rec.fence_destroys==3);
 puts("PASS actual uniform shader/binding/draw callbacks: snapshot, persistent constants, 3 recorded colors, map failures, stale shader, cleanup; SIMULATED retirement, zero SGX");return 0;
}
