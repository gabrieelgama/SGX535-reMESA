/* SPDX-License-Identifier: MIT */
#ifndef REMESA_TEXTURE_BINDING_H
#define REMESA_TEXTURE_BINDING_H
#include <stddef.h>
#include <stdint.h>
struct pipe_screen;
struct pipe_resource;
struct pipe_sampler_view;
struct remesa_texture_binding_bank;
/* Calls and Gallium's non-atomic sampler-view references are serialized by
 * their frontend owner. This module does not add a second GPU ownership model. */
/* Host layout admission, not SGX texture descriptor bytes. The resource cookie
 * identifies one frontend allocation lifetime, not a kernel generation or a
 * proof of GPU visibility. Query must reject quarantined/foreign resources. */
struct remesa_texture_storage {
   uint64_t resource_cookie;
   size_t bytes;
   uint32_t stride;
};
typedef int (*remesa_texture_storage_query)(void *, struct pipe_resource *,
                                           struct remesa_texture_storage *);
typedef void *(*remesa_texture_binding_alloc)(void *, size_t);
typedef void (*remesa_texture_binding_free)(void *, void *);
struct remesa_texture_binding_bank *remesa_texture_binding_create(
   struct pipe_screen *, remesa_texture_storage_query, void *query_user,
   remesa_texture_binding_alloc, remesa_texture_binding_free, void *alloc_user);
/* Drops every bound reference and closes admission. External Gallium view
 * references remain valid. Their resource and destruction proxy retain the
 * bank until the final reference drops, independently of the real context.
 * Caller must discard its bank pointer after close. */
void remesa_texture_binding_close(struct remesa_texture_binding_bank *);
int remesa_texture_view_create(struct remesa_texture_binding_bank *,
   struct pipe_resource *, const struct pipe_sampler_view *,
   struct pipe_sampler_view **);
/* Called by Gallium's destructor at reference count zero. Pointer membership
 * is checked before dereferencing: foreign/stale pointers are rejected. */
int remesa_texture_view_destroy(struct remesa_texture_binding_bank *,
                                struct pipe_sampler_view *);
int remesa_texture_views_bind(struct remesa_texture_binding_bank *,
   unsigned stage, unsigned start, unsigned count, unsigned trailing,
   struct pipe_sampler_view *const *);
/* Empty bindings only. Successful metadata creation never admits GPU sampling.
 * Revalidates owned backing/cookies before reporting the unsupported draw. */
int remesa_texture_draw_admit(struct remesa_texture_binding_bank *);
int remesa_texture_view_storage(struct remesa_texture_binding_bank *,
   const struct pipe_sampler_view *, struct remesa_texture_storage *);
#endif
