/* SPDX-License-Identifier: MIT */
#ifndef REMESA_XCB_H
#define REMESA_XCB_H
#include <stdbool.h>
#include <stdint.h>
#include "remesa_present_scale.h"
struct pipe_context;struct pipe_resource;struct xcb_connection_t;
struct remesa_xcb_receipt {
 uint32_t snapshot_bytes;bool image_request_sent,image_acknowledged,gc_released;
 uint8_t x_error;
 uint32_t image_requests,image_bytes;
 struct remesa_present_scale_receipt scale;
 bool fullscreen_request_sent,fullscreen_request_acknowledged;
};
/* CPU-assisted publication into a caller-owned window, never the root window.
 * Caller owns a dedicated, externally serialized XCB connection and window.
 * The resource is retained until checked acknowledgement/GC cleanup completes.
 * The context and connection must outlive this synchronous call.
 * Does not own scanout, change desktop state, create a compositor, or retry.
 * An acknowledged PutImage proves server acceptance, not physical visibility.
 * Direct and scaled images are split into checked row tiles bounded by the
 * server's base request limit; height must fit signed X11 destination Y.
 * A failed/uncertain GC cleanup requires the caller to close its dedicated
 * connection to release server resources. No internal resend or disconnect.
 * Call only with separate authorization when a real display is involved. */
int remesa_xcb_present(struct pipe_context *,struct pipe_resource *,struct xcb_connection_t *,uint32_t window,struct remesa_xcb_receipt *);
/* Opt-in CPU scaling into an exactly matching resized drawable. Sends bounded
 * row tiles using the base X request limit (height <= INT16_MAX for signed
 * tile coordinates), no retries or synchronization with
 * scanout. A resize racing geometry validation may clip server-side output. */
int remesa_xcb_present_scaled(struct pipe_context *,struct pipe_resource *,struct xcb_connection_t *,
 uint32_t window,uint16_t width,uint16_t height,enum remesa_present_scale_mode,struct remesa_xcb_receipt *);
/* EWMH fullscreen request for a caller-owned window. Acknowledgement means
 * server acceptance only; the window manager may ignore or reject the request.
 * The caller observes ConfigureNotify before choosing publication dimensions. */
int remesa_xcb_request_fullscreen(struct xcb_connection_t *,uint32_t window,bool enabled,struct remesa_xcb_receipt *);
#endif
