/* SPDX-License-Identifier: MIT. Public Mesa NIR builder; no GPU. */
#include "remesa_nir.h"
#include "nir_builder.h"
#include "util/ralloc.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#ifdef NDEBUG
#error Admission tests require assertions
#endif
int main(void)
{
 static const nir_shader_compiler_options options={0};unsigned cases=0;
 glsl_type_singleton_init_or_ref();
 for(unsigned lowered=0;lowered<2;lowered++)for(unsigned color=0;color<3;color++)for(unsigned copies=0;copies<3;copies++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,&options,"constant constructor");
  nir_def *value=nir_vec4(&b,nir_imm_float(&b,color==1?0:1),nir_imm_float(&b,color==0?0:1),
                         nir_imm_float(&b,color==0?1:0),nir_imm_float(&b,1));
  for(unsigned i=0;i<copies;i++)value=nir_mov(&b,value);
  if(lowered) {
   b.shader->info.io_lowered=true;
   nir_store_output(&b,value,nir_imm_int(&b,0),.write_mask=15,.src_type=nir_type_float32,
                    .io_semantics={.location=FRAG_RESULT_DATA0,.num_slots=1});
  } else {
   nir_variable *output=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"color");
   output->data.location=FRAG_RESULT_DATA0;nir_store_deref(&b,nir_build_deref_var(&b,output),value,15);
  }
  uint32_t out=0;
  assert(!remesa_nir_constant_fragment(b.shader,&out));
  assert(out==(color==0?0xffff00ff:color==1?0xff00ff00:0xffffff00));cases++;
  b.shader->info.fs.uses_discard=true;out=0x12345678;
  assert(remesa_nir_constant_fragment(b.shader,&out)==ENOTSUP&&out==0x12345678);cases++;
  ralloc_free(b.shader);
 }
 for(unsigned mode=0;mode<11;mode++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,&options,"decorated output reject");
  nir_variable *v=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"color");
  v->data.location=FRAG_RESULT_DATA0;
  nir_store_deref(&b,nir_build_deref_var(&b,v),nir_imm_vec4(&b,1,0,1,1),15);
  if(mode==0)v->data.driver_location=1;
  if(mode==1)v->data.read_only=true;
  if(mode==2)v->data.explicit_xfb_buffer=true;
  if(mode==3)v->data.explicit_xfb_stride=true;
  if(mode==4)v->data.explicit_offset=true;
  if(mode==5)v->data.matrix_layout=GLSL_MATRIX_LAYOUT_ROW_MAJOR;
  if(mode==6)v->data.from_named_ifc_block=true;
  if(mode==7)v->data.from_ssbo_unsized_array=true;
  if(mode==8)v->data.offset=4;
  if(mode==9)v->data.ray_query=true;
  if(mode==10)v->data.precision=GLSL_PRECISION_MEDIUM;
  uint32_t out=0x12345678;
  assert(remesa_nir_constant_fragment(b.shader,&out)==ENOTSUP&&out==0x12345678);cases++;
  ralloc_free(b.shader);
 }
 /* Public builder emits actual swizzled MOV and vec2/vec3/vec4 instructions.
  * Recognition folds channel routing to the same admitted constant color;
  * this is not hardware ALU or register-encoding evidence. */
 for(unsigned lowered=0;lowered<2;lowered++)for(unsigned mode=0;mode<4;mode++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,&options,"constant channel routing");
  nir_def *base=nir_imm_vec4(&b,0,1,0,1),*value;
  const unsigned channels[4]={1,0,3,3};
  if(!mode) {
   const unsigned magenta[4]={1,0,1,3};value=nir_swizzle(&b,base,magenta,4);
  } else {
   nir_def *route;
   if(mode==1)route=nir_swizzle(&b,base,channels,2);
   else if(mode==2)route=nir_vec2(&b,nir_channel(&b,base,1),nir_channel(&b,base,0));
   else route=nir_vec3(&b,nir_channel(&b,base,1),nir_channel(&b,base,0),nir_channel(&b,base,3));
   nir_def *red=nir_channel(&b,route,0),*green=nir_channel(&b,route,1);
   nir_def *alpha=nir_channel(&b,mode==3?route:base,mode==3?2:3);
   value=nir_vec4(&b,red,green,red,alpha);
  }
  if(lowered) {
   b.shader->info.io_lowered=true;
   nir_store_output(&b,value,nir_imm_int(&b,0),.write_mask=15,.src_type=nir_type_float32,
                    .io_semantics={.location=FRAG_RESULT_DATA0,.num_slots=1});
  } else {
   nir_variable *v=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"color");
   v->data.location=FRAG_RESULT_DATA0;nir_store_deref(&b,nir_build_deref_var(&b,v),value,15);
  }
  uint32_t out=123;assert(!remesa_nir_constant_fragment(b.shader,&out)&&out==0xffff00ff);cases++;
  b.shader->info.fs.uses_discard=true;out=123;
  assert(remesa_nir_constant_fragment(b.shader,&out)==ENOTSUP&&out==123);cases++;
  ralloc_free(b.shader);
 }
 /* Exercise Mesa's real IO scalarizer, which interleaves channel MOVs and
  * stores. The complete color must survive without native arithmetic. */
 for(unsigned color=0;color<3;color++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,&options,"scalarized color output");
  nir_def *value=nir_imm_vec4(&b,color==1?0:1,color==0?0:1,color==0?1:0,1);
  b.shader->info.io_lowered=true;
  nir_store_output(&b,value,nir_imm_int(&b,0),.write_mask=15,.src_type=nir_type_float32,
                   .io_semantics={.location=FRAG_RESULT_DATA0,.num_slots=1});
  assert(nir_lower_io_to_scalar(b.shader,nir_var_shader_out,NULL,NULL));
  uint32_t out=123;
  assert(!remesa_nir_constant_fragment(b.shader,&out));
  assert(out==(color==0?0xffff00ff:color==1?0xff00ff00:0xffffff00));cases++;
  ralloc_free(b.shader);
 }
 for(unsigned lowered=0;lowered<2;lowered++)for(unsigned mode=0;mode<6;mode++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,&options,"partitioned color output");
  nir_variable *v=NULL;nir_deref_instr *d=NULL;
  if(!lowered) {
   v=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"color");
   v->data.location=FRAG_RESULT_DATA0;d=nir_build_deref_var(&b,v);
  } else b.shader->info.io_lowered=true;
  for(unsigned half=0;half<(mode==1?1u:2u);half++) {
   unsigned component=(mode==2||mode==5)?0:2*half;
   if(lowered) {
    nir_def *value=nir_imm_vec2(&b,0,1);
    nir_store_output(&b,value,nir_imm_int(&b,mode==3&&half?1:0),.component=component,
     .write_mask=mode==5?1:3,.src_type=nir_type_float32,
     .io_semantics={.location=mode==4&&half?FRAG_RESULT_COLOR:FRAG_RESULT_DATA0,.num_slots=1});
   } else {
    nir_def *value=nir_imm_vec4(&b,0,1,0,mode==3&&half?0:1);
    if(mode==4&&half) {
     nir_variable *other=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"other");
     other->data.location=FRAG_RESULT_COLOR;d=nir_build_deref_var(&b,other);
    }
    nir_store_deref(&b,d,value,mode==5?1u:(3u<<component));
   }
  }
  uint32_t out=123;int error=remesa_nir_constant_fragment(b.shader,&out);
  if(!mode)assert(!error&&out==0xff00ff00);
  else assert(error!=0&&out==123);
  cases++;ralloc_free(b.shader);
 }
 glsl_type_singleton_decref();printf("PASS %u actual Mesa builder constructor cases (NO GPU)\n",cases);return 0;
}
