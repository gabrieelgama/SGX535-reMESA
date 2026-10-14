/* SPDX-License-Identifier: MIT */
#ifndef REMESA_NIR_H
#define REMESA_NIR_H
#include <stdint.h>
struct nir_shader;
/* Pure admission. No NIR mutation, GPU program generation or default color.
 * Accepts bounded float32 constants, checked channel routing, vector construction
 * and up to four disjoint stores covering one complete RGBA output. Lowered
 * scalar component stores and variable-deref write masks normalize to the same
 * constant profile; partial, overlapping or mixed-output writes are rejected.
 * Only opaque observed RGB endpoint colors; general UNORM/alpha is unknown.
 * Constant evaluation is CPU-side admission, not native shader ALU execution. */
int remesa_nir_constant_fragment(const struct nir_shader *,uint32_t *argb);
int remesa_constant_rgba32(const uint32_t rgba_bits[4],uint32_t *argb);
/* Restricted lowered or variable-deref POSITION = GENERIC0 float32 vec4 description.
 * Bounded bit-preserving channel copies, vec2/vec3 grouping and identity vec4 reconstruction are
 * admitted in either IO representation. This recognizes semantics only; no
 * native VS ISA is generated. */
int remesa_nir_position_vertex(const struct nir_shader *,unsigned *input_slot);
#endif
