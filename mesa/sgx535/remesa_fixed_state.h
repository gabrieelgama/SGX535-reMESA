/* SPDX-License-Identifier: MIT */
#ifndef REMESA_FIXED_STATE_H
#define REMESA_FIXED_STATE_H
#include <stdint.h>
#include <stddef.h>
struct pipe_viewport_state;
struct pipe_scissor_state;
struct pipe_blend_state;
struct pipe_depth_stencil_alpha_state;
struct pipe_rasterizer_state;
union pipe_color_union;
/* Pure compatibility admission for the unchanged restricted triangle profile.
 * No state bytes, ISA encoding or general OpenGL support are generated.
 * Return 0, EINVAL for NULL, or ENOTSUP for unsupported state. */
int remesa_validate_blend(const struct pipe_blend_state *);
int remesa_validate_dsa(const struct pipe_depth_stencil_alpha_state *);
int remesa_validate_rasterizer(const struct pipe_rasterizer_state *);
/* CPU geometry preparation for constant fragment output. This performs no
 * rasterization, interpolation of varyings, command encoding or GPU admission.
 * Six homogeneous clip planes can turn a triangle into at most nine vertices.
 * Perspective division is host arithmetic, not evidence of a GPU shader.
 * Output is unchanged on failure. The caller must independently admit the
 * resulting viewport, edge rules, attachment and commands for its backend. */
#define REMESA_CLIPPED_MAX_VERTICES 9u
struct remesa_clipped_polygon {
   unsigned count;
   float window[REMESA_CLIPPED_MAX_VERTICES][4];
};
int remesa_prepare_constant_triangle(const float position[3][4],
                                    const struct pipe_viewport_state *,
                                    uint32_t width, uint32_t height,
                                    struct remesa_clipped_polygon *);
/* Intersect a well-formed Gallium scissor with attachment bounds. Empty
 * intersections are valid. This does not establish SGX scissor support. */
int remesa_clip_scissor(const struct pipe_scissor_state *, uint32_t width,
                        uint32_t height, struct pipe_scissor_state *);
/* Checked CPU-transfer clear preparation for RGBA8/BGRA8 storage. This does
 * not map, synchronize or write a resource and is not a GPU clear. The caller
 * must authenticate the writer fence and hold the mapping lease before using
 * the returned rows. Row padding is excluded. Output is unchanged on failure. */
struct remesa_color_clear {
   size_t offset, row_bytes, stride;
   uint32_t rows;
   unsigned char pixel[4];
};
int remesa_prepare_color_clear(unsigned format, uint32_t attachment_width,
                               uint32_t attachment_height, uint32_t stride,
                               size_t backing_bytes,
                               const union pipe_color_union *,
                               uint32_t x, uint32_t y,
                               uint32_t width, uint32_t height,
                               struct remesa_color_clear *);
#endif
