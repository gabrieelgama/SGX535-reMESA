/* SPDX-License-Identifier: MIT */
#ifndef REMESA_PRESENT_H
#define REMESA_PRESENT_H
#include <stddef.h>
struct pipe_context;struct pipe_resource;
/* Fence-dependent BGRA8 snapshot for future CPU-assisted publication.
 * Copies existing Gallium pixels, never rasterizes. Does not present, export,
 * import, or claim scanout compatibility. Destination unchanged on map failure. */
int remesa_present_snapshot(struct pipe_context *,struct pipe_resource *,void *destination,size_t stride,size_t capacity);
#endif
