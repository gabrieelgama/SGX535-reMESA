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
 printf("PASS %u fragment construction/order/bounds cases (NO GPU)\n",cases);
 return 0;
}
