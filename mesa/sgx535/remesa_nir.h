/* SPDX-License-Identifier: MIT */
#ifndef REMESA_NIR_H
#define REMESA_NIR_H
#include <stdint.h>
struct nir_shader;
/* Pure admission. No NIR mutation, GPU program generation or default color.
 * Accepts bounded float32 constants, exact copies and scalar vec4 construction.
 * Only opaque observed RGB endpoint colors; general UNORM/alpha is unknown.
 * Constant evaluation is CPU-side admission, not native shader ALU execution. */
int remesa_nir_constant_fragment(const struct nir_shader *,uint32_t *argb);
int remesa_constant_rgba32(const uint32_t rgba_bits[4],uint32_t *argb);
/* Restricted lowered or variable-deref POSITION = GENERIC0 float32 vec4 description.
 * This recognizes semantics only; no native VS ISA is generated. */
int remesa_nir_position_vertex(const struct nir_shader *,unsigned *input_slot);
#endif
