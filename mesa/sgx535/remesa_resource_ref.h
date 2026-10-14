/* SPDX-License-Identifier: MIT */
#ifndef REMESA_RESOURCE_REF_H
#define REMESA_RESOURCE_REF_H
#include "pipe/p_state.h"
#include "pipe/p_screen.h"
/* Resource references can cross serialized contexts; use atomic count updates. */
static inline void remesa_resource_reference(struct pipe_resource **dst,struct pipe_resource *src)
{
 struct pipe_resource *old=*dst;
 if(src) __atomic_add_fetch(&src->reference.count,1,__ATOMIC_RELAXED);
 *dst=src;
 if(old && !__atomic_sub_fetch(&old->reference.count,1,__ATOMIC_ACQ_REL))
  old->screen->resource_destroy(old->screen,old);
}
#endif
