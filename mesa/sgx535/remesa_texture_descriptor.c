/* SPDX-License-Identifier: MIT */
#include "remesa_texture_descriptor.h"
#include "remesa_sampler_state.h"
#include "pipe/p_state.h"
#include <errno.h>
#include <string.h>

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
   /* Existing SGX mappings and the full-width DRM relocation use a 32-bit
    * byte address. Reject truncation before any future descriptor encoder.
    * An exclusive end of 2^32 is valid; its final byte is UINT32_MAX. */
   if (m->gpu_va>UINT32_MAX ||
       m->mapped_bytes>(UINT64_C(1)<<32)-m->gpu_va) return ERANGE;
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
   error=remesa_sampler_state_validate(sampler); if (error) return error;
   struct remesa_texture_descriptor d={0};
   d.width=storage.width; d.height=storage.height;
   d.stride=storage.stride; d.image_bytes=(uint64_t)d.stride*d.height;
   d.resource_bytes=storage.bytes;d.level_offset=storage.offset;
   d.first_level=view->u.tex.first_level;d.last_level=view->u.tex.last_level;
   d.content_epoch=storage.content_epoch;
   d.format=view->format==PIPE_FORMAT_R8G8B8A8_UNORM?1u:2u;
   d.swizzle[0]=host_swizzle(view->swizzle_r); d.swizzle[1]=host_swizzle(view->swizzle_g);
   d.swizzle[2]=host_swizzle(view->swizzle_b); d.swizzle[3]=host_swizzle(view->swizzle_a);
   error=query(priv,view->texture,&d.mapping);
   if (error) return error>0?error:EPROTO;
   error=mapping_valid(&d.mapping,owner,storage.resource_cookie,d.resource_bytes,
                       d.image_bytes);
   if (error) return error;
   *out=d; return 0;
}
static int descriptor_valid(const struct remesa_texture_descriptor *d)
{
   if (!d || !d->width || !d->height || !d->stride || (d->stride&3u) ||
       (uint64_t)d->width*4u>d->stride ||
       (uint64_t)d->stride*d->height!=d->image_bytes ||
       (d->format!=1 && d->format!=2) ||
       d->first_level>d->last_level || d->last_level>=REMESA_TEXTURE_STORAGE_LEVELS ||
       (d->level_offset&3u) || d->level_offset>d->resource_bytes ||
       d->image_bytes>d->resource_bytes-d->level_offset) return EINVAL;
   for (unsigned i=0;i<4;i++) if (d->swizzle[i]>5) return EINVAL;
   return mapping_valid(&d->mapping,d->mapping.owner,d->mapping.resource_cookie,
                        d->resource_bytes,d->image_bytes);
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
   memcpy(p,"RMTD",4); put32(p+4,3);
   const uint64_t fields[]={d->mapping.owner,d->mapping.resource_cookie,
      d->mapping.allocation_generation,d->mapping.mapping_generation,
      d->mapping.gpu_va,d->mapping.mapped_bytes,d->mapping.resource_offset,
      d->image_bytes,d->mapping.cpu_write_epoch,d->mapping.device_visible_epoch,
      d->mapping.writer_operation,d->mapping.writer_generation};
   for (unsigned i=0;i<12;i++) put64(p+8+i*8,fields[i]);
   put32(p+104,d->width); put32(p+108,d->height); put32(p+112,d->stride);
   put32(p+116,d->format); put32(p+120,d->mapping.gem_handle);
   memcpy(p+124,d->swizzle,4);
   put64(p+128,d->resource_bytes);put64(p+136,d->level_offset);
   put32(p+144,d->first_level);put32(p+148,d->last_level);
   put64(p+152,d->content_epoch);
   memcpy(out,p,sizeof(p)); return 0;
}
int remesa_texture_descriptor_image_address(const struct remesa_texture_descriptor *d,
                                            uint32_t *address)
{
   if (!address) return EINVAL;
   int error=descriptor_valid(d); if (error) return error;
   /* descriptor_valid bounds the whole resource, then the selected mip,
    * inside one authenticated mapping. Each sum is therefore <=2^32. */
   uint64_t start=d->mapping.gpu_va+d->mapping.resource_offset+d->level_offset;
   if (start>UINT32_MAX || d->image_bytes>(UINT64_C(1)<<32)-start)
      return ERANGE;
   *address=(uint32_t)start; return 0;
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
