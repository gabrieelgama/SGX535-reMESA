/* SPDX-License-Identifier: MIT */
#include "remesa_screen.h"
#include "remesa_device.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#ifdef REMESA_WITH_NIR
#include "remesa_shader_program.h"
#include "remesa_draw.h"
#endif
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
struct bo { unsigned refs; bool pending,published; void *bytes; };
struct recorded_programs;
struct fence { struct bo *target; bool retired; struct recorded_programs *programs; };
struct recorder { struct remesa_draw_packet packet; unsigned draws,allocs,frees,maps,unmaps,map_mode; int submit_error; bool allocation_error,error_fence; unsigned fence_destroys; struct fence *last;
 unsigned shader_prepares,shader_submits,shader_allocs,shader_frees;
 int shader_prepare_error;bool shader_prepare_null;
};
static int alloc(void *priv,size_t n,void **out)
{ struct recorder *rec=priv;struct bo *b=calloc(1,sizeof(*b));assert(b);b->bytes=calloc(1,n);assert(b->bytes);b->refs=1;*out=b;++rec->allocs;return rec->allocation_error ? ENOMEM : 0; }
static void drop(void *priv,void *object)
{ struct recorder *rec=priv;struct bo *b=object;if(!--b->refs) { free(b->bytes);free(b);++rec->frees; } }
static int map_bo(void *priv,void *object,bool write,void **out)
{ struct recorder *rec=priv;struct bo *b=object;(void)write;if(b->pending && !b->published) return EBUSY;++rec->maps;*out=rec->map_mode==1?NULL:b->bytes;return rec->map_mode==2?EIO:0; }
static void unmap_bo(void *priv,void *object,bool write) { ++((struct recorder *)priv)->unmaps;(void)object;(void)write; }
static int submit(void *priv,void *target,const struct remesa_draw_packet *packet,void **out)
{
 struct recorder *rec=priv;struct bo *b=target;struct fence *f;
 if(rec->submit_error) { if(rec->error_fence) { f=calloc(1,sizeof(*f));assert(f);f->retired=true;*out=f; } return rec->submit_error; }
 assert(!b->pending);assert(remesa_draw_packet_validate(packet)==REMESA_PACKET_OK);
 rec->packet=*packet;++rec->draws;f=calloc(1,sizeof(*f));assert(f);f->target=b;++b->refs;
 b->pending=true;b->published=false;rec->last=f;*out=f;return 0;
}
static int wait_fence(void *priv,void *object,uint64_t ns,bool published)
{ struct fence *f=object;(void)priv;(void)ns;return f->retired && (!published || f->target->published) ? 0 : ETIMEDOUT; }
#ifdef REMESA_WITH_NIR
/* Recording-only original-shader handoff. No instruction encoding, hardware
 * allocation, pixel generation or claim of executable programs occurs here. */
struct recorded_programs {
 unsigned refs;struct remesa_shader_draw_programs snapshot;
 struct remesa_shader_draw_inputs inputs;
};
static int prepare_shaders(void *priv,const struct remesa_shader_draw_programs *snapshot,void **out)
{
 struct recorder *rec=priv;++rec->shader_prepares;
 assert(!remesa_shader_draw_programs_validate(snapshot));
 if(rec->shader_prepare_null)return rec->shader_prepare_error;
 struct recorded_programs *programs=calloc(1,sizeof(*programs));assert(programs);
 programs->refs=1;programs->snapshot=*snapshot;++rec->shader_allocs;*out=programs;
 return rec->shader_prepare_error;
}
static void destroy_shaders(void *priv,void *object)
{
 struct recorder *rec=priv;struct recorded_programs *programs=object;
 assert(!remesa_shader_draw_programs_validate(&programs->snapshot));
 if(!--programs->refs){free((void *)programs->inputs.vertex_data);
  free((void *)programs->inputs.index_data);++rec->shader_frees;free(programs);}
}
static int submit_shaders(void *priv,void *target,const struct remesa_draw_packet *packet,
                          const struct remesa_shader_draw_inputs *inputs,void *object,void **out)
{
 struct recorder *rec=priv;struct recorded_programs *programs=object;
 assert(programs&&programs->refs&&!remesa_shader_draw_programs_validate(&programs->snapshot));
 assert(programs->snapshot.pipeline.fragment_argb==packet->argb);
 assert(inputs&&inputs->vertex_data&&inputs->vertex_bytes&&inputs->count&&inputs->count%3==0);
 assert(inputs->position_input_slot==programs->snapshot.pipeline.position_input_slot);
 assert(inputs->position_format==PIPE_FORMAT_R32G32_FLOAT||
        inputs->position_format==PIPE_FORMAT_R32G32B32_FLOAT||
        inputs->position_format==PIPE_FORMAT_R32G32B32A32_FLOAT);
 assert(inputs->color_format==PIPE_FORMAT_B8G8R8A8_UNORM&&inputs->color_width==packet->width&&
        inputs->color_height==packet->height&&inputs->color_stride==128&&inputs->color_bytes==4096);
 assert(!programs->inputs.vertex_data&&!programs->inputs.index_data);
 programs->inputs=*inputs;
 void *vertices=malloc(inputs->vertex_bytes);assert(vertices);
 memcpy(vertices,inputs->vertex_data,inputs->vertex_bytes);programs->inputs.vertex_data=vertices;
 if(inputs->index_size) {
  assert(inputs->index_data&&inputs->index_bytes);
  void *indices=malloc(inputs->index_bytes);assert(indices);
  memcpy(indices,inputs->index_data,inputs->index_bytes);programs->inputs.index_data=indices;
 } else assert(!inputs->index_data&&!inputs->index_bytes&&!inputs->index_bias);
 ++rec->shader_submits;
 int error=submit(priv,target,packet,out);
 if(*out){struct fence *f=*out;f->programs=programs;++programs->refs;}
 return error;
}
#endif
static void destroy_fence(void *priv,void *object)
{ struct fence *f=object;struct recorder *rec=priv;assert(f->retired);if(f->target) { f->target->pending=false;drop(priv,f->target); }
#ifdef REMESA_WITH_NIR
 if(f->programs)destroy_shaders(priv,f->programs);
#endif
 ++rec->fence_destroys;free(f); }
static const struct remesa_device_ops ops={.allocate=alloc,.destroy_bo=drop,.map=map_bo,.unmap=unmap_bo,
 .submit=submit,.wait=wait_fence,.destroy_fence=destroy_fence,
#ifdef REMESA_WITH_NIR
 .prepare_shader_programs=prepare_shaders,.submit_shader_programs=submit_shaders,.destroy_shader_programs=destroy_shaders,
#endif
};
static void resource_drop(struct pipe_resource *p)
{ if(!--p->reference.count) p->screen->resource_destroy(p->screen,p); }
static struct pipe_resource *buffer(struct pipe_screen *s,unsigned size,unsigned bind)
{
 struct pipe_resource t={0};t.target=PIPE_BUFFER;t.format=PIPE_FORMAT_R8_UNORM;t.width0=size;
 t.height0=t.depth0=t.array_size=1;t.bind=bind;return s->resource_create(s,&t);
}
int main(void)
{
 struct recorder rec={0};struct remesa_device device={.ops=&ops,.priv=&rec,.offline_test_backend=true};
 struct pipe_screen *screen;struct pipe_context *ctx;struct pipe_resource *vb,*ib,*target;
 struct pipe_resource rt={0};struct pipe_transfer *transfer=NULL;void *ptr;struct pipe_box box={0};
 struct pipe_vertex_buffer vertex={0};struct pipe_vertex_element element={0};void *ve;
 struct pipe_framebuffer_state fb={0};struct pipe_draw_info info={0};struct pipe_draw_start_count_bias d={0};
 struct pipe_fence_handle *f=NULL;struct pipe_surface *surface;struct pipe_surface st={0};
 const float window_positions[3][4]={{8,8,.5,1},{24,8,.5,1},{8,24,.5,1}};
#ifdef REMESA_TEST_CLIP_INPUT
 const float positions[3][4]={{-.5,-.5,0,1},{.5,-.5,0,1},{-.5,.5,0,1}};
#else
 const float (*positions)[4]=window_positions;
#endif
 const uint16_t indices[]={0,1,2};
 assert(!remesa_screen_create(NULL));device.offline_test_backend=false;assert(!remesa_screen_create(&device));device.offline_test_backend=true;
 screen=remesa_screen_create(&device);assert(screen);assert(strstr(screen->get_name(screen),"NO SGX"));
 ctx=screen->context_create(screen,NULL,0);assert(ctx);assert(!screen->context_create(screen,NULL,1));
 rec.allocation_error=true;assert(!buffer(screen,48,PIPE_BIND_VERTEX_BUFFER));assert(rec.allocs==rec.frees);rec.allocation_error=false;
 vb=buffer(screen,sizeof(window_positions),PIPE_BIND_VERTEX_BUFFER);ib=buffer(screen,sizeof(indices),PIPE_BIND_INDEX_BUFFER);assert(vb&&ib);
 box.width=sizeof(window_positions);box.height=box.depth=1;ptr=ctx->buffer_map(ctx,vb,0,PIPE_MAP_WRITE,&box,&transfer);assert(ptr&&transfer);
 memcpy(ptr,positions,sizeof(window_positions));ctx->buffer_unmap(ctx,transfer);
 box.width=sizeof(indices);ptr=ctx->buffer_map(ctx,ib,0,PIPE_MAP_WRITE,&box,&transfer);assert(ptr);memcpy(ptr,indices,sizeof(indices));ctx->buffer_unmap(ctx,transfer);
 box.x=5;box.width=2;assert(!ctx->buffer_map(ctx,ib,0,PIPE_MAP_READ,&box,&transfer));assert(!transfer);box.x=0;
 box.height=2;assert(!ctx->buffer_map(ctx,ib,0,PIPE_MAP_READ,&box,&transfer));assert(!transfer);box.height=1;
 box.z=1;assert(!ctx->buffer_map(ctx,ib,0,PIPE_MAP_READ,&box,&transfer));assert(!transfer);box.z=0;
 rt.target=PIPE_TEXTURE_2D;rt.format=PIPE_FORMAT_B8G8R8A8_UNORM;rt.width0=rt.height0=32;rt.depth0=rt.array_size=1;rt.bind=PIPE_BIND_RENDER_TARGET;
 target=screen->resource_create(screen,&rt);assert(target);rt.width0=64;assert(!screen->resource_create(screen,&rt));
 st.format=target->format;surface=ctx->create_surface(ctx,target,&st);assert(surface);assert(target->reference.count==2);ctx->surface_destroy(ctx,surface);assert(target->reference.count==1);
 assert(!ctx->create_fs_state(ctx,NULL));ctx->clear(ctx,PIPE_CLEAR_COLOR0,NULL,NULL,0,0);assert(remesa_context_error(ctx)==ENOTSUP);
 fb.width=fb.height=32;fb.nr_cbufs=1;fb.cbufs[0].texture=target;fb.cbufs[0].format=target->format;ctx->set_framebuffer_state(ctx,&fb);
 ++vb->reference.count;vertex.buffer.resource=vb;ctx->set_vertex_buffers(ctx,1,&vertex);
 element.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT;element.src_stride=16;ve=ctx->create_vertex_elements_state(ctx,1,&element);assert(ve);ctx->bind_vertex_elements_state(ctx,ve);
 info.mode=MESA_PRIM_TRIANGLES;info.index_size=0;info.instance_count=1;info.index.user=(void *)1;d.count=3;
 ctx->draw_vbo(ctx,&info,0,NULL,&d,1);assert(remesa_context_error(ctx)==ENOTSUP && !rec.draws);
 assert(!remesa_bind_diagnostic_pipeline(ctx,0xffff00ff));
 for(unsigned mode=1;mode<=2;mode++) {
  rec.map_mode=mode;ctx->draw_vbo(ctx,&info,0,NULL,&d,1);
  assert(remesa_context_error(ctx)==(mode==1?EPROTO:EIO)&&!rec.draws&&rec.maps==rec.unmaps);
  info.index_size=2;info.index.resource=ib;ctx->draw_vbo(ctx,&info,0,NULL,&d,1);
  assert(remesa_context_error(ctx)==(mode==1?EPROTO:EIO)&&!rec.draws&&rec.maps==rec.unmaps);
  box=(struct pipe_box){.width=48,.height=1,.depth=1};assert(!ctx->buffer_map(ctx,vb,0,PIPE_MAP_READ,&box,&transfer));assert(!transfer&&rec.maps==rec.unmaps);
 }
 rec.map_mode=0;info.index_size=2;info.index.resource=target;ctx->draw_vbo(ctx,&info,0,NULL,&d,1);assert(remesa_context_error(ctx)==ENOTSUP&&!rec.draws);
 ++target->reference.count;vertex.buffer.resource=target;ctx->set_vertex_buffers(ctx,1,&vertex);assert(remesa_context_error(ctx)==EINVAL);
 ++vb->reference.count;vertex.buffer.resource=vb;ctx->set_vertex_buffers(ctx,1,&vertex);
 info.index_size=0;info.index.user=(void *)1;ctx->draw_vbo(ctx,&info,0,NULL,&d,1);assert(!remesa_context_error(ctx)&&rec.draws==1);
 assert(rec.packet.argb==0xffff00ff && rec.packet.index_count==3 && !memcmp(rec.packet.positions,window_positions,sizeof(window_positions)));
 ctx->flush(ctx,&f,0);assert(f);assert(!screen->fence_finish(screen,ctx,f,0));
 box.width=box.height=32;assert(!ctx->texture_map(ctx,target,0,PIPE_MAP_READ,&box,&transfer));
 /* Test-only completion/publication are explicit independent transitions. No
  * mock pixel is generated, and neither transition is SGX evidence. */
 {
  struct fence *recorded=rec.last;recorded->retired=true;
  assert(!screen->fence_finish(screen,ctx,f,0));recorded->target->published=true;
 }
 assert(screen->fence_finish(screen,ctx,f,0));ptr=ctx->texture_map(ctx,target,0,PIPE_MAP_READ,&box,&transfer);assert(ptr&&transfer->stride==128);
 for(unsigned i=0;i<4096;i++) assert(!((unsigned char *)ptr)[i]);
 ctx->texture_unmap(ctx,transfer);
 screen->fence_reference(screen,&f,NULL);
 /* A failed submit is reported rather than manufacturing a success fence. */
 info.index_size=2;info.index.resource=ib;rec.submit_error=EOPNOTSUPP;rec.error_fence=true;ctx->draw_vbo(ctx,&info,0,NULL,&d,1);assert(remesa_context_error(ctx)==EOPNOTSUPP && rec.draws==1 && rec.fence_destroys==1);ctx->flush(ctx,&f,0);assert(!f);
 info.index_size=0;info.index.user=(void *)1;ctx->draw_vbo(ctx,&info,0,NULL,&d,1);assert(remesa_context_error(ctx)==EOPNOTSUPP);
 info.index_size=2;info.index.resource=ib;d.count=0;ctx->draw_vbo(ctx,&info,0,NULL,&d,1);assert(!remesa_context_error(ctx)&&rec.draws==1);d.count=3;
 ++ib->reference.count;info.take_index_buffer_ownership=true;ctx->draw_vbo(ctx,&info,1,NULL,&d,1);assert(remesa_context_error(ctx)==ENOTSUP&&ib->reference.count==1);info.take_index_buffer_ownership=false;
 info.has_user_indices=true;info.index.user=(void *)1;ctx->draw_vbo(ctx,&info,0,NULL,&d,1);assert(remesa_context_error(ctx)==ENOTSUP);
 ctx->bind_vertex_elements_state(ctx,NULL);ctx->delete_vertex_elements_state(ctx,ve);ctx->destroy(ctx);
 resource_drop(vb);resource_drop(ib);resource_drop(target);assert(rec.allocs==rec.frees&&rec.maps==rec.unmaps);screen->destroy(screen);
 puts("Gallium callback/resource/submission contract PASS (recording backend, zero SGX)");return 0;
}
