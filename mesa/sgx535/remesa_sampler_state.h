/* SPDX-License-Identifier: MIT */
#ifndef REMESA_SAMPLER_STATE_H
#define REMESA_SAMPLER_STATE_H
#include <stddef.h>
struct pipe_sampler_state;
struct pipe_sampler_view;
struct remesa_sampler_bank;
typedef void *(*remesa_sampler_alloc)(void *, size_t);
typedef void (*remesa_sampler_free)(void *, void *);
/* Host state ownership only: no sampler program, texture instruction or GPU
 * capability is produced. Any nonempty binding prevents draw admission. */
struct remesa_sampler_bank *remesa_sampler_bank_create(remesa_sampler_alloc,
                                                      remesa_sampler_free,
                                                      void *);
void remesa_sampler_bank_destroy(struct remesa_sampler_bank *);
/* Shared semantic validation for state creation and host descriptors. The
 * nearest/non-mipmapped subset accepts Mesa's inactive compare function and
 * finite nonnegative max_lod; neither selects another filter or mip level. */
int remesa_sampler_state_validate(const struct pipe_sampler_state *);
int remesa_sampler_create(struct remesa_sampler_bank *,
                          const struct pipe_sampler_state *, void **);
int remesa_sampler_delete(struct remesa_sampler_bank *, void *);
int remesa_sampler_bind(struct remesa_sampler_bank *, unsigned stage,
                        unsigned start, unsigned count, void *const *);
/* Mesa 25.2 has no take_ownership flag. Nonnull views are rejected without
 * dereferencing, retaining or consuming their references. Their slots remain
 * blocked until explicitly unbound. NULL arrays mean unbind the given range. */
int remesa_sampler_views(struct remesa_sampler_bank *, unsigned stage,
                         unsigned start, unsigned count, unsigned trailing,
                         struct pipe_sampler_view *const *);
int remesa_sampler_draw_admit(const struct remesa_sampler_bank *);
#endif
