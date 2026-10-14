/* SPDX-License-Identifier: MIT */
#ifndef REMESA_PRESENT_SCALE_H
#define REMESA_PRESENT_SCALE_H
#include <stddef.h>
#include <stdint.h>
struct pipe_context;
struct pipe_resource;
enum remesa_present_scale_mode { REMESA_PRESENT_STRETCH, REMESA_PRESENT_FIT };
struct remesa_present_scale_receipt {
 uint32_t source_width,source_height,width,height;
 uint32_t content_x,content_y,content_width,content_height;
 size_t source_bytes,cpu_output_bytes;
};
/* CPU nearest-neighbor publication preparation for a resized window or a
 * fullscreen-sized drawable. FIT preserves aspect ratio with opaque black
 * bars; STRETCH fills the drawable. This never changes the window/WM, draws
 * geometry, submits GPU work, or claims direct scanout/tear-free presentation.
 * Destination remains unchanged on failure. Caller owns and serializes the
 * live context/resource and destination; their storage must not overlap.
 * Readback polls the existing resource publication fence, with no retries. */
int remesa_present_scaled_snapshot(struct pipe_context *,struct pipe_resource *,
 void *destination,size_t stride,size_t capacity,uint32_t width,uint32_t height,
 enum remesa_present_scale_mode,struct remesa_present_scale_receipt *);
#endif
