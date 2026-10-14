/* SPDX-License-Identifier: MIT; offline actual Mesa NIR fixture, zero GPU. */
#ifndef REMESA_UNIFORM_NIR_FIXTURE_H
#define REMESA_UNIFORM_NIR_FIXTURE_H
#include "nir.h"
#include "util/ralloc.h"
struct fixture {nir_shader *s;nir_block *block;nir_load_const_instr *index,*offset;nir_intrinsic_instr *load,*store;};
static nir_load_const_instr *imm(struct fixture *f,uint32_t bits)
{
 nir_load_const_instr *c=rzalloc_size(f->s,sizeof(*c)+sizeof(nir_const_value));
 c->instr.type=nir_instr_type_load_const;c->instr.block=f->block;c->def.parent_instr=&c->instr;c->def.num_components=1;c->def.bit_size=32;c->value[0].u32=bits;
 exec_list_push_tail(&f->block->instr_list,&c->instr.node);return c;
}
static struct fixture make(uint32_t offset)
{
 struct fixture f={0};f.s=rzalloc(NULL,nir_shader);exec_list_make_empty(&f.s->variables);exec_list_make_empty(&f.s->functions);
 f.s->info.stage=MESA_SHADER_FRAGMENT;f.s->info.io_lowered=true;f.s->info.num_ubos=1;
 nir_function *fn=rzalloc(f.s,nir_function);fn->shader=f.s;fn->is_entrypoint=true;exec_list_push_tail(&f.s->functions,&fn->node);
 nir_function_impl *impl=rzalloc(f.s,nir_function_impl);fn->impl=impl;impl->function=fn;impl->structured=true;
 exec_list_make_empty(&impl->body);exec_list_make_empty(&impl->locals);impl->end_block=rzalloc(f.s,nir_block);exec_list_make_empty(&impl->end_block->instr_list);
 f.block=rzalloc(f.s,nir_block);f.block->cf_node.type=nir_cf_node_block;f.block->cf_node.parent=&impl->cf_node;exec_list_make_empty(&f.block->instr_list);exec_list_push_tail(&impl->body,&f.block->cf_node.node);
 f.index=imm(&f,0);f.offset=imm(&f,offset);
 f.load=rzalloc_size(f.s,sizeof(*f.load)+2*sizeof(nir_src));f.load->instr.type=nir_instr_type_intrinsic;f.load->instr.block=f.block;f.load->intrinsic=nir_intrinsic_load_ubo;f.load->num_components=4;f.load->def.parent_instr=&f.load->instr;f.load->def.bit_size=32;f.load->def.num_components=4;f.load->src[0].ssa=&f.index->def;f.load->src[1].ssa=&f.offset->def;
 nir_intrinsic_set_align_mul(f.load,16);nir_intrinsic_set_align_offset(f.load,0);nir_intrinsic_set_range_base(f.load,offset);nir_intrinsic_set_range(f.load,16);exec_list_push_tail(&f.block->instr_list,&f.load->instr.node);
 f.store=rzalloc_size(f.s,sizeof(*f.store)+2*sizeof(nir_src));f.store->instr.type=nir_instr_type_intrinsic;f.store->instr.block=f.block;f.store->intrinsic=nir_intrinsic_store_output;f.store->num_components=4;f.store->src[0].ssa=&f.load->def;f.store->src[1].ssa=&f.index->def;
 nir_intrinsic_set_write_mask(f.store,15);nir_intrinsic_set_src_type(f.store,nir_type_float32);nir_intrinsic_set_io_semantics(f.store,(nir_io_semantics){.location=FRAG_RESULT_DATA0,.num_slots=1});exec_list_push_tail(&f.block->instr_list,&f.store->instr.node);
 return f;
}
#endif
