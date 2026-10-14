/* SPDX-License-Identifier: MIT
 * Actual Mesa 25.2 NIR structures, manually constructed offline fixtures.
 * No SGX execution or GPU completion is simulated by this test. */
#include "remesa_nir.h"
#include "remesa_screen.h"
#include "remesa_device.h"
#include "nir.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/ralloc.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
static unsigned destroyed;
static bool fail_next_calloc;
void *__real_calloc(size_t,size_t);
void *__wrap_calloc(size_t count,size_t bytes)
{if(fail_next_calloc){fail_next_calloc=false;return NULL;}return __real_calloc(count,bytes);}
static void destroy_marker(void *p){(void)p;destroyed++;}
static nir_shader *fixture(uint32_t color)
{
 nir_shader *s=rzalloc(NULL,nir_shader);assert(s);ralloc_set_destructor(s,destroy_marker);
 exec_list_make_empty(&s->variables);exec_list_make_empty(&s->functions);
 s->info.stage=MESA_SHADER_FRAGMENT;s->info.io_lowered=true;
 nir_function *fn=rzalloc(s,nir_function);fn->shader=s;fn->is_entrypoint=true;exec_list_push_tail(&s->functions,&fn->node);
 nir_function_impl *impl=rzalloc(s,nir_function_impl);fn->impl=impl;impl->function=fn;impl->structured=true;
 exec_list_make_empty(&impl->body);exec_list_make_empty(&impl->locals);
 impl->end_block=rzalloc(s,nir_block);exec_list_make_empty(&impl->end_block->instr_list);
 nir_block *b=rzalloc(s,nir_block);b->cf_node.type=nir_cf_node_block;b->cf_node.parent=&impl->cf_node;
 exec_list_push_tail(&impl->body,&b->cf_node.node);exec_list_make_empty(&b->instr_list);
 nir_load_const_instr *c=rzalloc_size(s,sizeof(*c)+4*sizeof(nir_const_value));
 c->instr.type=nir_instr_type_load_const;c->instr.block=b;c->def.parent_instr=&c->instr;c->def.bit_size=32;c->def.num_components=4;
 c->value[0].u32=(color&0xff0000)?0x3f800000:0;c->value[1].u32=(color&0xff00)?0x3f800000:0;c->value[2].u32=(color&0xff)?0x3f800000:0;c->value[3].u32=(color&0xff000000)?0x3f800000:0;
 exec_list_push_tail(&b->instr_list,&c->instr.node);
 nir_load_const_instr *z=rzalloc_size(s,sizeof(*z)+sizeof(nir_const_value));z->instr.type=nir_instr_type_load_const;z->instr.block=b;z->def.parent_instr=&z->instr;z->def.bit_size=32;z->def.num_components=1;exec_list_push_tail(&b->instr_list,&z->instr.node);
 nir_intrinsic_instr *st=rzalloc_size(s,sizeof(*st)+2*sizeof(nir_src));st->instr.type=nir_instr_type_intrinsic;st->instr.block=b;st->intrinsic=nir_intrinsic_store_output;st->num_components=4;st->src[0].ssa=&c->def;st->src[1].ssa=&z->def;
 nir_intrinsic_set_write_mask(st,15);nir_intrinsic_set_src_type(st,nir_type_float32);nir_intrinsic_set_io_semantics(st,(nir_io_semantics){.location=FRAG_RESULT_DATA0,.num_slots=1});exec_list_push_tail(&b->instr_list,&st->instr.node);
 return s;
}
static int alloc(void *p,size_t n,void **out){(void)p;(void)n;(void)out;return ENOTSUP;}
static void release(void *p,void *o){(void)p;(void)o;}
static int map(void *p,void *o,bool w,void **out){(void)p;(void)o;(void)w;(void)out;return ENOTSUP;}
static void unmap(void *p,void *o,bool w){(void)p;(void)o;(void)w;}
static int submit(void *p,void *o,const struct remesa_draw_packet *q,void **f){(void)p;(void)o;(void)q;(void)f;assert(!"no draw permitted");return ENOTSUP;}
static int wait(void *p,void *o,uint64_t n,bool v){(void)p;(void)o;(void)n;(void)v;return ENOTSUP;}
int main(void)
{
 unsigned cases=0;uint32_t out=123;
 assert(remesa_nir_constant_fragment(NULL,&out)==EINVAL&&out==123);cases++;
 const uint32_t colors[]={0xffff00ff,0xff00ff00,0xffffff00};
 for(unsigned i=0;i<3;i++){nir_shader *s=fixture(colors[i]);assert(!remesa_nir_constant_fragment(s,&out)&&out==colors[i]);ralloc_free(s);cases++;}
 for(unsigned i=0;i<3;i++){nir_shader *s=fixture(i==0?0x00ff00ff:i==1?0xffffffff:0xff000000);out=123;assert(remesa_nir_constant_fragment(s,&out)==ENOTSUP&&out==123);ralloc_free(s);cases++;}
 for(unsigned i=0;i<6;i++){
  nir_shader *s=fixture(0xffff00ff);nir_function *fn=exec_node_data(nir_function,exec_list_get_head(&s->functions),node);
  nir_block *b=exec_node_data(nir_block,exec_list_get_head(&fn->impl->body),cf_node.node);
  nir_load_const_instr *c=exec_node_data(nir_load_const_instr,exec_list_get_head(&b->instr_list),instr.node);
  nir_intrinsic_instr *st=exec_node_data(nir_intrinsic_instr,exec_list_get_tail(&b->instr_list),instr.node);
  if(i==0)s->info.stage=MESA_SHADER_VERTEX;
  if(i==1)s->info.fs.uses_discard=true;
  if(i==2)c->value[0].u32=0x3f000000;
  if(i==3)nir_intrinsic_set_write_mask(st,7);
  if(i==4)nir_intrinsic_set_io_semantics(st,(nir_io_semantics){.location=FRAG_RESULT_DEPTH,.num_slots=1});
  if(i==5)st->intrinsic=nir_intrinsic_load_input;
  out=123;assert(remesa_nir_constant_fragment(s,&out)!=0&&out==123);ralloc_free(s);cases++;
 }
 static const struct remesa_device_ops ops={.allocate=alloc,.destroy_bo=release,.map=map,.unmap=unmap,.submit=submit,.wait=wait,.destroy_fence=release};
 struct remesa_device dev={.ops=&ops,.offline_test_backend=true};struct pipe_screen *screen=remesa_screen_create(&dev);assert(screen);
 struct pipe_context *ctx=screen->context_create(screen,NULL,0),*other=screen->context_create(screen,NULL,0);assert(ctx&&other);
 struct pipe_viewport_state viewport={.scale={1,1,1}};ctx->set_viewport_states(ctx,0,1,&viewport);assert(!remesa_context_error(ctx));cases++;
 viewport.scale[0]=2;ctx->set_viewport_states(ctx,0,1,&viewport);assert(remesa_context_error(ctx)==ENOTSUP);cases++;
 struct pipe_scissor_state scissor={.maxx=32,.maxy=32};ctx->set_scissor_states(ctx,0,1,&scissor);assert(!remesa_context_error(ctx));cases++;
 scissor.minx=1;ctx->set_scissor_states(ctx,0,1,&scissor);assert(remesa_context_error(ctx)==ENOTSUP);cases++;
 ctx->clear(ctx,0,NULL,NULL,0,0);assert(!remesa_context_error(ctx));cases++;
 struct pipe_shader_state state={.type=PIPE_SHADER_IR_NIR};unsigned before=destroyed;
 state.ir.nir=fixture(0xffff00ff);void *fs=ctx->create_fs_state(ctx,&state);assert(fs&&destroyed==before+1);cases++;
 ctx->bind_fs_state(ctx,fs);assert(!remesa_context_error(ctx));cases++;
 other->bind_fs_state(other,fs);assert(remesa_context_error(other)==EINVAL);cases++;
 ctx->delete_fs_state(ctx,fs);ctx->bind_fs_state(ctx,fs);assert(remesa_context_error(ctx)==EINVAL);cases++;
 state.ir.nir=fixture(0x00ff00ff);before=destroyed;assert(!ctx->create_fs_state(ctx,&state)&&destroyed==before+1);cases++;
 state.ir.nir=fixture(0xffff00ff);before=destroyed;assert(!ctx->create_vs_state(ctx,&state)&&destroyed==before+1);cases++;
 state.ir.nir=fixture(0xffff00ff);before=destroyed;fail_next_calloc=true;
 assert(!ctx->create_fs_state(ctx,&state)&&remesa_context_error(ctx)==ENOMEM&&destroyed==before+1);cases++;
 state.ir.nir=fixture(0xff00ff00);assert(ctx->create_fs_state(ctx,&state));ctx->destroy(ctx);other->destroy(other);screen->destroy(screen);cases++;
 printf("PASS %u NIR/conversion/callback cases (offline structures; no GPU)\n",cases);return 0;
}
