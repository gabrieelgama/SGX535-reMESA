/* SPDX-License-Identifier: MIT; actual Mesa builder, CPU profile specialization. */
#include "remesa_nir_uniform.h"
#include "nir_builder.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef NDEBUG
#error Actual Mesa admission tests require assertions enabled
#endif
#define CHECK(x) do {if(!(x)){fprintf(stderr,"check failed: %d\n",__LINE__);abort();}} while(0)
int main(void)
{
 static const nir_shader_compiler_options options={0};unsigned cases=0;glsl_type_singleton_init_or_ref();
 for(unsigned mode=0;mode<14;mode++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,&options,"reMESA static UBO color profile");
  b.shader->info.io_lowered=true;b.shader->info.num_ubos=1;
  nir_def *zero=nir_imm_int(&b,0),*offset=nir_imm_int(&b,16);
  nir_def *color=nir_load_ubo(&b,4,32,zero,offset,.align_mul=16,.align_offset=0,.range_base=16,.range=16);
  if(mode==1)color=nir_fadd(&b,color,nir_imm_vec4(&b,0,0,0,0));
  if(mode==2)nir_push_if(&b,nir_imm_true(&b));
  nir_store_output(&b,color,zero,.write_mask=15,.src_type=nir_type_float32,.io_semantics={.location=FRAG_RESULT_DATA0,.num_slots=1});
  if(mode==2)nir_pop_if(&b,NULL);
  if(mode==3)b.shader->info.fs.uses_discard=true;
  if(mode==4)b.shader->info.num_textures=1;
  if(mode==5)b.shader->info.num_ubos=0;
  if(mode==6)b.shader->info.num_images=1;
  if(mode==7)b.shader->info.fs.early_fragment_tests=true;
  if(mode==8)b.shader->info.system_values_read[0]=1;
  if(mode==9)b.shader->info.outputs_written=BITFIELD64_BIT(FRAG_RESULT_DATA0);
  if(mode==10)b.shader->info.outputs_written=BITFIELD64_BIT(FRAG_RESULT_COLOR);
  if(mode==11)b.shader->info.fs.pixel_center_integer=true;
  if(mode==12)b.shader->info.fs.origin_upper_left=true;
  if(mode==13)b.shader->info.fs.color_is_dual_source=true;
  struct remesa_uniform_color desc={7,99,11};int err=remesa_nir_uniform_fragment(b.shader,&desc);
  CHECK(mode==0||mode==9 ? (!err&&desc.slot==0&&desc.byte_offset==16&&desc.byte_size==16) : (err&&desc.slot==7&&desc.byte_offset==99&&desc.byte_size==11));
  ralloc_free(b.shader);cases++;
 }
 glsl_type_singleton_decref();printf("PASS %u actual Mesa NIR builder UBO profile cases; no GPU\n",cases);return 0;
}
