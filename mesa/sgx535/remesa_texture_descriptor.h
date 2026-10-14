/* SPDX-License-Identifier: MIT */
#ifndef REMESA_TEXTURE_DESCRIPTOR_H
#define REMESA_TEXTURE_DESCRIPTOR_H
#include "remesa_texture_binding.h"
#include <stdbool.h>
/* Original HOST metadata only. No field below encodes SGX registers or ISA.
 * Gallium enum meanings originate in the pinned licensed Mesa 25.2.0 headers.
 * The provider query must report retained, authenticated mapping/visibility
 * state under its existing serialization. Caller input or an ioctl's success
 * alone cannot supply this evidence. This helper neither takes references nor
 * authenticates physical GPU completion; bank/view and provider must remain
 * alive until revalidation and command retirement. */
struct remesa_texture_mapping {
   uint64_t owner, resource_cookie, allocation_generation, mapping_generation;
   uint64_t gpu_va, mapped_bytes, resource_offset;
   uint64_t cpu_write_epoch, device_visible_epoch;
   uint64_t writer_operation, writer_generation;
   uint64_t completed_operation, completed_generation;
   uint32_t gem_handle, pins, cpu_leases;
   bool mapped, pfn_verified, quarantined;
};
typedef int (*remesa_texture_mapping_query)(void *, struct pipe_resource *,
                                           struct remesa_texture_mapping *);
struct pipe_sampler_state;
struct remesa_texture_descriptor {
   struct remesa_texture_mapping mapping;
   uint64_t image_bytes;
   uint32_t width, height, stride, format;
   uint8_t swizzle[4];
   uint64_t resource_bytes, level_offset;
   uint32_t first_level, last_level;
   uint64_t content_epoch;
};
/* Immutable preparation; every failure leaves *out unchanged. Uses the real
 * bank's pointer-membership, resource-cookie, layout and lifetime validation. */
int remesa_texture_descriptor_prepare(struct remesa_texture_binding_bank *,
   const struct pipe_sampler_view *, const struct pipe_sampler_state *,
   uint64_t expected_owner, remesa_texture_mapping_query, void *,
   struct remesa_texture_descriptor *out);
/* Re-query immediately before use: stale mapping, contents, view or owner is
 * rejected. This admits only semantic metadata, NOT physical GPU sampling. */
int remesa_texture_descriptor_revalidate(const struct remesa_texture_descriptor *,
   struct remesa_texture_binding_bank *, const struct pipe_sampler_view *,
   const struct pipe_sampler_state *, uint64_t expected_owner,
   remesa_texture_mapping_query, void *);
#define REMESA_TEXTURE_HOST_METADATA_BYTES 160u
/* Canonical little-endian diagnostic host format RMTD v3. The mapping offset
 * selects the whole resource; level_offset additionally selects the base mip
 * of the view. This is not an SGX address bitfield or sampling instruction.
 * It MUST NOT be
 * submitted as a GPU descriptor. No native pointers, padding, or enum ABI. */
int remesa_texture_descriptor_serialize(const struct remesa_texture_descriptor *,
                                       void *, size_t);
/* Checked byte address for the selected base mip in the current 32-bit
 * relocation address space. This is NOT a texture descriptor or proof of
 * alignment/format applicability to a sampling unit. No wrapping relocation
 * arithmetic is permitted; failure preserves *address. */
int remesa_texture_descriptor_image_address(const struct remesa_texture_descriptor *,
                                            uint32_t *address);
/* There is no independently established SGX texture encoding. Always ENOTSUP;
 * destination and *written are preserved. Never substitutes host metadata. */
int remesa_texture_descriptor_emit_sgx(const struct remesa_texture_descriptor *,
                                      void *, size_t, size_t *written);
#endif
