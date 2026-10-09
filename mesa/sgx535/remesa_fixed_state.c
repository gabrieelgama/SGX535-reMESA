/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <math.h>
#include <string.h>
#include "pipe/p_state.h"
#include "remesa_fixed_state.h"

int
remesa_validate_blend(const struct pipe_blend_state *s)
{
   if (!s) return EINVAL;
   if (s->independent_blend_enable || s->max_rt || s->logicop_enable ||
       s->dither || s->alpha_to_coverage || s->alpha_to_coverage_dither ||
       s->alpha_to_one || s->advanced_blend_func || s->rt[0].blend_enable ||
       s->rt[0].colormask != PIPE_MASK_RGBA)
      return ENOTSUP;
   /* One render target, independent blending disabled. Slots 1..7 are
    * inapplicable. Disabled blend/logic-op operands and coherence have no
    * effect. Field comparisons deliberately ignore C padding. */
   return 0;
}

int
remesa_validate_dsa(const struct pipe_depth_stencil_alpha_state *s)
{
   if (!s) return EINVAL;
   if (s->depth_enabled || s->depth_writemask || s->depth_bounds_test ||
       s->stencil[0].enabled || s->stencil[1].enabled || s->alpha_enabled)
      return ENOTSUP;
   /* No depth/stencil resource is admitted by the caller. Functions, masks,
    * references and bounds of disabled tests cannot change this profile. */
   return 0;
}

int
remesa_validate_rasterizer(const struct pipe_rasterizer_state *s)
{
   if (!s) return EINVAL;
   if (s->flatshade || s->light_twoside || s->clamp_vertex_color ||
       s->clamp_fragment_color || s->cull_face != PIPE_FACE_NONE ||
       s->fill_front != PIPE_POLYGON_MODE_FILL ||
       s->fill_back != PIPE_POLYGON_MODE_FILL || s->offset_tri ||
       s->poly_smooth || s->poly_stipple_enable || s->multisample ||
       s->force_persample_interp || s->conservative_raster_mode ||
       s->rasterizer_discard || s->tile_raster_order_fixed ||
       s->half_pixel_center || s->bottom_edge_rule ||
       s->subpixel_precision_x || s->subpixel_precision_y || s->depth_clamp ||
       s->clip_halfz || s->unclamped_fragment_depth_values ||
       s->clip_plane_enable)
      return ENOTSUP;
   /* The caller admits triangles only, constant fragment output, no culling,
    * and full-target scissoring only. Orientation/provoking vertex and point/
    * line fields are therefore immaterial. Offset operands are immaterial
    * with triangle offset disabled. Tile directions/conservative dilate are
    * immaterial when the corresponding feature is disabled.
    *
    * Depth-clip flags are admitted ONLY because draw preparation rejects all
    * positions outside [-1,1] clip space and requires w=1. This validator does
    * not implement clipping; the parent draw admission must enforce it. No
    * unsupported alternative edge rule is silently translated. */
   return 0;
}

static double clip_distance(const double p[4], unsigned plane)
{
   /* OpenGL canonical homogeneous volume: -w <= x,y,z <= w. */
   return p[3] + ((plane & 1) ? -p[plane / 2] : p[plane / 2]);
}

static int append_clip_vertex(double polygon[REMESA_CLIPPED_MAX_VERTICES][4],
                              unsigned *count, const double point[4])
{
   if (*count && !memcmp(polygon[*count - 1], point, sizeof(polygon[0]))) return 0;
   if (*count == REMESA_CLIPPED_MAX_VERTICES) return EOVERFLOW;
   memcpy(polygon[(*count)++], point, sizeof(polygon[0]));
   return 0;
}

int remesa_prepare_constant_triangle(const float position[3][4],
                                    const struct pipe_viewport_state *viewport,
                                    uint32_t width, uint32_t height,
                                    struct remesa_clipped_polygon *out)
{
   double polygon[2][REMESA_CLIPPED_MAX_VERTICES][4];
   struct remesa_clipped_polygon result = {0};
   unsigned current = 0, count = 3;
   if (!position || !viewport || !out || !width || !height)
      return EINVAL;
   if (viewport->swizzle_x || viewport->swizzle_y || viewport->swizzle_z ||
       viewport->swizzle_w)
      return ENOTSUP;
   for (unsigned axis = 0; axis < 3; ++axis)
      if (!isfinite(viewport->scale[axis]) ||
          !isfinite(viewport->translate[axis])) return EINVAL;
   /* Restrict this preparation contract to viewports inside the attachment.
    * Negative scale supports origin reversal. Scissoring remains separate. */
   for (unsigned axis = 0; axis < 2; ++axis) {
      double low = (double)viewport->translate[axis] - fabs(viewport->scale[axis]);
      double high = (double)viewport->translate[axis] + fabs(viewport->scale[axis]);
      if (low < 0 || high > (axis ? height : width)) return ENOTSUP;
   }
   for (unsigned vertex = 0; vertex < 3; ++vertex)
      for (unsigned axis = 0; axis < 4; ++axis) {
         if (!isfinite(position[vertex][axis])) return EINVAL;
         polygon[0][vertex][axis] = position[vertex][axis];
      }
   for (unsigned plane = 0; plane < 6 && count; ++plane) {
      unsigned next = current ^ 1, produced = 0;
      for (unsigned index = 0; index < count; ++index) {
         const double *a = polygon[current][(index + count - 1) % count];
         const double *b = polygon[current][index];
         double da = clip_distance(a, plane), db = clip_distance(b, plane);
         if ((da < 0) != (db < 0)) {
            double point[4];
            double fraction = da / (da - db);
            for (unsigned axis = 0; axis < 4; ++axis)
               point[axis] = a[axis] + fraction * (b[axis] - a[axis]);
            if (append_clip_vertex(polygon[next], &produced, point)) return EOVERFLOW;
         }
         if (db >= 0) {
            if (append_clip_vertex(polygon[next], &produced, b)) return EOVERFLOW;
         }
      }
      if (produced > 1 && !memcmp(polygon[next][0], polygon[next][produced - 1],
                                  sizeof(polygon[next][0]))) --produced;
      count = produced; current = next;
   }
   if (count < 3) count = 0;
   for (unsigned index = 0; index < count; ++index) {
      const double *p = polygon[current][index];
      /* Zero-w points at the homogeneous origin have no defined projection.
       * Fail closed rather than manufacture screen coordinates. */
      if (p[3] <= 0) return ENOTSUP;
      for (unsigned axis = 0; axis < 3; ++axis) {
         double value = p[axis] / p[3] * viewport->scale[axis] + viewport->translate[axis];
         float converted = (float)value;
         if (!isfinite(converted)) return EOVERFLOW;
         result.window[index][axis] = converted;
      }
      result.window[index][3] = 1;
   }
   result.count = count;
   *out = result;
   return 0;
}

int remesa_clip_scissor(const struct pipe_scissor_state *input,
                        uint32_t width, uint32_t height,
                        struct pipe_scissor_state *out)
{
   struct pipe_scissor_state result;
   if (!input || !out || !width || !height ||
       input->minx > input->maxx || input->miny > input->maxy) return EINVAL;
   result.minx = input->minx < width ? input->minx : width;
   result.maxx = input->maxx < width ? input->maxx : width;
   result.miny = input->miny < height ? input->miny : height;
   result.maxy = input->maxy < height ? input->maxy : height;
   *out = result;
   return 0;
}

int remesa_prepare_color_clear(unsigned format, uint32_t attachment_width,
                               uint32_t attachment_height, uint32_t stride,
                               size_t backing_bytes,
                               const union pipe_color_union *color,
                               uint32_t x, uint32_t y,
                               uint32_t width, uint32_t height,
                               struct remesa_color_clear *out)
{
   struct remesa_color_clear result = {0};
   uint64_t packed, allocation, offset;
   if (!color || !out || !attachment_width || !attachment_height) return EINVAL;
   if (format != PIPE_FORMAT_B8G8R8A8_UNORM &&
       format != PIPE_FORMAT_R8G8B8A8_UNORM) return ENOTSUP;
   packed = (uint64_t)attachment_width * 4;
   if (packed > UINT32_MAX) return EOVERFLOW;
   if (stride < packed || stride % 4) return EINVAL;
   allocation = (uint64_t)stride * attachment_height;
   if (allocation > SIZE_MAX) return EOVERFLOW;
   if (allocation > backing_bytes) return ENOSPC;
   /* Subtraction checks reject wrapping rectangles before offset arithmetic. */
   if (x > attachment_width || y > attachment_height ||
       width > attachment_width - x || height > attachment_height - y)
      return EINVAL;
   for (unsigned channel = 0; channel < 4; ++channel) {
      float value = color->f[channel];
      if (!isfinite(value)) return EINVAL;
      unsigned byte = value <= 0 ? 0 : value >= 1 ? 255 :
                      (unsigned)(value * 255.0f + 0.5f);
      unsigned destination = channel;
      if (format == PIPE_FORMAT_B8G8R8A8_UNORM)
         destination = channel == 0 ? 2 : channel == 2 ? 0 : channel;
      result.pixel[destination] = byte;
   }
   result.stride = stride;
   /* Empty rectangles do not have a meaningful mapping offset, especially at
    * the bottom-right boundary. Do not expose a pointer beyond allocation. */
   if (width && height) {
      offset = (uint64_t)y * stride + (uint64_t)x * 4;
      result.offset = (size_t)offset;
      result.row_bytes = (size_t)width * 4;
      result.rows = height;
   }
   *out = result;
   return 0;
}
