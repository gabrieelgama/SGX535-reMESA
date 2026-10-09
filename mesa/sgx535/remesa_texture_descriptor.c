/* SPDX-License-Identifier: MIT */
#include "remesa_texture_descriptor.h"
#include "pipe/p_state.h"
#include <errno.h>
#include <string.h>

static int sampler_valid(const struct pipe_sampler_state *s)
{
   if (!s) return EINVAL;
   if (s->wrap_s!=PIPE_TEX_WRAP_CLAMP_TO_EDGE ||
       s->wrap_t!=PIPE_TEX_WRAP_CLAMP_TO_EDGE ||
       s->wrap_r!=PIPE_TEX_WRAP_CLAMP_TO_EDGE ||
       s->min_img_filter!=PIPE_TEX_FILTER_NEAREST ||
       s->mag_img_filter!=PIPE_TEX_FILTER_NEAREST ||
       s->min_mip_filter!=PIPE_TEX_MIPFILTER_NONE ||
       s->compare_mode || s->unnormalized_coords || s->max_anisotropy>1 ||
       s->seamless_cube_map || s->reduction_mode || s->pad ||
       s->lod_bias!=0.0f || s->min_lod!=0.0f || s->max_lod!=0.0f)
      return ENOTSUP;
   return 0;
}
static int mapping_valid(const struct remesa_texture_mapping *m,uint64_t owner,
                         uint64_t cookie,uint64_t bytes,uint64_t span)
{
   if (!owner || !m->owner || !m->resource_cookie || !m->gem_handle ||
       !m->allocation_generation || !m->mapping_generation) return EINVAL;
   if (m->owner!=owner || m->resource_cookie!=cookie) return ESTALE;
   if (m->quarantined) return EIO;
   if (!m->mapped || !m->pfn_verified || !m->pins || m->cpu_leases) return EBUSY;
   if (!m->gpu_va || (m->gpu_va&3u) || (m->resource_offset&3u) ||
       !m->mapped_bytes || m->mapped_bytes>UINT64_MAX-m->gpu_va ||
       m->resource_offset>m->mapped_bytes || bytes>m->mapped_bytes-m->resource_offset ||
       span>bytes) return EINVAL;
   if (!m->cpu_write_epoch || m->device_visible_epoch!=m->cpu_write_epoch)
      return EAGAIN;
   if (!!m->writer_operation!=!!m->writer_generation ||
       !!m->completed_operation!=!!m->completed_generation) return EPROTO;
   if (m->writer_operation!=m->completed_operation ||
       m->writer_generation!=m->completed_generation) return EAGAIN;
   return 0;
}
/* Independent host tags: X,Y,Z,W,ZERO,ONE are 0..5. Mesa's licensed
 * semantic enums are mapped explicitly, never emitted as a hardware field. */
static uint8_t host_swizzle(unsigned swizzle)
{
   switch (swizzle) {
   case PIPE_SWIZZLE_X: return 0;
   case PIPE_SWIZZLE_Y: return 1;
   case PIPE_SWIZZLE_Z: return 2;
   case PIPE_SWIZZLE_W: return 3;
   case PIPE_SWIZZLE_0: return 4;
   case PIPE_SWIZZLE_1: return 5;
   default: return 255;
   }
}
int remesa_texture_descriptor_prepare(struct remesa_texture_binding_bank *bank,
   const struct pipe_sampler_view *view,const struct pipe_sampler_state *sampler,
   uint64_t owner,remesa_texture_mapping_query query,void *priv,
   struct remesa_texture_descriptor *out)
{
   if (!bank || !view || !query || !out || !owner) return EINVAL;
   struct remesa_texture_storage storage;
   /* Membership precedes every view dereference. */
   int error=remesa_texture_view_storage(bank,view,&storage);
   if (error) return error;
   error=sampler_valid(sampler); if (error) return error;
   struct remesa_texture_descriptor d={0};
   d.width=view->texture->width0; d.height=view->texture->height0;
   d.stride=storage.stride; d.image_bytes=(uint64_t)d.stride*d.height;
   d.format=view->format==PIPE_FORMAT_R8G8B8A8_UNORM?1u:2u;
   d.swizzle[0]=host_swizzle(view->swizzle_r); d.swizzle[1]=host_swizzle(view->swizzle_g);
   d.swizzle[2]=host_swizzle(view->swizzle_b); d.swizzle[3]=host_swizzle(view->swizzle_a);
   error=query(priv,view->texture,&d.mapping);
   if (error) return error>0?error:EPROTO;
   error=mapping_valid(&d.mapping,owner,storage.resource_cookie,storage.bytes,
                       d.image_bytes);
   if (error) return error;
   *out=d; return 0;
}
static int descriptor_valid(const struct remesa_texture_descriptor *d)
{
   if (!d || !d->width || !d->height || !d->stride || (d->stride&3u) ||
       (uint64_t)d->width*4u>d->stride ||
       (uint64_t)d->stride*d->height!=d->image_bytes ||
       (d->format!=1 && d->format!=2)) return EINVAL;
   for (unsigned i=0;i<4;i++) if (d->swizzle[i]>5) return EINVAL;
   return mapping_valid(&d->mapping,d->mapping.owner,d->mapping.resource_cookie,
                        d->image_bytes,d->image_bytes);
}
static void put32(uint8_t *p,uint32_t v)
{ for (unsigned i=0;i<4;i++) p[i]=(uint8_t)(v>>(i*8)); }
static void put64(uint8_t *p,uint64_t v)
{ for (unsigned i=0;i<8;i++) p[i]=(uint8_t)(v>>(i*8)); }
int remesa_texture_descriptor_serialize(const struct remesa_texture_descriptor *d,
                                       void *out,size_t bytes)
{
   if (!out || bytes!=REMESA_TEXTURE_HOST_METADATA_BYTES) return EINVAL;
   int error=descriptor_valid(d); if (error) return error;
   uint8_t p[REMESA_TEXTURE_HOST_METADATA_BYTES]={0};
   /* Original host-format tags and positions, never SGX bitfields. */
   memcpy(p,"RMTD",4); put32(p+4,1);
   const uint64_t fields[]={d->mapping.owner,d->mapping.resource_cookie,
      d->mapping.allocation_generation,d->mapping.mapping_generation,
      d->mapping.gpu_va,d->mapping.mapped_bytes,d->mapping.resource_offset,
      d->image_bytes,d->mapping.cpu_write_epoch,d->mapping.device_visible_epoch,
      d->mapping.writer_operation,d->mapping.writer_generation};
   for (unsigned i=0;i<12;i++) put64(p+8+i*8,fields[i]);
   put32(p+104,d->width); put32(p+108,d->height); put32(p+112,d->stride);
   put32(p+116,d->format); put32(p+120,d->mapping.gem_handle);
   memcpy(p+124,d->swizzle,4); memcpy(out,p,sizeof(p)); return 0;
}
int remesa_texture_descriptor_revalidate(const struct remesa_texture_descriptor *d,
   struct remesa_texture_binding_bank *bank,const struct pipe_sampler_view *view,
   const struct pipe_sampler_state *sampler,uint64_t owner,
   remesa_texture_mapping_query query,void *priv)
{
   struct remesa_texture_descriptor current;
   uint8_t original[REMESA_TEXTURE_HOST_METADATA_BYTES],fresh[sizeof(original)];
   int error=remesa_texture_descriptor_serialize(d,original,sizeof(original));
   if (error) return error;
   error=remesa_texture_descriptor_prepare(bank,view,sampler,owner,query,priv,&current);
   if (error) return error;
   error=remesa_texture_descriptor_serialize(&current,fresh,sizeof(fresh));
   if (error) return error;
   return memcmp(original,fresh,sizeof(original))?ESTALE:0;
}
int remesa_texture_descriptor_emit_sgx(const struct remesa_texture_descriptor *d,
                                      void *out,size_t bytes,size_t *written)
{ (void)d; (void)out; (void)bytes; (void)written; return ENOTSUP; }
