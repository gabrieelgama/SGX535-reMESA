/* SPDX-License-Identifier: MIT */
#include "remesa_sampler_state.h"
#include "pipe/p_state.h"
#include "compiler/shader_enums.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Keep tombstones until context teardown so a freed/reallocated pointer can
 * never accidentally become a live state. This bounded metadata registry is
 * not a second GPU resource registry. API entry is externally serialized. */
#define STATE_LIMIT 1024u
struct sampler {
   struct sampler *next;
   struct pipe_sampler_state state;
   bool deleted;
};
struct remesa_sampler_bank {
   struct sampler *states;
   unsigned count;
   struct sampler *bound[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];
   bool invalid[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];
   bool views[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];
   bool fault;
   remesa_sampler_alloc alloc;
   remesa_sampler_free release;
   void *opaque;
};
static void *default_alloc(void *p, size_t n) { (void)p; return calloc(1,n); }
static void default_free(void *p, void *v) { (void)p; free(v); }
struct remesa_sampler_bank *remesa_sampler_bank_create(remesa_sampler_alloc a,
                                                       remesa_sampler_free f,
                                                       void *p)
{
   if (!!a != !!f) return NULL;
   if (!a) { a=default_alloc; f=default_free; }
   struct remesa_sampler_bank *b=a(p,sizeof(*b));
   if (!b) return NULL;
   memset(b,0,sizeof(*b)); b->alloc=a; b->release=f; b->opaque=p;
   return b;
}
void remesa_sampler_bank_destroy(struct remesa_sampler_bank *b)
{
   if (!b) return;
   struct sampler *s=b->states;
   while (s) { struct sampler *next=s->next; b->release(b->opaque,s); s=next; }
   b->release(b->opaque,b);
}
static struct sampler *lookup(struct remesa_sampler_bank *b, const void *v)
{
   for (struct sampler *s=b->states;s;s=s->next) if (s==v) return s;
   return NULL;
}
int remesa_sampler_state_validate(const struct pipe_sampler_state *s)
{
   if (!s) return EINVAL;
   /* Strict nearest/clamp metadata subset; it is not admitted for GPU use. */
   if (s->wrap_s!=PIPE_TEX_WRAP_CLAMP_TO_EDGE ||
       s->wrap_t!=PIPE_TEX_WRAP_CLAMP_TO_EDGE ||
       s->wrap_r!=PIPE_TEX_WRAP_CLAMP_TO_EDGE ||
       s->min_img_filter!=PIPE_TEX_FILTER_NEAREST ||
       s->mag_img_filter!=PIPE_TEX_FILTER_NEAREST ||
       s->min_mip_filter!=PIPE_TEX_MIPFILTER_NONE ||
       s->compare_mode || s->unnormalized_coords ||
       s->max_anisotropy>1 || s->seamless_cube_map ||
       s->border_color_is_integer || s->reduction_mode!=PIPE_TEX_REDUCTION_WEIGHTED_AVERAGE ||
       s->pad || s->lod_bias!=0.0f || s->min_lod!=0.0f ||
       !isfinite(s->max_lod) || s->max_lod<0.0f ||
       s->border_color_format!=PIPE_FORMAT_NONE) return ENOTSUP;
   /* Mesa's GL sampler defaults retain LEQUAL even when comparison is off,
    * and max_lod=1000 even after selecting non-mipmapped NEAREST. Neither
    * affects this subset: comparison is disabled and both image filters
    * select the view's first level. Preserve the supplied state without
    * inventing hardware fields or admitting sampling. */
   for (unsigned i=0;i<4;i++) if (s->border_color.ui[i]) return ENOTSUP;
   return 0;
}
int remesa_sampler_create(struct remesa_sampler_bank *b,
                          const struct pipe_sampler_state *state, void **out)
{
   if (out) *out=NULL;
   if (!b || !out) return EINVAL;
   int error=remesa_sampler_state_validate(state); if (error) return error;
   if (b->count==STATE_LIMIT) return ENOSPC;
   struct sampler *s=b->alloc(b->opaque,sizeof(*s)); if (!s) return ENOMEM;
   memset(s,0,sizeof(*s)); s->state=*state;
   s->next=b->states; b->states=s; b->count++; *out=s; return 0;
}
int remesa_sampler_delete(struct remesa_sampler_bank *b, void *v)
{
   if (!b || !v) return EINVAL;
   struct sampler *s=lookup(b,v);
   if (!s || s->deleted) return ESTALE;
   s->deleted=true; return 0;
}
static bool range(unsigned start,unsigned count,unsigned max)
{ return start<=max && count<=max-start; }
int remesa_sampler_bind(struct remesa_sampler_bank *b, unsigned stage,
                        unsigned start,unsigned count,void *const *states)
{
   if (!b) return EINVAL;
   if (stage>=PIPE_SHADER_TYPES || !range(start,count,PIPE_MAX_SAMPLERS)) {
      b->fault=true; return EINVAL;
   }
   int error=0;
   for (unsigned i=0;i<count;i++) {
      void *v=states ? states[i] : NULL;
      struct sampler *s=v ? lookup(b,v) : NULL;
      bool bad=v && (!s || s->deleted);
      b->bound[stage][start+i]=bad ? NULL : s;
      b->invalid[stage][start+i]=bad;
      if (bad) error=ESTALE;
   }
   return error;
}
int remesa_sampler_views(struct remesa_sampler_bank *b,unsigned stage,
                         unsigned start,unsigned count,unsigned trailing,
                         struct pipe_sampler_view *const *views)
{
   if (!b) return EINVAL;
   if (stage>=PIPE_SHADER_TYPES || !range(start,count,PIPE_MAX_SHADER_SAMPLER_VIEWS) ||
       trailing>PIPE_MAX_SHADER_SAMPLER_VIEWS-start-count) {
      b->fault=true; return EINVAL;
   }
   int error=0;
   for (unsigned i=0;i<count;i++) {
      bool present=views && views[i];
      b->views[stage][start+i]=present;
      if (present) error=ENOTSUP;
   }
   for (unsigned i=0;i<trailing;i++) b->views[stage][start+count+i]=false;
   return error;
}
int remesa_sampler_draw_admit(const struct remesa_sampler_bank *b)
{
   if (!b || b->fault) return EINVAL;
   for (unsigned stage=0;stage<PIPE_SHADER_TYPES;stage++) {
      for (unsigned i=0;i<PIPE_MAX_SAMPLERS;i++) {
         if (b->invalid[stage][i] ||
             (b->bound[stage][i] && b->bound[stage][i]->deleted)) return ESTALE;
         if (b->bound[stage][i]) return ENOTSUP;
      }
      for (unsigned i=0;i<PIPE_MAX_SHADER_SAMPLER_VIEWS;i++)
         if (b->views[stage][i]) return ENOTSUP;
   }
   return 0;
}
