/* SPDX-License-Identifier: MIT */
#ifndef REMESA_TEXTURE_H
#define REMESA_TEXTURE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Offline resource contract.  This module owns ordinary backing storage and
 * proves transfer/lifetime semantics; it does not encode or submit SGX state.
 * Calls are externally serialized; callbacks must not reenter this object.
 * Wait success is supplied by the caller, never manufactured by this module. */
enum remesa_texture_format {
   REMESA_TEXTURE_RGBA8 = 1,
   REMESA_TEXTURE_BGRA8 = 2
};

struct remesa_texture_desc {
   unsigned width, height;
   enum remesa_texture_format format;
};

struct remesa_texture;
typedef int (*remesa_texture_wait_fn)(void *user, uint64_t serial);

struct remesa_texture_sampler {
   bool min_nearest, mag_nearest;
   bool wrap_s_clamp, wrap_t_clamp;
   bool mipmaps;
};

struct remesa_texture_pixel { uint8_t r, g, b, a; };

int remesa_texture_validate_desc(const struct remesa_texture_desc *);
struct remesa_texture *remesa_texture_create(const struct remesa_texture_desc *);
int remesa_texture_retain(struct remesa_texture *);
/* EBUSY leaves the final caller-owned reference intact while a writer may
 * access backing.  The caller must keep the object and complete teardown. */
int remesa_texture_release(struct remesa_texture *);

/* A writer fence remains attached until the wait callback reports success.
 * EBUSY/EAGAIN/EINTR preserve pending state; other errors quarantine contents.
 * Callback/user ownership must outlive writer retirement. */
int remesa_texture_attach_writer(struct remesa_texture *, uint64_t serial,
                                 remesa_texture_wait_fn, void *user);
int remesa_texture_wait(struct remesa_texture *);
/* Terminal wait failures quarantine contents permanently.  This teardown-only
 * callback must attest that the exact writer no longer accesses backing; it
 * does not authenticate rendering or make contents readable again. */
int remesa_texture_retire_quarantine(struct remesa_texture *, uint64_t serial,
                                     remesa_texture_wait_fn retirement, void *user);
bool remesa_texture_is_quarantined(const struct remesa_texture *);

int remesa_texture_upload(struct remesa_texture *, unsigned x, unsigned y,
                          unsigned width, unsigned height,
                          const void *src, size_t src_stride, size_t src_capacity);
int remesa_texture_readback(struct remesa_texture *, unsigned x, unsigned y,
                            unsigned width, unsigned height,
                            void *dst, size_t dst_stride, size_t dst_capacity);
int remesa_texture_sample_nearest(struct remesa_texture *,
                                  const struct remesa_texture_sampler *,
                                  float s, float t,
                                  struct remesa_texture_pixel *out);
int remesa_texture_validate_sampler(const struct remesa_texture_sampler *);

unsigned remesa_texture_refs(const struct remesa_texture *);
bool remesa_texture_has_writer(const struct remesa_texture *);
uint64_t remesa_texture_generation(const struct remesa_texture *);
#ifdef REMESA_TEXTURE_TEST
/* Fault injection is compiled only by the isolated host-contract test suite. */
int remesa_texture_test_counters(struct remesa_texture *, unsigned refs,
                                 uint64_t generation);
#endif

#endif
