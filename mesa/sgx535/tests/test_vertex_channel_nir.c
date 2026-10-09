/* SPDX-License-Identifier: MIT; actual Mesa NIR structs, no shader execution. */
#include "remesa_nir.h"
#include "vertex_nir_fixture.h"
#include <errno.h>
#include <stdio.h>
static nir_alu_instr *copy(nir_shader *s,nir_block *block,nir_instr *before,
                           nir_def *source,unsigned components,unsigned first)
{
 nir_alu_instr *a=rzalloc_size(s,sizeof(*a)+sizeof(nir_alu_src));
 a->instr.type=nir_instr_type_alu;a->instr.block=block;a->op=nir_op_mov;
 a->def.parent_instr=&a->instr;a->def.num_components=components;a->def.bit_size=32;
 a->src[0].src.ssa=source;
 for(unsigned i=0;i<components;i++)a->src[0].swizzle[i]=first+i;
 exec_node_insert_node_before(&before->node,&a->instr.node);return a;
}
static nir_alu_instr *assemble(nir_shader *s,nir_block *block,nir_instr *before,nir_def *channels[4])
{
 nir_alu_instr *a=rzalloc_size(s,sizeof(*a)+4*sizeof(nir_alu_src));
 a->instr.type=nir_instr_type_alu;a->instr.block=block;a->op=nir_op_vec4;
 a->def.parent_instr=&a->instr;a->def.num_components=4;a->def.bit_size=32;
 for(unsigned i=0;i<4;i++)a->src[i].src.ssa=channels[i];
 exec_node_insert_node_before(&before->node,&a->instr.node);return a;
}
int main(void)
{
 unsigned cases=0;
 for(unsigned mode=0;mode<16;mode++) {
  nir_shader *s=vertex_fixture();unsigned slot=99;
  nir_function *fn=exec_node_data(nir_function,exec_list_get_head(&s->functions),node);
  nir_block *b=exec_node_data(nir_block,exec_list_get_head(&fn->impl->body),cf_node.node);
  nir_instr *zero=exec_node_data(nir_instr,exec_list_get_head(&b->instr_list),node);
  nir_intrinsic_instr *load=exec_node_data(nir_intrinsic_instr,zero->node.next,instr.node);
  nir_intrinsic_instr *store=exec_node_data(nir_intrinsic_instr,exec_list_get_tail(&b->instr_list),instr.node);
  nir_def *channels[4];nir_alu_instr *scalar[4];
  for(unsigned i=0;i<4;i++){scalar[i]=copy(s,b,&store->instr,&load->def,1,i);channels[i]=&scalar[i]->def;}
  nir_alu_instr *vec=assemble(s,b,&store->instr,channels);store->src[0].ssa=&vec->def;
  switch(mode) {
  case 1:store->src[0].ssa=&copy(s,b,&store->instr,&vec->def,4,0)->def;break;
  case 2:scalar[0]->src[0].swizzle[0]=1;break;
  case 3:vec->src[2].src.ssa=channels[1];break;
  case 4:scalar[0]->src[0].swizzle[0]=4;break;
  case 5:scalar[1]->src[0].src.ssa=&vec->def;break;
  case 6:scalar[0]->op=nir_op_fneg;break;
  case 7:scalar[0]->exact=true;break;
  case 8:scalar[0]->def.bit_size=16;break;
  case 9:scalar[0]->def.parent_instr=&vec->instr;break;
  case 10:(void)copy(s,b,&store->instr,&load->def,4,0);break;
  case 11:vec->def.num_components=3;break;
  case 12:vec->src[0].swizzle[0]=1;break;
  case 13:for(unsigned i=0;i<11;i++)store->src[0].ssa=&copy(s,b,&store->instr,store->src[0].ssa,4,0)->def;break;
  case 14:for(unsigned i=0;i<10;i++)store->src[0].ssa=&copy(s,b,&store->instr,store->src[0].ssa,4,0)->def;break;
  case 15:scalar[0]->src[0].src.ssa=NULL;break;
  }
  int ret=remesa_nir_position_vertex(s,&slot);
  if(mode==0||mode==1||mode==14)assert(!ret&&slot==0);
  else assert(ret!=0&&slot==99);
  ralloc_free(s);cases++;
 }
 printf("PASS %u bounded identity vertex channel cases; no GPU\n",cases);return 0;
}
