/* SPDX-License-Identifier: MIT */
#include "remesa_texture_binding.h"
#include "remesa_resource_ref.h"
#include "pipe/p_context.h"
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define VIEW_LIMIT 1024u
struct texture_view {
   struct pipe_sampler_view base;
   struct pipe_sampler_view descriptor;
   struct pipe_resource *backing;
   struct texture_view *next;
   struct remesa_texture_storage storage[REMESA_TEXTURE_STORAGE_LEVELS];
   unsigned width, height, levels;
   bool destroyed;
};
struct remesa_texture_binding_bank {
   /* Destruction-only Gallium owner. It neither submits nor samples. This
    * proxy outlives the rendering context when a frontend retains a view. */
   struct pipe_context view_owner;
   struct texture_view *views;
   struct texture_view *bound[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];
   bool invalid[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];
   unsigned count, live;
   bool closed, fault;
   remesa_texture_storage_query query;
   void *query_user;
   remesa_texture_binding_alloc alloc;
   remesa_texture_binding_free release;
   void *alloc_user;
};
static void *default_alloc(void *p, size_t bytes)
{ (void)p; return calloc(1, bytes); }
static void default_free(void *p, void *v) { (void)p; free(v); }
static struct texture_view *find(struct remesa_texture_binding_bank *b,
                                 const struct pipe_sampler_view *v)
{
   for (struct texture_view *t=b->views; t; t=t->next)
      if (&t->base==v) return t;
   return NULL;
}
static void free_closed(struct remesa_texture_binding_bank *b)
{
   if (!b->closed || b->live) return;
   struct texture_view *t=b->views;
   while (t) { struct texture_view *next=t->next;
      b->release(b->alloc_user,t); t=next; }
   b->release(b->alloc_user,b);
}
static void owner_destroy(struct pipe_context *p, struct pipe_sampler_view *v)
{
   /* Only the public Gallium reference destructor calls this callback. */
   (void)remesa_texture_view_destroy((struct remesa_texture_binding_bank *)p,v);
}
static void owner_release(struct pipe_context *p, struct pipe_sampler_view *view)
{
   struct remesa_texture_binding_bank *b=(struct remesa_texture_binding_bank *)p;
   struct texture_view *v=find(b,view);
   if (!v || v->destroyed || v->base.reference.count<=0) return;
   if (!--v->base.reference.count)
      (void)remesa_texture_view_destroy(b,&v->base);
}
struct remesa_texture_binding_bank *remesa_texture_binding_create(
   struct pipe_screen *screen, remesa_texture_storage_query query, void *user,
   remesa_texture_binding_alloc alloc, remesa_texture_binding_free release,
   void *alloc_user)
{
   if (!screen || !query || (!!alloc!=!!release)) return NULL;
   if (!alloc) { alloc=default_alloc; release=default_free; }
   struct remesa_texture_binding_bank *b=alloc(alloc_user,sizeof(*b));
   if (!b) return NULL;
   memset(b,0,sizeof(*b)); b->view_owner.screen=screen;
   b->view_owner.sampler_view_destroy=owner_destroy;
   b->view_owner.sampler_view_release=owner_release;
   b->query=query; b->query_user=user; b->alloc=alloc;
   b->release=release; b->alloc_user=alloc_user; return b;
}
static int storage_valid(struct remesa_texture_binding_bank *b,
   struct pipe_resource *r, struct remesa_texture_storage *out)
{
   if (!r || r->screen!=b->view_owner.screen ||
       __atomic_load_n(&r->reference.count,__ATOMIC_RELAXED)<=0)
      return EINVAL;
   if (r->target!=PIPE_TEXTURE_2D || !r->width0 || !r->height0 ||
       r->depth0!=1 || r->array_size!=1 || r->last_level>=REMESA_TEXTURE_STORAGE_LEVELS || r->nr_samples>1 ||
       r->nr_storage_samples>1 || r->flags ||
       (r->format!=PIPE_FORMAT_R8G8B8A8_UNORM &&
        r->format!=PIPE_FORMAT_B8G8R8A8_UNORM)) return ENOTSUP;
   unsigned largest=r->width0>r->height0?r->width0:r->height0, max_level=0;
   while(largest>1){largest>>=1;++max_level;}
   if(r->last_level>max_level)return EINVAL;
   size_t end=0;
   for(unsigned level=0;level<=r->last_level;level++) {
      struct remesa_texture_storage storage={0};
      int error=b->query(b->query_user,r,level,&storage);
      if(error)return error>0?error:EPROTO;
      unsigned width=r->width0>>level,height=r->height0>>level;
      if(!width)width=1;
      if(!height)height=1;
      uint64_t span=(uint64_t)storage.stride*height;
      if(!storage.resource_cookie || storage.levels!=r->last_level+1u ||
         storage.width!=width || storage.height!=height || (storage.stride&3u) ||
         (uint64_t)width*4u>storage.stride || storage.offset!=end ||
         storage.bytes>UINT32_MAX || storage.offset>storage.bytes ||
         span>storage.bytes-storage.offset ||
         (level&&(storage.bytes!=out[0].bytes ||
                  storage.resource_cookie!=out[0].resource_cookie ||
                  storage.content_epoch!=out[0].content_epoch)))return EINVAL;
      out[level]=storage;end=storage.offset+(size_t)span;
   }
   return 0;
}
static bool swizzle_valid(unsigned swizzle) { return swizzle<=PIPE_SWIZZLE_1; }
static int acquire_backing(struct pipe_resource *r)
{
   int count=__atomic_load_n(&r->reference.count,__ATOMIC_RELAXED);
   for (;;) {
      if (count<=0) return ESTALE;
      if (count==INT_MAX) return EOVERFLOW;
      if (__atomic_compare_exchange_n(&r->reference.count,&count,count+1,
          true,__ATOMIC_RELAXED,__ATOMIC_RELAXED)) return 0;
   }
}
int remesa_texture_view_create(struct remesa_texture_binding_bank *b,
   struct pipe_resource *r, const struct pipe_sampler_view *template,
   struct pipe_sampler_view **out)
{
   if (out) *out=NULL;
   if (!b || !template || !out || b->closed) return EINVAL;
   struct remesa_texture_storage storage[REMESA_TEXTURE_STORAGE_LEVELS]={0};
   int error=storage_valid(b,r,storage); if (error) return error;
   if (template->target!=PIPE_TEXTURE_2D || template->format!=r->format ||
       template->astc_decode_format || template->is_tex2d_from_buf ||
       template->u.tex.first_layer || template->u.tex.last_layer ||
       !swizzle_valid(template->swizzle_r) ||
       !swizzle_valid(template->swizzle_g) ||
       !swizzle_valid(template->swizzle_b) ||
       !swizzle_valid(template->swizzle_a)) return ENOTSUP;
   if(template->u.tex.first_level>template->u.tex.last_level ||
      template->u.tex.last_level>r->last_level)return EINVAL;
   /* The existing atomic resource helper acquires exactly one reference.
    * Reject the signed-counter limit before allocation or acquisition. */
   if (__atomic_load_n(&r->reference.count,__ATOMIC_RELAXED)==INT_MAX)
      return EOVERFLOW;
   if (b->count==VIEW_LIMIT) return ENOSPC;
   struct texture_view *v=b->alloc(b->alloc_user,sizeof(*v));
   if (!v) return ENOMEM;
   /* Resource references can be shared across separately serialized contexts.
    * A competing acquisition after the early check must not wrap this count. */
   error=acquire_backing(r);
   if (error) { b->release(b->alloc_user,v); return error; }
   memset(v,0,sizeof(*v)); v->base=*template;
   v->base.reference.count=1; v->base.texture=NULL;
   v->base.context=&b->view_owner;
   v->backing=r;
   v->base.texture=v->backing; v->descriptor=v->base;
   memcpy(v->storage,storage,sizeof(storage));
   v->width=r->width0; v->height=r->height0;v->levels=r->last_level+1u;
   v->next=b->views; b->views=v; ++b->count; ++b->live;
   *out=&v->base; return 0;
}
int remesa_texture_view_destroy(struct remesa_texture_binding_bank *b,
                                struct pipe_sampler_view *view)
{
   if (!b || !view) return EINVAL;
   struct texture_view *v=find(b,view);
   if (!v || v->destroyed) return ESTALE;
   if (v->base.reference.count) return EBUSY;
   v->destroyed=true;
   remesa_resource_reference(&v->backing,NULL); v->base.texture=NULL;
   --b->live; free_closed(b); return 0;
}
static int valid_view(struct remesa_texture_binding_bank *b,
                       struct texture_view *v)
{
   if (!v || v->destroyed || v->base.reference.count<=0) return ESTALE;
   if (v->base.texture!=v->backing ||
       v->base.context!=&b->view_owner ||
       v->base.format!=v->descriptor.format ||
       v->base.target!=v->descriptor.target ||
       v->base.astc_decode_format!=v->descriptor.astc_decode_format ||
       v->base.is_tex2d_from_buf!=v->descriptor.is_tex2d_from_buf ||
       v->base.swizzle_r!=v->descriptor.swizzle_r ||
       v->base.swizzle_g!=v->descriptor.swizzle_g ||
       v->base.swizzle_b!=v->descriptor.swizzle_b ||
       v->base.swizzle_a!=v->descriptor.swizzle_a ||
       memcmp(&v->base.u,&v->descriptor.u,sizeof(v->base.u))) return ESTALE;
   struct remesa_texture_storage storage[REMESA_TEXTURE_STORAGE_LEVELS]={0};
   int error=storage_valid(b,v->backing,storage); if (error) return error;
   if(v->base.texture->last_level+1u!=v->levels ||
       v->base.texture->width0!=v->width ||
       v->base.texture->height0!=v->height ||
       v->base.texture->format!=v->base.format ||
       v->base.context!=&b->view_owner) return ESTALE;
   for(unsigned level=0;level<v->levels;level++) {
      const struct remesa_texture_storage *a=&storage[level],*z=&v->storage[level];
      /* Content changes do not invalidate a view of retained storage. They
       * invalidate immutable prepared descriptors, which capture the epoch. */
      if(a->resource_cookie!=z->resource_cookie || a->bytes!=z->bytes ||
         a->stride!=z->stride || a->offset!=z->offset || a->width!=z->width ||
         a->height!=z->height || a->levels!=z->levels ||
         a->content_epoch<z->content_epoch)return ESTALE;
   }
   /* Refresh only content revisions after every immutable layout check has
    * succeeded. A view remains usable after upload, while a prepared
    * descriptor must observe the new revision at its next revalidation. */
   for(unsigned level=0;level<v->levels;level++)
      v->storage[level].content_epoch=storage[level].content_epoch;
   return 0;
}
static void drop_view(struct remesa_texture_binding_bank *b,struct texture_view *v)
{
   if (v && !--v->base.reference.count)
      (void)remesa_texture_view_destroy(b,&v->base);
}
static bool range(unsigned start,unsigned count)
{ return start<=PIPE_MAX_SHADER_SAMPLER_VIEWS &&
         count<=PIPE_MAX_SHADER_SAMPLER_VIEWS-start; }
int remesa_texture_views_bind(struct remesa_texture_binding_bank *b,
   unsigned stage,unsigned start,unsigned count,unsigned trailing,
   struct pipe_sampler_view *const *views)
{
   if (!b || b->closed) return EINVAL;
   if (stage>=PIPE_SHADER_TYPES || !range(start,count) ||
       trailing>PIPE_MAX_SHADER_SAMPLER_VIEWS-start-count) {
      b->fault=true; return EINVAL;
   }
   int error=0;
   for (unsigned i=0;i<count+trailing;i++) {
      unsigned slot=start+i;
      struct pipe_sampler_view *src=i<count&&views?views[i]:NULL;
      struct texture_view *v=src?find(b,src):NULL;
      int bad=src?valid_view(b,v):0;
      if (!bad && v && v->base.reference.count==INT_MAX) bad=EOVERFLOW;
      if (!bad && v) ++v->base.reference.count;
      struct texture_view *old=b->bound[stage][slot];
      b->bound[stage][slot]=bad?NULL:v; b->invalid[stage][slot]=bad!=0;
      drop_view(b,old); if (bad) error=bad;
   }
   return error;
}
int remesa_texture_draw_admit(struct remesa_texture_binding_bank *b)
{
   if (!b || b->closed || b->fault) return EINVAL;
   bool present=false;
   for (unsigned stage=0;stage<PIPE_SHADER_TYPES;stage++)
      for (unsigned slot=0;slot<PIPE_MAX_SHADER_SAMPLER_VIEWS;slot++) {
         if (b->invalid[stage][slot]) return ESTALE;
         if (b->bound[stage][slot]) {
            int error=valid_view(b,b->bound[stage][slot]);
            if (error) return error;
            present=true;
         }
      }
   return present?ENOTSUP:0;
}
int remesa_texture_view_storage(struct remesa_texture_binding_bank *b,
   const struct pipe_sampler_view *view,struct remesa_texture_storage *out)
{
   if (!b || !view || !out || b->closed) return EINVAL;
   struct texture_view *v=find(b,view);
   int error=valid_view(b,v); if (error) return error;
   *out=v->storage[v->descriptor.u.tex.first_level]; return 0;
}
int remesa_texture_view_level_storage(struct remesa_texture_binding_bank *b,
   const struct pipe_sampler_view *view,unsigned level,struct remesa_texture_storage *out)
{
   if(!b || !view || !out || b->closed)return EINVAL;
   struct texture_view *v=find(b,view);
   int error=valid_view(b,v);if(error)return error;
   if(level<v->descriptor.u.tex.first_level || level>v->descriptor.u.tex.last_level)return EINVAL;
   *out=v->storage[level];return 0;
}
void remesa_texture_binding_close(struct remesa_texture_binding_bank *b)
{
   if (!b || b->closed) return;
   /* Keep closed=false while dropping bindings so last-reference callbacks
    * cannot free the bank partway through this loop. */
   for (unsigned stage=0;stage<PIPE_SHADER_TYPES;stage++)
      for (unsigned slot=0;slot<PIPE_MAX_SHADER_SAMPLER_VIEWS;slot++) {
         struct texture_view *old=b->bound[stage][slot];
         b->bound[stage][slot]=NULL; drop_view(b,old);
      }
   b->closed=true;
   /* Do not query an already destroyed rendering context during late drops. */
   b->query=NULL; b->query_user=NULL; free_closed(b);
}
