/* SPDX-License-Identifier: MIT; actual Mesa builder, no hardware or pixels. */
#include "remesa_nir.h"
#include "nir_builder.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(condition) do { if(!(condition)) {fprintf(stderr,"check failed mode %u line %d\n",mode,__LINE__);abort();} } while(0)
int main(void)
{
 static const nir_shader_compiler_options options={0};unsigned cases=0;glsl_type_singleton_init_or_ref();
 for(unsigned mode=0;mode<26;mode++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_VERTEX,&options,"reMESA deref position");
  nir_variable *in=nir_variable_create(b.shader,nir_var_shader_in,mode==3?glsl_vec_type(3):glsl_vec4_type(),"position");
  nir_variable *out=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"gl_Position");
  in->data.location=VERT_ATTRIB_GENERIC0;out->data.location=VARYING_SLOT_POS;
  nir_deref_instr *id=nir_build_deref_var(&b,in),*od=nir_build_deref_var(&b,out);
  nir_def *value=nir_load_deref(&b,id);
  if(mode==1)value=nir_fadd(&b,value,nir_imm_vec4(&b,0,0,0,0));
  if(mode==2)value=nir_imm_vec4(&b,0,0,0,1);
  nir_store_deref(&b,od,value,mode==4?7:15);
  if(mode==5)in->data.location=VERT_ATTRIB_GENERIC1;
  if(mode==6)out->data.location=VARYING_SLOT_COL0;
  if(mode==7)in->data.location_frac=1;
  if(mode==8)out->data.patch=true;
  if(mode==9)nir_variable_create(b.shader,nir_var_shader_in,glsl_vec4_type(),"extra");
  if(mode==10)b.shader->info.vs.window_space_position=true;
  if(mode==11)b.shader->info.writes_memory=true;
  if(mode==12)out->data.is_xfb=true;
  if(mode==13)in->data.precision=GLSL_PRECISION_MEDIUM;
  if(mode==14)b.shader->info.stage=MESA_SHADER_FRAGMENT;
  if(mode==15)in->data.driver_location=1;
  if(mode==16)out->data.per_primitive=true;
  if(mode==17){b.shader->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_GENERIC0);b.shader->info.outputs_written=BITFIELD64_BIT(VARYING_SLOT_POS);}
  if(mode==18){in->data.explicit_location=true;out->data.invariant=true;}
  if(mode==19)b.shader->info.io_lowered=true;
  if(mode>=20)in->data.location=VERT_ATTRIB_POS;
  if(mode==21)b.shader->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_POS);
  if(mode==22)b.shader->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_GENERIC0);
  if(mode==23)b.shader->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_POS)|BITFIELD64_BIT(VERT_ATTRIB_GENERIC0);
  if(mode==24)in->data.location=VERT_ATTRIB_COLOR0;
  if(mode==25)out->data.location=VARYING_SLOT_COL0;
  unsigned slot=99;int result=remesa_nir_position_vertex(b.shader,&slot);
  CHECK((mode==0||mode==17||mode==18||mode==20||mode==21)?(!result&&slot==0):(result&&slot==99));
  ralloc_free(b.shader);cases++;
 }
 glsl_type_singleton_decref();printf("PASS %u actual builder deref vertex cases; no GPU\n",cases);return 0;
}
