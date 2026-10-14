/* SPDX-License-Identifier: MIT
 * Drive actual Gallium shader callbacks and draw_vbo into a packet recorder.
 * Completion transitions remain explicit simulations from test_driver.c. */
#include "nir_fixture.h"
#include "vertex_nir_fixture.h"
#include "remesa_screen.h"
#include "remesa_device.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
/* Actual NIR constructor form exercises the new recognizer through the
 * normal create/bind/delete lifecycle and draw_vbo packet translation. */
static nir_shader *constructed_fragment(uint32_t color)
{
 nir_shader *shader=fixture(color);
 nir_function *fn=exec_node_data(nir_function,exec_list_get_head(&shader->functions),node);
 nir_block *block=exec_node_data(nir_block,exec_list_get_head(&fn->impl->body),cf_node.node);
 nir_load_const_instr *old=exec_node_data(nir_load_const_instr,exec_list_get_head(&block->instr_list),instr.node);
 nir_intrinsic_instr *store=exec_node_data(nir_intrinsic_instr,exec_list_get_tail(&block->instr_list),instr.node);
 nir_alu_instr *vec=rzalloc_size(shader,sizeof(*vec)+4*sizeof(nir_alu_src));
 vec->instr.type=nir_instr_type_alu;vec->instr.block=block;vec->op=nir_op_vec4;
 vec->def.parent_instr=&vec->instr;vec->def.bit_size=32;vec->def.num_components=4;
 for(unsigned i=0;i<4;i++) {
  nir_load_const_instr *scalar=rzalloc_size(shader,sizeof(*scalar)+sizeof(nir_const_value));
  scalar->instr.type=nir_instr_type_load_const;scalar->instr.block=block;
  scalar->def.parent_instr=&scalar->instr;scalar->def.bit_size=32;scalar->def.num_components=1;
  scalar->value[0].u32=old->value[i].u32;vec->src[i].src.ssa=&scalar->def;
  exec_node_insert_node_before(&old->instr.node,&scalar->instr.node);
 }
 exec_node_remove(&old->instr.node);
 exec_node_insert_node_before(&store->instr.node,&vec->instr.node);
 store->src[0].ssa=&vec->def;return shader;
}
static int bind_nir(struct pipe_context *ctx,uint32_t color)
{
 struct pipe_shader_state vertex={.type=PIPE_SHADER_IR_NIR,.ir.nir=vertex_fixture()};
 void *vs=ctx->create_vs_state(ctx,&vertex);if(!vs)return remesa_context_error(ctx);
 ctx->bind_vs_state(ctx,vs);
 struct pipe_blend_state blend={0};blend.rt[0].colormask=PIPE_MASK_RGBA;
 struct pipe_depth_stencil_alpha_state depth={0};
 struct pipe_rasterizer_state raster={.fill_front=PIPE_POLYGON_MODE_FILL,.fill_back=PIPE_POLYGON_MODE_FILL};
 void *bs=ctx->create_blend_state(ctx,&blend),*ds=ctx->create_depth_stencil_alpha_state(ctx,&depth),
      *rs=ctx->create_rasterizer_state(ctx,&raster);
 if(!bs||!ds||!rs)return remesa_context_error(ctx);
 ctx->bind_blend_state(ctx,bs);ctx->bind_depth_stencil_alpha_state(ctx,ds);ctx->bind_rasterizer_state(ctx,rs);
 struct pipe_viewport_state viewport={.scale={16,16,.5},.translate={16,16,.5}};
 ctx->set_viewport_states(ctx,0,1,&viewport);
 struct pipe_shader_state state={.type=PIPE_SHADER_IR_NIR};state.ir.nir=constructed_fragment(color);
 void *fs=ctx->create_fs_state(ctx,&state);if(!fs)return remesa_context_error(ctx);
 ctx->bind_fs_state(ctx,fs);
 if(remesa_context_error(ctx))return remesa_context_error(ctx);
 /* test_driver.c explicitly exercises diagnostic replacement and submit-error
  * fences. A normal NIR bind requests preservation, which must reject its
  * second REPLACE-only draw before dispatch. Opt into the test's DISCARD here;
  * this is not a state-tracker/OpenGL preservation test. */
 return remesa_context_set_framebuffer_load(ctx,false);
}
#define remesa_bind_diagnostic_pipeline bind_nir
#define REMESA_TEST_CLIP_INPUT
#include "test_driver.c"
