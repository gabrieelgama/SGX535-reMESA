/* SPDX-License-Identifier: MIT
 * Actual Gallium callbacks; CPU coordinate preparation and recording transport.
 * Completion flags below are SIMULATED. No pixels or SGX events are generated. */
#include "nir_fixture.h"
#include "vertex_nir_fixture.h"
#define main inherited_driver_main
#include "test_driver.c"
#undef main
#include <math.h>
/* Exercise scalarized identity IO through the actual shader callbacks. The
 * pipeline still records CPU-prepared positions and simulated completion. */
static nir_shader *scalarized_vertex(void)
{
 nir_shader *shader=vertex_fixture();
 nir_function *fn=exec_node_data(nir_function,exec_list_get_head(&shader->functions),node);
 nir_block *block=exec_node_data(nir_block,exec_list_get_head(&fn->impl->body),cf_node.node);
 nir_instr *zero=exec_node_data(nir_instr,exec_list_get_head(&block->instr_list),node);
 nir_intrinsic_instr *input=exec_node_data(nir_intrinsic_instr,zero->node.next,instr.node);
 nir_intrinsic_instr *store=exec_node_data(nir_intrinsic_instr,exec_list_get_tail(&block->instr_list),instr.node);
 nir_alu_instr *vec=rzalloc_size(shader,sizeof(*vec)+4*sizeof(nir_alu_src));
 vec->instr.type=nir_instr_type_alu;vec->instr.block=block;vec->op=nir_op_vec4;
 vec->def.parent_instr=&vec->instr;vec->def.bit_size=32;vec->def.num_components=4;
 for(unsigned i=0;i<4;i++) {
  nir_alu_instr *scalar=rzalloc_size(shader,sizeof(*scalar)+sizeof(nir_alu_src));
  scalar->instr.type=nir_instr_type_alu;scalar->instr.block=block;scalar->op=nir_op_mov;
  scalar->def.parent_instr=&scalar->instr;scalar->def.bit_size=32;scalar->def.num_components=1;
  scalar->src[0].src.ssa=&input->def;scalar->src[0].swizzle[0]=i;
  vec->src[i].src.ssa=&scalar->def;
  exec_node_insert_node_before(&store->instr.node,&scalar->instr.node);
 }
 exec_node_insert_node_before(&store->instr.node,&vec->instr.node);
 store->src[0].ssa=&vec->def;return shader;
}
static void *vertex_shader(struct pipe_context *ctx)
{ struct pipe_shader_state state={.type=PIPE_SHADER_IR_NIR};state.ir.nir=scalarized_vertex();return ctx->create_vs_state(ctx,&state); }
static void *fragment_shader(struct pipe_context *ctx)
{ struct pipe_shader_state state={.type=PIPE_SHADER_IR_NIR};state.ir.nir=fixture(0xffff00ff);return ctx->create_fs_state(ctx,&state); }
static void upload(struct pipe_context *ctx,struct pipe_resource *vb,const float positions[3][4])
{ struct pipe_box box={.width=48,.height=1,.depth=1};struct pipe_transfer *t=NULL;void *p=ctx->buffer_map(ctx,vb,0,PIPE_MAP_WRITE,&box,&t);assert(p&&t);memcpy(p,positions,48);ctx->buffer_unmap(ctx,t); }
int main(void)
{
 struct recorder rec={0};struct remesa_device dev={.ops=&ops,.priv=&rec,.offline_test_backend=true};
 struct pipe_screen *screen=remesa_screen_create(&dev);assert(screen);
 struct pipe_context *ctx=screen->context_create(screen,NULL,0),*other=screen->context_create(screen,NULL,0);assert(ctx&&other);
 struct pipe_resource *vb=buffer(screen,48,PIPE_BIND_VERTEX_BUFFER);
 struct pipe_resource rt={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,.width0=32,.height0=32,.depth0=1,.array_size=1,.bind=PIPE_BIND_RENDER_TARGET};
 struct pipe_resource *target=screen->resource_create(screen,&rt);assert(vb&&target);
 struct pipe_framebuffer_state fb={.width=32,.height=32,.nr_cbufs=1};fb.cbufs[0].texture=target;fb.cbufs[0].format=target->format;ctx->set_framebuffer_state(ctx,&fb);
 struct pipe_vertex_buffer v={.buffer.resource=vb};++vb->reference.count;ctx->set_vertex_buffers(ctx,1,&v);
 struct pipe_vertex_element e={.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT,.src_stride=16};void *ve=ctx->create_vertex_elements_state(ctx,1,&e);assert(ve);ctx->bind_vertex_elements_state(ctx,ve);
 assert(ctx->create_blend_state&&ctx->bind_blend_state&&ctx->delete_blend_state);
 assert(ctx->create_depth_stencil_alpha_state&&ctx->bind_depth_stencil_alpha_state&&ctx->delete_depth_stencil_alpha_state);
 assert(ctx->create_rasterizer_state&&ctx->bind_rasterizer_state&&ctx->delete_rasterizer_state);
 struct pipe_blend_state blend={0};blend.rt[0].colormask=PIPE_MASK_RGBA;
 struct pipe_depth_stencil_alpha_state depth={0};struct pipe_rasterizer_state raster={0};
 raster.fill_front=raster.fill_back=PIPE_POLYGON_MODE_FILL;
 void *bs=ctx->create_blend_state(ctx,&blend),*ds=ctx->create_depth_stencil_alpha_state(ctx,&depth),*rs=ctx->create_rasterizer_state(ctx,&raster);assert(bs&&ds&&rs);
 ctx->bind_blend_state(ctx,bs);ctx->bind_depth_stencil_alpha_state(ctx,ds);ctx->bind_rasterizer_state(ctx,rs);
 struct pipe_viewport_state view={.scale={16,16,.5},.translate={16,16,.5}};
 /* Explicit shader creation negatives: stage, precision, partial output. */
 struct pipe_shader_state rejected={.type=PIPE_SHADER_IR_NIR};rejected.ir.nir=fixture(0xffff00ff);
 assert(!ctx->create_vs_state(ctx,&rejected)&&remesa_context_error(ctx));
 nir_shader *malformed=vertex_fixture();nir_function_impl *impl=nir_shader_get_entrypoint(malformed);nir_block *block=nir_cf_node_as_block(exec_node_data(nir_cf_node,exec_list_get_head(&impl->body),node));
 nir_intrinsic_instr *input=NULL,*output=NULL;nir_foreach_instr(i,block)if(i->type==nir_instr_type_intrinsic){nir_intrinsic_instr *in=nir_instr_as_intrinsic(i);if(in->intrinsic==nir_intrinsic_load_input)input=in;else if(in->intrinsic==nir_intrinsic_store_output)output=in;}
 assert(input&&output);input->def.bit_size=16;rejected.ir.nir=malformed;assert(!ctx->create_vs_state(ctx,&rejected)&&remesa_context_error(ctx));
 malformed=vertex_fixture();impl=nir_shader_get_entrypoint(malformed);block=nir_cf_node_as_block(exec_node_data(nir_cf_node,exec_list_get_head(&impl->body),node));nir_foreach_instr(i,block)if(i->type==nir_instr_type_intrinsic){nir_intrinsic_instr *in=nir_instr_as_intrinsic(i);if(in->intrinsic==nir_intrinsic_store_output)nir_intrinsic_set_write_mask(in,7);}
 rejected.ir.nir=malformed;assert(!ctx->create_vs_state(ctx,&rejected)&&remesa_context_error(ctx));
 void *vs=vertex_shader(ctx),*fs=fragment_shader(ctx);assert(vs&&fs);ctx->bind_vs_state(ctx,vs);ctx->bind_fs_state(ctx,fs);
 ctx->set_viewport_states(ctx,0,1,&view);assert(!remesa_context_error(ctx));
 struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1};struct pipe_draw_start_count_bias draw={.count=3};
 const float clip[3][4]={{-.5,-.5,0,1},{.5,-.5,0,1},{-.5,.5,0,1}};
 float bad[3][4];upload(ctx,vb,clip);
 /* Foreign handle and deleted handle must invalidate, never silently retain VS. */
 void *foreign=vertex_shader(other);assert(foreign);ctx->bind_vs_state(ctx,foreign);assert(remesa_context_error(ctx));ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws);
 ctx->bind_vs_state(ctx,vs);void *stale=vertex_shader(ctx);assert(stale);ctx->delete_vs_state(ctx,stale);ctx->bind_vs_state(ctx,stale);assert(remesa_context_error(ctx));ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws);ctx->bind_vs_state(ctx,vs);
 /* CSO ownership and deletion cannot be masked by a subsequent FS bind. */
 void *foreign_blend=other->create_blend_state(other,&blend);assert(foreign_blend);ctx->bind_blend_state(ctx,foreign_blend);assert(remesa_context_error(ctx));ctx->bind_fs_state(ctx,fs);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));ctx->bind_blend_state(ctx,bs);other->delete_blend_state(other,foreign_blend);
 void *stale_depth=ctx->create_depth_stencil_alpha_state(ctx,&depth);assert(stale_depth);ctx->delete_depth_stencil_alpha_state(ctx,stale_depth);ctx->bind_depth_stencil_alpha_state(ctx,stale_depth);ctx->bind_fs_state(ctx,fs);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));ctx->bind_depth_stencil_alpha_state(ctx,ds);
 void *bound_raster=ctx->create_rasterizer_state(ctx,&raster);assert(bound_raster);ctx->bind_rasterizer_state(ctx,bound_raster);ctx->delete_rasterizer_state(ctx,bound_raster);ctx->bind_fs_state(ctx,fs);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));ctx->bind_rasterizer_state(ctx,rs);
 struct pipe_blend_state unsupported_blend=blend;unsupported_blend.rt[0].blend_enable=true;assert(!ctx->create_blend_state(ctx,&unsupported_blend)&&remesa_context_error(ctx));ctx->bind_fs_state(ctx,fs);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));ctx->bind_blend_state(ctx,bs);
 /* Vertex element handles have context ownership, not arbitrary pointers. */
 void *foreign_ve=other->create_vertex_elements_state(other,1,&e);assert(foreign_ve);
 ctx->bind_vertex_elements_state(ctx,foreign_ve);assert(remesa_context_error(ctx)==EINVAL);
 ctx->bind_fs_state(ctx,fs);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));ctx->bind_vertex_elements_state(ctx,ve);other->delete_vertex_elements_state(other,foreign_ve);
 void *deleted_ve=ctx->create_vertex_elements_state(ctx,1,&e);assert(deleted_ve);ctx->delete_vertex_elements_state(ctx,deleted_ve);
 ctx->delete_vertex_elements_state(ctx,deleted_ve);assert(remesa_context_error(ctx)==EINVAL);ctx->bind_vertex_elements_state(ctx,deleted_ve);assert(remesa_context_error(ctx)==EINVAL);ctx->bind_fs_state(ctx,fs);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));ctx->bind_vertex_elements_state(ctx,ve);
 void *bound_ve=ctx->create_vertex_elements_state(ctx,1,&e);assert(bound_ve);ctx->bind_vertex_elements_state(ctx,bound_ve);ctx->delete_vertex_elements_state(ctx,bound_ve);ctx->bind_fs_state(ctx,fs);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));
 ctx->delete_vertex_elements_state(ctx,ve);ve=ctx->create_vertex_elements_state(ctx,1,&e);assert(ve&&ve!=deleted_ve&&ve!=bound_ve);ctx->bind_vertex_elements_state(ctx,ve);assert(!remesa_context_error(ctx));
 /* Invalid viewport stays invalid even after a successful shader bind. */
 view.scale[0]=8;ctx->set_viewport_states(ctx,0,1,&view);assert(remesa_context_error(ctx));ctx->bind_fs_state(ctx,fs);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));view.scale[0]=16;ctx->set_viewport_states(ctx,0,1,&view);
 /* No unsupported perspective divide, clipping or nonfinite substitution. */
 memcpy(bad,clip,sizeof(bad));bad[0][3]=2;upload(ctx,vb,bad);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));
 memcpy(bad,clip,sizeof(bad));bad[1][0]=1.01f;upload(ctx,vb,bad);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));
 memcpy(bad,clip,sizeof(bad));bad[1][2]=NAN;upload(ctx,vb,bad);ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!rec.draws&&remesa_context_error(ctx));
 /* Failed backend issuance must quarantine that surface, not fake a fence. */
 upload(ctx,vb,clip);struct pipe_resource *failed_target=screen->resource_create(screen,&rt);assert(failed_target);
 fb.cbufs[0].texture=failed_target;ctx->set_framebuffer_state(ctx,&fb);rec.submit_error=EIO;
 ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(remesa_context_error(ctx)==EIO&&!rec.draws);
 struct pipe_fence_handle *failed_fence=NULL;ctx->flush(ctx,&failed_fence,0);assert(!failed_fence);
 struct pipe_box failed_box={.width=32,.height=32,.depth=1};struct pipe_transfer *failed_transfer=NULL;
 assert(!ctx->texture_map(ctx,failed_target,0,PIPE_MAP_READ,&failed_box,&failed_transfer)&&!failed_transfer);
 rec.submit_error=0;fb.cbufs[0].texture=target;ctx->set_framebuffer_state(ctx,&fb);resource_drop(failed_target);
 ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(!remesa_context_error(ctx)&&rec.draws==1);
 const float expected[3][4]={{8,8,.5,1},{24,8,.5,1},{8,24,.5,1}};assert(!memcmp(rec.packet.positions,expected,sizeof(expected)));assert(rec.packet.argb==0xffff00ff);
 struct pipe_fence_handle *f=NULL;ctx->flush(ctx,&f,0);assert(f&&!screen->fence_finish(screen,ctx,f,0));
 struct pipe_box box={.width=32,.height=32,.depth=1};struct pipe_transfer *transfer=NULL;assert(!ctx->texture_map(ctx,target,0,PIPE_MAP_READ,&box,&transfer));
 /* Destroy shader/context while pending: target's writer must survive. */
 ctx->delete_blend_state(ctx,bs);ctx->delete_depth_stencil_alpha_state(ctx,ds);ctx->delete_rasterizer_state(ctx,rs);ctx->delete_vs_state(ctx,vs);ctx->delete_fs_state(ctx,fs);ctx->bind_vertex_elements_state(ctx,NULL);ctx->delete_vertex_elements_state(ctx,ve);ctx->destroy(ctx);
 assert(!screen->fence_finish(screen,NULL,f,0));rec.last->retired=true;assert(!screen->fence_finish(screen,NULL,f,0));rec.last->target->published=true;assert(screen->fence_finish(screen,NULL,f,0));
 screen->fence_reference(screen,&f,NULL);other->delete_vs_state(other,foreign);other->destroy(other);resource_drop(vb);resource_drop(target);assert(rec.allocs==rec.frees);screen->destroy(screen);
 puts("Vertex NIR callback pipeline PASS: rejection, transform, pending teardown; SIMULATED completion, zero SGX");return 0;
}
