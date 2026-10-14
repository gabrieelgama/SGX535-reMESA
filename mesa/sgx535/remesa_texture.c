/* SPDX-License-Identifier: MIT */
#include "remesa_texture.h"
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

struct remesa_texture {
   struct remesa_texture_desc desc;
   unsigned refs;
   size_t stride, bytes;
   uint8_t *data;
   bool writer_pending;
   int failure;
   uint64_t writer_serial, last_writer_serial, generation;
   remesa_texture_wait_fn wait;
   void *wait_user;
};

static bool valid_format(enum remesa_texture_format f)
{ return f == REMESA_TEXTURE_RGBA8 || f == REMESA_TEXTURE_BGRA8; }

int remesa_texture_validate_desc(const struct remesa_texture_desc *d)
{
   size_t pixels;
   if (!d || !d->width || !d->height || !valid_format(d->format)) return EINVAL;
   if ((size_t)d->width > SIZE_MAX / (size_t)d->height) return EOVERFLOW;
   pixels = (size_t)d->width * d->height;
   return pixels > SIZE_MAX / 4 ? EOVERFLOW : 0;
}

struct remesa_texture *remesa_texture_create(const struct remesa_texture_desc *d)
{
   struct remesa_texture *t;
   int error = remesa_texture_validate_desc(d);
   if (error) return NULL;
   t = calloc(1, sizeof(*t));
   if (!t) return NULL;
   t->desc = *d;
   t->stride = (size_t)d->width * 4;
   t->bytes = t->stride * d->height;
   t->data = calloc(1, t->bytes);
   if (!t->data) { free(t); return NULL; }
   t->refs = 1;
   t->generation = 1;
   return t;
}

int remesa_texture_retain(struct remesa_texture *t)
{
   if (!t || !t->refs) return EINVAL;
   if (t->refs == UINT_MAX) return EOVERFLOW;
   ++t->refs;
   return 0;
}

int remesa_texture_release(struct remesa_texture *t)
{
   if (!t || !t->refs) return EINVAL;
   if (t->refs == 1 && t->writer_pending) return EBUSY;
   if (--t->refs) return 0;
   free(t->data); free(t);
   return 0;
}

int remesa_texture_attach_writer(struct remesa_texture *t, uint64_t serial,
                                 remesa_texture_wait_fn wait, void *user)
{
   if (!t || !wait || !serial || t->writer_pending) return EINVAL;
   if (t->failure) return t->failure;
   if (serial <= t->last_writer_serial) return ESTALE;
   /* Reserve both the attachment and eventual retirement generation. */
   if (t->generation > UINT64_MAX - 2) return EOVERFLOW;
   t->writer_pending = true; t->writer_serial = serial;
   t->last_writer_serial = serial;
   t->wait = wait; t->wait_user = user; ++t->generation;
   return 0;
}

int remesa_texture_wait(struct remesa_texture *t)
{
   int error;
   if (!t) return EINVAL;
   if (t->failure) return t->failure;
   if (!t->writer_pending) return 0;
   error = t->wait(t->wait_user, t->writer_serial);
   if (error) {
      if (error != EBUSY && error != EAGAIN && error != EINTR)
         t->failure = error > 0 ? error : EPROTO;
      return t->failure ? t->failure : error;
   }
   t->writer_pending = false; t->writer_serial = 0;
   t->wait = NULL; t->wait_user = NULL; ++t->generation;
   return 0;
}

int remesa_texture_retire_quarantine(struct remesa_texture *t, uint64_t serial,
                                     remesa_texture_wait_fn retirement, void *user)
{
   int error;
   if (!t || !retirement || !t->failure || !t->writer_pending) return EINVAL;
   if (serial != t->writer_serial) return ESTALE;
   error = retirement(user, serial);
   if (error) return error > 0 ? error : EPROTO;
   t->writer_pending = false;
   t->writer_serial = 0;
   t->wait = NULL;
   t->wait_user = NULL;
   ++t->generation;
   return 0;
}

bool remesa_texture_is_quarantined(const struct remesa_texture *t)
{ return t && t->failure; }

static int box_ok(const struct remesa_texture *t, unsigned x, unsigned y,
                  unsigned width, unsigned height)
{
   if (!t || !width || !height || x > t->desc.width || y > t->desc.height ||
       width > t->desc.width - x || height > t->desc.height - y) return EINVAL;
   return 0;
}

int remesa_texture_upload(struct remesa_texture *t, unsigned x, unsigned y,
                          unsigned width, unsigned height,
                          const void *src, size_t src_stride, size_t src_capacity)
{
   int error = box_ok(t,x,y,width,height);
   if (error || !src) return error ? error : EINVAL;
   if (src_stride < (size_t)width * 4) return EINVAL;
   if (height>1 && src_stride>(SIZE_MAX-(size_t)width*4)/(height-1))return EOVERFLOW;
   if ((size_t)(height-1)*src_stride+(size_t)width*4>src_capacity)return EINVAL;
   if (t->generation == UINT64_MAX) return EOVERFLOW;
   error = remesa_texture_wait(t); if (error) return error;
   if (t->generation == UINT64_MAX) return EOVERFLOW;
   for (unsigned row=0; row<height; ++row)
      memcpy(t->data + (size_t)(y+row)*t->stride + (size_t)x*4,
             (const uint8_t *)src + (size_t)row*src_stride, (size_t)width*4);
   ++t->generation;
   return 0;
}

int remesa_texture_readback(struct remesa_texture *t, unsigned x, unsigned y,
                            unsigned width, unsigned height,
                            void *dst, size_t dst_stride, size_t dst_capacity)
{
   int error = box_ok(t,x,y,width,height);
   if (error || !dst) return error ? error : EINVAL;
   if (dst_stride < (size_t)width * 4) return EINVAL;
   if (height>1 && dst_stride>(SIZE_MAX-(size_t)width*4)/(height-1))return EOVERFLOW;
   if ((size_t)(height-1)*dst_stride+(size_t)width*4>dst_capacity)return EINVAL;
   error = remesa_texture_wait(t); if (error) return error;
   for (unsigned row=0; row<height; ++row)
      memcpy((uint8_t *)dst + (size_t)row*dst_stride,
             t->data + (size_t)(y+row)*t->stride + (size_t)x*4, (size_t)width*4);
   return 0;
}

int remesa_texture_validate_sampler(const struct remesa_texture_sampler *s)
{
   if (!s || !s->min_nearest || !s->mag_nearest || !s->wrap_s_clamp ||
       !s->wrap_t_clamp || s->mipmaps) return ENOTSUP;
   return 0;
}

int remesa_texture_sample_nearest(struct remesa_texture *t,
                                  const struct remesa_texture_sampler *s,
                                  float u, float v,
                                  struct remesa_texture_pixel *out)
{
   unsigned x, y; uint8_t p[4]; int error;
   if (!t || !out || !isfinite(u) || !isfinite(v)) return EINVAL;
   error = remesa_texture_validate_sampler(s); if (error) return error;
   error = remesa_texture_wait(t); if (error) return error;
   if (u < 0) u=0;
   if (u > 1) u=1;
   if (v < 0) v=0;
   if (v > 1) v=1;
   x = u >= 1 ? t->desc.width-1 : (unsigned)floor((double)u * t->desc.width);
   y = v >= 1 ? t->desc.height-1 : (unsigned)floor((double)v * t->desc.height);
   memcpy(p,t->data + (size_t)y*t->stride + (size_t)x*4,4);
   if (t->desc.format == REMESA_TEXTURE_RGBA8)
      out->r=p[0],out->g=p[1],out->b=p[2],out->a=p[3];
   else out->b=p[0],out->g=p[1],out->r=p[2],out->a=p[3];
   return 0;
}

unsigned remesa_texture_refs(const struct remesa_texture *t) { return t ? t->refs : 0; }
bool remesa_texture_has_writer(const struct remesa_texture *t) { return t && t->writer_pending; }
uint64_t remesa_texture_generation(const struct remesa_texture *t) { return t ? t->generation : 0; }
#ifdef REMESA_TEXTURE_TEST
int remesa_texture_test_counters(struct remesa_texture *t, unsigned refs,
                                 uint64_t generation)
{
   if (!t || !refs || !generation || t->writer_pending) return EINVAL;
   t->refs = refs;
   t->generation = generation;
   return 0;
}
#endif
