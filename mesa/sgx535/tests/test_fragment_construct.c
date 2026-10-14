/* SPDX-License-Identifier: MIT. Actual Mesa NIR structures; no GPU. */
#include "remesa_nir.h"
#include "nir_fixture.h"
#include <errno.h>
#include <stdio.h>
#ifdef NDEBUG
#error Admission tests require assertions
#endif
static nir_shader *construct(unsigned copies)
{
 nir_shader *s=fixture(0xffff00ff);
 nir_function *fn=exec_node_data(nir_function,exec_list_get_head(&s->functions),node);
 nir_block *block=exec_node_data(nir_block,exec_list_get_head(&fn->impl->body),cf_node.node);
 nir_load_const_instr *old=exec_node_data(nir_load_const_instr,exec_list_get_head(&block->instr_list),instr.node);
 nir_intrinsic_instr *store=exec_node_data(nir_intrinsic_instr,exec_list_get_tail(&block->instr_list),instr.node);
 nir_def *parts[4];
 for(unsigned i=0;i<4;i++) {
  nir_load_const_instr *scalar=rzalloc_size(s,sizeof(*scalar)+sizeof(nir_const_value));
  scalar->instr.type=nir_instr_type_load_const;scalar->instr.block=block;
  scalar->def.parent_instr=&scalar->instr;scalar->def.bit_size=32;scalar->def.num_components=1;
  scalar->value[0].u32=old->value[i].u32;parts[i]=&scalar->def;
  exec_node_insert_node_before(&old->instr.node,&scalar->instr.node);
 }
 exec_node_remove(&old->instr.node);
 nir_alu_instr *vec=rzalloc_size(s,sizeof(*vec)+4*sizeof(nir_alu_src));
 vec->instr.type=nir_instr_type_alu;vec->instr.block=block;vec->op=nir_op_vec4;
 vec->def.parent_instr=&vec->instr;vec->def.bit_size=32;vec->def.num_components=4;
 for(unsigned i=0;i<4;i++)vec->src[i].src.ssa=parts[i];
 exec_node_insert_node_before(&store->instr.node,&vec->instr.node);
 nir_def *value=&vec->def;
 for(unsigned i=0;i<copies;i++) {
  nir_alu_instr *mov=rzalloc_size(s,sizeof(*mov)+sizeof(nir_alu_src));
  mov->instr.type=nir_instr_type_alu;mov->instr.block=block;mov->op=nir_op_mov;
  mov->def.parent_instr=&mov->instr;mov->def.bit_size=32;mov->def.num_components=4;mov->src[0].src.ssa=value;
  for(unsigned j=0;j<4;j++)mov->src[0].swizzle[j]=j;
  exec_node_insert_node_before(&store->instr.node,&mov->instr.node);value=&mov->def;
 }
 store->src[0].ssa=value;return s;
}
static nir_alu_instr *vector(nir_shader *s)
{
 nir_function *fn=exec_node_data(nir_function,exec_list_get_head(&s->functions),node);
 nir_block *b=exec_node_data(nir_block,exec_list_get_head(&fn->impl->body),cf_node.node);
 nir_foreach_instr(instr,b)if(instr->type==nir_instr_type_alu)return nir_instr_as_alu(instr);
 assert(!"missing vec4");return NULL;
}
static nir_shader *split_export(unsigned mode)
{
 nir_shader *s=fixture(0xff00ff00);
 nir_function *fn=exec_node_data(nir_function,exec_list_get_head(&s->functions),node);
 nir_block *b=exec_node_data(nir_block,exec_list_get_head(&fn->impl->body),cf_node.node);
 nir_load_const_instr *old=exec_node_data(nir_load_const_instr,exec_list_get_head(&b->instr_list),instr.node);
 nir_intrinsic_instr *last=exec_node_data(nir_intrinsic_instr,exec_list_get_tail(&b->instr_list),instr.node);
 nir_def *offset=last->src[1].ssa;
 const unsigned order[4]={2,0,3,1};
 unsigned count=mode==1?3:4;
 for(unsigned n=0;n<count;n++) {
  unsigned c=mode==0?order[n]:n;
  nir_load_const_instr *v=rzalloc_size(s,sizeof(*v)+sizeof(nir_const_value));
  v->instr.type=nir_instr_type_load_const;v->instr.block=b;v->def.parent_instr=&v->instr;
  v->def.bit_size=32;v->def.num_components=1;v->value[0].u32=old->value[c].u32;
  exec_list_push_tail(&b->instr_list,&v->instr.node);
  nir_intrinsic_instr *st=rzalloc_size(s,sizeof(*st)+2*sizeof(nir_src));
  st->instr.type=nir_instr_type_intrinsic;st->instr.block=b;st->intrinsic=nir_intrinsic_store_output;
  st->num_components=1;st->src[0].ssa=&v->def;st->src[1].ssa=offset;
  nir_intrinsic_set_write_mask(st,1);nir_intrinsic_set_component(st,c);
  nir_intrinsic_set_src_type(st,nir_type_float32);
  nir_intrinsic_set_io_semantics(st,(nir_io_semantics){.location=FRAG_RESULT_DATA0,.num_slots=1});
  if(n==3) {
   if(mode==2)nir_intrinsic_set_component(st,2);
   if(mode==3)nir_intrinsic_set_io_semantics(st,(nir_io_semantics){.location=FRAG_RESULT_COLOR,.num_slots=1});
   if(mode==4)nir_intrinsic_set_component(st,4);
   if(mode==5)nir_intrinsic_set_src_type(st,nir_type_int32);
   if(mode==6)nir_intrinsic_set_write_mask(st,2);
   if(mode==7)st->num_components=2;
   if(mode==8)v->def.bit_size=16;
   if(mode==9)v->value[0].u32=UINT32_C(0x3f000000);
   if(mode==10)nir_intrinsic_set_write_mask(st,0);
  }
  exec_list_push_tail(&b->instr_list,&st->instr.node);
 }
 exec_node_remove(&old->instr.node);exec_node_remove(&last->instr.node);
 if(mode==11)nir_instr_as_load_const(offset->parent_instr)->value[0].u32=1;
 return s;
}
int main(void)
{
 unsigned cases=0;
 for(unsigned copies=0;copies<=10;copies++) {
  nir_shader *s=construct(copies);uint32_t out=0;
  assert(!remesa_nir_constant_fragment(s,&out)&&out==0xffff00ff);cases++;ralloc_free(s);
 }
 for(unsigned mode=0;mode<13;mode++) {
  nir_shader *s=construct(mode==9?11:0);nir_alu_instr *vec=vector(s);uint32_t out=0x12345678;
  nir_def *source=vec->src[0].src.ssa;
  if(mode==0)vec->op=nir_op_fadd;
  if(mode==1)vec->def.bit_size=16;
  if(mode==2)vec->def.num_components=3;
  if(mode==3)vec->src[0].swizzle[0]=1;
  if(mode==4)vec->src[0].src.ssa=NULL;
  if(mode==5)vec->src[0].src.ssa=&vec->def;
  if(mode==6)source->bit_size=64;
  if(mode==7)nir_instr_as_load_const(source->parent_instr)->value[0].u32=0x3f000000;
  if(mode==8)vec->exact=true;
  if(mode==10)vec->src[0].src.ssa=vec->src[1].src.ssa; /* dead scalar, wrong color */
  if(mode==11)nir_instr_as_load_const(source->parent_instr)->value[0].u32=0x7fc00000;
  if(mode==12)source->parent_instr=&vec->instr;
  assert(remesa_nir_constant_fragment(s,&out)!=0&&out==0x12345678);cases++;ralloc_free(s);
 }
 for(unsigned mode=0;mode<12;mode++) {
  nir_shader *s=split_export(mode);uint32_t out=0x12345678;
  int error=remesa_nir_constant_fragment(s,&out);
  if(!mode)assert(!error&&out==0xff00ff00);
  else assert(error!=0&&out==0x12345678);
  cases++;ralloc_free(s);
 }
 printf("PASS %u fragment construction/order/bounds cases (NO GPU)\n",cases);
 return 0;
}
