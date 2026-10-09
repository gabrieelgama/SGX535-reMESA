/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "pipe/p_state.h"
#include "remesa_fixed_state.h"

int main(void)
{
   unsigned cases = 0;
#define CHECK(condition) do { ++cases; assert(condition); } while (0)
   struct pipe_blend_state b = {0};
   struct pipe_depth_stencil_alpha_state d = {0};
   struct pipe_rasterizer_state r = {0};
   b.rt[0].colormask = PIPE_MASK_RGBA;
   CHECK(remesa_validate_blend(&b) == 0);
   CHECK(remesa_validate_dsa(&d) == 0);
   CHECK(remesa_validate_rasterizer(&r) == 0);
   CHECK(remesa_validate_blend(NULL) == EINVAL);
   CHECK(remesa_validate_dsa(NULL) == EINVAL);
   CHECK(remesa_validate_rasterizer(NULL) == EINVAL);
#define BAD_B(field, value) do { struct pipe_blend_state t=b; t.field=value; CHECK(remesa_validate_blend(&t)==ENOTSUP); } while (0)
   BAD_B(rt[0].colormask, PIPE_MASK_R);
   BAD_B(rt[0].blend_enable, 1);
   BAD_B(logicop_enable, 1);
   BAD_B(dither, 1);
   BAD_B(alpha_to_coverage, 1);
   BAD_B(alpha_to_coverage_dither, 1);
   BAD_B(alpha_to_one, 1);
   BAD_B(advanced_blend_func, 1);
   BAD_B(independent_blend_enable, 1);
   BAD_B(max_rt, 1);
   /* Disabled blend/logic-op operands and unused RT slots cannot affect RT0. */
   b.rt[0].rgb_func=7; b.rt[0].alpha_src_factor=31;
   b.logicop_func=15; b.rt[7].blend_enable=1;
   CHECK(remesa_validate_blend(&b)==0);
#define BAD_D(field, value) do { struct pipe_depth_stencil_alpha_state t=d; t.field=value; CHECK(remesa_validate_dsa(&t)==ENOTSUP); } while (0)
   BAD_D(depth_enabled, 1);
   BAD_D(depth_writemask, 1);
   BAD_D(stencil[0].enabled, 1);
   BAD_D(stencil[1].enabled, 1);
   BAD_D(alpha_enabled, 1);
   BAD_D(depth_bounds_test, 1);
   d.depth_func=7; d.alpha_ref_value=0.7f; d.stencil[0].writemask=255;
   CHECK(remesa_validate_dsa(&d)==0);
#define BAD_R(field, value) do { struct pipe_rasterizer_state t=r; t.field=value; CHECK(remesa_validate_rasterizer(&t)==ENOTSUP); } while (0)
   BAD_R(cull_face, PIPE_FACE_BACK);
   BAD_R(fill_front, PIPE_POLYGON_MODE_LINE);
   BAD_R(fill_back, PIPE_POLYGON_MODE_POINT);
   BAD_R(offset_tri, 1);
   BAD_R(poly_smooth, 1);
   BAD_R(poly_stipple_enable, 1);
   BAD_R(multisample, 1);
   BAD_R(force_persample_interp, 1);
   BAD_R(conservative_raster_mode, 1);
   BAD_R(rasterizer_discard, 1);
   BAD_R(tile_raster_order_fixed, 1);
   BAD_R(clip_plane_enable, 1);
   BAD_R(unclamped_fragment_depth_values, 1);
   BAD_R(depth_clamp, 1);
   BAD_R(clip_halfz, 1);
   BAD_R(half_pixel_center, 1);
   BAD_R(bottom_edge_rule, 1);
   BAD_R(subpixel_precision_x, 1);
   BAD_R(subpixel_precision_y, 1);
   BAD_R(flatshade, 1);
   BAD_R(light_twoside, 1);
   BAD_R(clamp_vertex_color, 1);
   BAD_R(clamp_fragment_color, 1);
   /* Triangle-only draws have no point/line primitives or varying color. */
   r.depth_clip_near=1; r.depth_clip_far=1;
   r.front_ccw=1; r.flatshade_first=1; r.scissor=1;
   r.point_size=19; r.line_width=3; r.line_stipple_enable=1;
   r.offset_units=99; r.offset_scale=99;
   CHECK(remesa_validate_rasterizer(&r)==0);
   printf("fixed state: %u admission cases PASS; no GPU execution\n", cases);
   return 0;
}
