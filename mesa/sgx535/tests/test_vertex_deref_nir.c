/* SPDX-License-Identifier: MIT; manual actual-Mesa NIR structs, no GPU. */
#include "remesa_nir.h"
#include "vertex_nir_fixture.h"
#include <errno.h>
#include <stdio.h>
static const struct glsl_type vec4={.base_type=GLSL_TYPE_FLOAT,.vector_elements=4,.matrix_columns=1};
static const struct glsl_type vec3={.base_type=GLSL_TYPE_FLOAT,.vector_elements=3,.matrix_columns=1};
struct inputs {nir_shader *s;nir_variable *in,*out;nir_deref_instr *id,*od;nir_intrinsic_instr *load,*store;nir_function *fn;nir_block *block;};
static struct inputs make(void)
{
 struct inputs f={0};f.s=vertex_fixture();f.s->info.io_lowered=false;
 f.fn=exec_node_data(nir_function,exec_list_get_head(&f.s->functions),node);
 f.block=exec_node_data(nir_block,exec_list_get_head(&f.fn->impl->body),cf_node.node);exec_list_make_empty(&f.block->instr_list);
 f.in=rzalloc(f.s,nir_variable);f.out=rzalloc(f.s,nir_variable);f.in->type=&vec4;f.out->type=&vec4;
 f.in->data.mode=nir_var_shader_in;f.in->data.location=VERT_ATTRIB_GENERIC0;f.in->data.read_only=true;
 f.out->data.mode=nir_var_shader_out;f.out->data.location=VARYING_SLOT_POS;f.out->data.interpolation=INTERP_MODE_SMOOTH;
 exec_list_push_tail(&f.s->variables,&f.in->node);exec_list_push_tail(&f.s->variables,&f.out->node);
 f.id=rzalloc(f.s,nir_deref_instr);f.od=rzalloc(f.s,nir_deref_instr);
 nir_deref_instr *ds[2]={f.id,f.od};nir_variable *vs[2]={f.in,f.out};
 for(unsigned i=0;i<2;i++){ds[i]->instr.type=nir_instr_type_deref;ds[i]->instr.block=f.block;ds[i]->deref_type=nir_deref_type_var;ds[i]->modes=vs[i]->data.mode;ds[i]->type=vs[i]->type;ds[i]->var=vs[i];ds[i]->def.parent_instr=&ds[i]->instr;ds[i]->def.bit_size=32;ds[i]->def.num_components=1;exec_list_push_tail(&f.block->instr_list,&ds[i]->instr.node);}
 f.load=rzalloc_size(f.s,sizeof(*f.load)+sizeof(nir_src));f.load->instr.type=nir_instr_type_intrinsic;f.load->instr.block=f.block;f.load->intrinsic=nir_intrinsic_load_deref;f.load->src[0].ssa=&f.id->def;f.load->num_components=4;f.load->def.parent_instr=&f.load->instr;f.load->def.bit_size=32;f.load->def.num_components=4;exec_list_push_tail(&f.block->instr_list,&f.load->instr.node);
 f.store=rzalloc_size(f.s,sizeof(*f.store)+2*sizeof(nir_src));f.store->instr.type=nir_instr_type_intrinsic;f.store->instr.block=f.block;f.store->intrinsic=nir_intrinsic_store_deref;f.store->src[0].ssa=&f.od->def;f.store->src[1].ssa=&f.load->def;f.store->num_components=4;nir_intrinsic_set_write_mask(f.store,15);exec_list_push_tail(&f.block->instr_list,&f.store->instr.node);
 return f;
}
int main(void)
{
 unsigned cases=0;
 for(unsigned mode=0;mode<35;mode++) {
  struct inputs f=make();unsigned slot=99;
  switch(mode) {
  case 1:f.in->data.binding=1;break;
  case 2:f.in->data.descriptor_set=1;break;
  case 3:f.out->data.explicit_xfb_buffer=1;break;
  case 4:f.out->data.explicit_xfb_stride=1;break;
  case 5:f.out->data.explicit_offset=1;break;
  case 6:f.in->data.from_named_ifc_block=1;break;
  case 7:f.in->data.matrix_layout=1;break;
  case 8:f.id->def.bit_size=16;break;
  case 9:f.od->def.num_components=2;break;
  case 10:f.in->type=&vec3;break;
  case 11:f.in->data.location_frac=1;break;
  case 12:f.out->data.is_xfb=1;break;
  case 13:nir_intrinsic_set_access(f.load,1);break;
  case 14:nir_intrinsic_set_access(f.store,1);break;
  case 15:nir_intrinsic_set_write_mask(f.store,7);break;
  case 16:f.store->src[1].ssa=&f.id->def;break;
  case 17:f.load->def.bit_size=16;break;
  case 18:f.id->var=f.out;break;
  case 19:f.in->data.driver_location=1;break;
  case 20:f.in->data.location=VERT_ATTRIB_GENERIC1;break;
  case 21:f.out->data.location=VARYING_SLOT_COL0;break;
  case 22:f.s->info.io_lowered=true;break;
  case 23:f.fn->num_params=1;break;
  case 24:f.in->data.precision=GLSL_PRECISION_MEDIUM;break;
  case 25:f.out->data.interpolation=INTERP_MODE_FLAT;break;
  case 26:f.s->info.clip_distance_array_size=1;break;
  case 27:f.s->info.writes_memory=true;break;
  case 28:f.in->data.explicit_location=true;f.out->data.invariant=true;break;
  case 29:f.s->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_GENERIC0);f.s->info.outputs_written=BITFIELD64_BIT(VARYING_SLOT_POS);break;
  case 30:f.in->data.location=VERT_ATTRIB_POS;f.s->info.inputs_read=0;break;
  case 31:f.in->data.location=VERT_ATTRIB_POS;f.s->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_POS);break;
  case 32:f.in->data.location=VERT_ATTRIB_POS;f.s->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_GENERIC0);break;
  case 33:f.s->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_POS);break;
  case 34:f.in->data.location=VERT_ATTRIB_POS;f.s->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_POS)|BITFIELD64_BIT(VERT_ATTRIB_GENERIC0);break;
  }
  int ret=remesa_nir_position_vertex(f.s,&slot);
  if(mode==0||mode==28||mode==29||mode==30||mode==31)assert(ret==0&&slot==0);
  else {if(ret==0)fprintf(stderr,"unexpected accepted mode %u\n",mode);assert(ret!=0&&slot==99);}
  ralloc_free(f.s);cases++;
 }
 printf("PASS %u variable vertex NIR cases; no GPU\n",cases);return 0;
}
