/* SPDX-License-Identifier: MIT
 * Mesa callback/lifetime fixtures. Records packets; never renders or uses SGX. */
#include "remesa_device.h"
#include "remesa_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h"
#ifdef REMESA_TEST_REAL_CSO
#include "gallium/auxiliary/cso_cache/cso_context.h"
#include "nir_builder.h"
#endif
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fixture { unsigned allocations, frees, submissions, fences, retired; };
static int allocate(void *p, size_t n, void **out)
{ struct fixture *f=p; *out=calloc(1,n); if(!*out)return ENOMEM; ++f->allocations; return 0; }
static void destroy_bo(void *p,void *bo)
{ ++((struct fixture *)p)->frees; free(bo); }
static int map_bo(void *p,void *bo,bool write,void **out)
{ (void)p;(void)write;*out=bo;return 0; }
static void unmap_bo(void *p,void *bo,bool write)
{ (void)p;(void)bo;(void)write; }
static int submit(void *p,void *target,const struct remesa_draw_packet *packet,void **out)
{
 struct fixture *f=p;assert(target);assert(remesa_draw_packet_validate(packet)==REMESA_PACKET_OK);
 *out=malloc(1);assert(*out);++f->submissions;++f->fences;return 0;
}
static int wait_fence(void *p,void *fence,uint64_t ns,bool visible)
{ (void)p;(void)ns;(void)visible;assert(fence);return 0; /* Mock completion only. */ }
static void destroy_fence(void *p,void *fence)
{ assert(fence);++((struct fixture *)p)->retired;free(fence); }
static void release(struct pipe_resource *r)
{ if(!--r->reference.count)r->screen->resource_destroy(r->screen,r); }
static struct pipe_resource *buffer(struct pipe_screen *s,unsigned bytes)
{
 struct pipe_resource r={0};r.target=PIPE_BUFFER;r.format=PIPE_FORMAT_R8_UNORM;
 r.width0=bytes;r.height0=r.depth0=r.array_size=1;r.bind=PIPE_BIND_VERTEX_BUFFER;
 return s->resource_create(s,&r);
}
static void draw_check(struct pipe_context *c,struct fixture *f,int error,unsigned submits)
{
 struct pipe_draw_info info={0};struct pipe_draw_start_count_bias draw={0};
 info.mode=MESA_PRIM_TRIANGLES;info.instance_count=1;draw.count=3;
 c->draw_vbo(c,&info,0,NULL,&draw,1);
 assert(remesa_context_error(c)==error&&f->submissions==submits);
}
int main(void)
{
 struct fixture f={0};const struct remesa_device_ops ops={allocate,destroy_bo,map_bo,unmap_bo,submit,wait_fence,destroy_fence};
 struct remesa_device device={.ops=&ops,.priv=&f,.offline_test_backend=true};
 struct pipe_screen *s=remesa_screen_create(&device);assert(s);
 struct pipe_context *c=s->context_create(s,NULL,0);assert(c);
 assert(c->set_constant_buffer&&c->set_stencil_ref&&c->set_blend_color&&c->set_sample_mask);
 struct pipe_stencil_ref sr={{255,17}};c->set_stencil_ref(c,sr);assert(!remesa_context_error(c));
 struct pipe_blend_color bc={{-3,2,.25f,1}};c->set_blend_color(c,&bc);assert(!remesa_context_error(c));
 bc.color[2]=NAN;c->set_blend_color(c,&bc);assert(remesa_context_error(c)==EINVAL);
 bc.color[2]=INFINITY;c->set_blend_color(c,&bc);assert(remesa_context_error(c)==EINVAL);
 c->set_blend_color(c,NULL);assert(remesa_context_error(c)==EINVAL);
 bc.color[2]=.25f;c->set_blend_color(c,&bc);assert(!remesa_context_error(c));
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,0,false,NULL);assert(!remesa_context_error(c));
 struct pipe_constant_buffer empty={0};c->set_constant_buffer(c,PIPE_SHADER_FRAGMENT,0,false,&empty);assert(!remesa_context_error(c));

 const float positions[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};
 struct pipe_resource *vb=buffer(s,sizeof(positions));assert(vb);
 struct pipe_box box={.width=sizeof(positions),.height=1,.depth=1};struct pipe_transfer *tr=NULL;
 void *bytes=c->buffer_map(c,vb,0,PIPE_MAP_WRITE,&box,&tr);assert(bytes&&tr);memcpy(bytes,positions,sizeof(positions));c->buffer_unmap(c,tr);
 struct pipe_vertex_buffer vertex={0};vertex.buffer.resource=vb;++vb->reference.count;c->set_vertex_buffers(c,1,&vertex);
 struct pipe_vertex_element e={0};e.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT;e.src_stride=16;
 void *elements=c->create_vertex_elements_state(c,1,&e);assert(elements);c->bind_vertex_elements_state(c,elements);
 struct pipe_resource rt={0};rt.target=PIPE_TEXTURE_2D;rt.format=PIPE_FORMAT_B8G8R8A8_UNORM;
 rt.width0=rt.height0=32;rt.depth0=rt.array_size=1;rt.bind=PIPE_BIND_RENDER_TARGET;
 struct pipe_resource *target=s->resource_create(s,&rt);assert(target);
 struct pipe_framebuffer_state fb={0};fb.width=fb.height=32;fb.nr_cbufs=1;fb.cbufs[0].texture=target;fb.cbufs[0].format=target->format;
 c->set_framebuffer_state(c,&fb);assert(!remesa_bind_diagnostic_pipeline(c,0xffff00ff));
 draw_check(c,&f,0,1);
 c->set_sample_mask(c,0);draw_check(c,&f,0,1);
 c->set_sample_mask(c,2);draw_check(c,&f,0,1); /* Other sample bits cannot enable sample zero. */
 c->set_sample_mask(c,1);draw_check(c,&f,0,2);

 float uniform=1;struct pipe_constant_buffer cb={.user_buffer=&uniform,.buffer_size=sizeof(uniform)};
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,0,false,&cb);assert(remesa_context_error(c)==ENOTSUP);
 assert(!remesa_bind_diagnostic_pipeline(c,0xffff00ff));draw_check(c,&f,ENOTSUP,2);
 c->set_stencil_ref(c,sr);c->set_sample_mask(c,1);c->set_blend_color(c,&bc);draw_check(c,&f,ENOTSUP,2);
 c->set_constant_buffer(c,PIPE_SHADER_FRAGMENT,0,false,NULL);draw_check(c,&f,ENOTSUP,2);
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,0,false,NULL);draw_check(c,&f,0,3);
 c->set_constant_buffer(c,PIPE_SHADER_FRAGMENT,0,false,&cb);assert(remesa_context_error(c)==ENOTSUP);
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,0,false,NULL);draw_check(c,&f,ENOTSUP,3);
 c->set_constant_buffer(c,PIPE_SHADER_FRAGMENT,0,false,NULL);draw_check(c,&f,0,4);

 struct pipe_resource *owned=buffer(s,16);assert(owned);unsigned freed=f.frees;
 cb=(struct pipe_constant_buffer){.buffer=owned,.buffer_size=16};
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,0,true,&cb);
 assert(remesa_context_error(c)==ENOTSUP&&f.frees==freed+1); /* Sole transferred ref released exactly once. */
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,0,false,NULL);draw_check(c,&f,0,5);
 owned=buffer(s,16);assert(owned);++owned->reference.count;freed=f.frees;cb.buffer=owned;
 c->set_constant_buffer(c,PIPE_SHADER_FRAGMENT,0,true,&cb);
 assert(remesa_context_error(c)==ENOTSUP&&owned->reference.count==1&&f.frees==freed);
 release(owned);assert(f.frees==freed+1);
 c->set_constant_buffer(c,PIPE_SHADER_FRAGMENT,0,false,NULL);draw_check(c,&f,0,6);

 cb=(struct pipe_constant_buffer){.user_buffer=&uniform,.buffer_size=sizeof(uniform)};
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,1,false,&cb);assert(remesa_context_error(c)==ENOTSUP);
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,0,false,NULL);draw_check(c,&f,ENOTSUP,6);
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,1,false,NULL);assert(!remesa_context_error(c));draw_check(c,&f,0,7);
 c->set_constant_buffer(c,PIPE_SHADER_FRAGMENT,31,false,NULL);assert(!remesa_context_error(c));
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,32,false,NULL);assert(remesa_context_error(c)==ENOTSUP);
 c->set_constant_buffer(c,PIPE_SHADER_VERTEX,0,false,NULL);draw_check(c,&f,ENOTSUP,7);
 c->set_constant_buffer(c,PIPE_SHADER_GEOMETRY,0,false,NULL);assert(remesa_context_error(c)==ENOTSUP);
 /* Invalid stages/slots cannot be described as accepted empty uniforms. */
 struct pipe_framebuffer_state unbound={0};int32_t refs=target->reference.count;
 c->set_framebuffer_state(c,&unbound);assert(!remesa_context_error(c)&&target->reference.count==refs-1);
 draw_check(c,&f,ENOTSUP,7);
 c->bind_vertex_elements_state(c,NULL);c->delete_vertex_elements_state(c,elements);
 c->destroy(c);release(vb);release(target);
#ifdef REMESA_TEST_REAL_CSO
 /* Actual pinned Mesa cleanup, isolated from sticky negative-input state. */
 for(unsigned stage=0;stage<PIPE_SHADER_TYPES;stage++) {
  if(stage==PIPE_SHADER_VERTEX||stage==PIPE_SHADER_FRAGMENT) continue;
  assert(!s->shader_caps[stage].max_instructions&&!s->shader_caps[stage].supported_irs);
 }
 c=s->context_create(s,NULL,0);assert(c);
 glsl_type_singleton_init_or_ref();
 assert(s->get_compiler_options);
 const nir_shader_compiler_options *options=s->get_compiler_options(s,PIPE_SHADER_VERTEX);
 assert(options&&s->get_compiler_options(s,PIPE_SHADER_FRAGMENT)==options);
 assert(!s->get_compiler_options(s,PIPE_SHADER_COMPUTE));
 nir_builder builder=nir_builder_init_simple_shader(MESA_SHADER_VERTEX,options,"context identity");
 nir_variable *input=nir_variable_create(builder.shader,nir_var_shader_in,glsl_vec4_type(),"position");
 nir_variable *output=nir_variable_create(builder.shader,nir_var_shader_out,glsl_vec4_type(),"gl_Position");
 input->data.location=VERT_ATTRIB_GENERIC0;output->data.location=VARYING_SLOT_POS;
 nir_deref_instr *id=nir_build_deref_var(&builder,input),*od=nir_build_deref_var(&builder,output);
 nir_store_deref(&builder,od,nir_load_deref(&builder,id),15);
 struct pipe_shader_state shader={.type=PIPE_SHADER_IR_NIR,.ir.nir=builder.shader};
 void *vs=c->create_vs_state(c,&shader);assert(vs);c->bind_vs_state(c,vs);assert(!remesa_context_error(c));
 struct cso_context *cso=cso_create_context(c,CSO_NO_VBUF);assert(cso);
 cso_unbind_context(cso);assert(!remesa_context_error(c));
 cso_destroy_context(cso);assert(!remesa_context_error(c));
 c->delete_vs_state(c,vs);assert(!remesa_context_error(c));c->destroy(c);glsl_type_singleton_decref();
 puts("PASS actual Mesa CSO create/unbind/destroy; no draws or hardware");
#endif
 s->destroy(s);
 assert(f.allocations==f.frees&&f.fences==f.retired);
 puts("PASS Mesa lifecycle callbacks, per-stage unsupported constants, transferred references, sample zero mask; packet recording only, NO SGX");
 return 0;
}
