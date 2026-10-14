/* SPDX-License-Identifier: MIT */
#ifndef REMESA_NIR_UNIFORM_H
#define REMESA_NIR_UNIFORM_H
#include <stddef.h>
#include <stdint.h>
struct nir_shader;
#define REMESA_UNIFORM_BYTES_MAX 4096u
/* Immutable semantic description. CPU specialization selects an already
 * qualified constant-color profile; this does not encode a GPU UBO instruction. */
struct remesa_uniform_color {
 uint32_t slot;
 uint32_t byte_offset;
 uint32_t byte_size;
};
int remesa_nir_uniform_fragment(const struct nir_shader *,struct remesa_uniform_color *);
/* The caller owns and synchronizes this CPU buffer; no resource mapping occurs
 * here. On error neither admission nor resolution mutates its output. */
int remesa_uniform_color_resolve(const struct remesa_uniform_color *,const void *,size_t,uint32_t *argb);
#endif
