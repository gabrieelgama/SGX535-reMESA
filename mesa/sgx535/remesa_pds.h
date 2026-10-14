/* SPDX-License-Identifier: MIT
 * Original host launch/relocation requirements. These are NOT SGX PDS
 * instructions, task-control words, relocation bitfields or proven VA limits.
 * Code/data resources below are symbolic future program storage requirements;
 * host USSE serialization must never be uploaded as a shader program.
 */
#ifndef REMESA_PDS_H
#define REMESA_PDS_H
#include "remesa_usse.h"
#include "remesa_program_registers.h"
#include "remesa_shader_storage.h"
#define REMESA_PDS_SCHEMA 1u
#define REMESA_PDS_RELOCATIONS 2u
/* IDs and slot count are original host plan definitions. */
enum remesa_pds_role { REMESA_PDS_CODE=1,REMESA_PDS_DATA=2 };
enum remesa_pds_relocation_kind { REMESA_PDS_SYMBOLIC_ADDRESS=1 };
struct remesa_pds_binding {
 /* address/allocation_bytes describe the complete caller-retained allocation
  * in the request's one symbolic address space, not just its selected window.
  * Distinct cookies must name disjoint extents. A shared cookie must carry one
  * exact mapping identity; its CODE/DATA windows may be disjoint subranges. */
 uint64_t allocation_cookie,owner_cookie,mapping_generation;
 uint64_t address,offset,bytes,allocation_bytes;
};
struct remesa_pds_request {
 uint64_t operation,generation,owner_cookie;
 struct remesa_pds_binding code,data;
};
/* Original host suballocation policy, not a SGX PDS layout specification.
 * allocation.offset/bytes retain a bounded window within one allocation.
 * DATA begins at that window; CODE follows DATA and explicit host padding to
 * code_alignment bytes in this symbolic address space. Alignment is supplied
 * by the caller, never inferred from an opcode or hardware descriptor.
 * No storage is allocated, mapped, written, pinned or made executable here. */
struct remesa_pds_block_request {
 uint64_t operation,generation;
 struct remesa_pds_binding allocation;
 uint64_t data_bytes,code_bytes,code_alignment;
};
struct remesa_pds_relocation {
 uint32_t role,kind;
 uint64_t address,bytes;
};
struct remesa_pds_plan {
 uint32_t schema;
 enum remesa_ir_stage stage;
 uint64_t program_identity;
 /* Immutable canonical semantic copy, not hardware bytes or GEM ownership. */
 struct remesa_usse_program program;
 struct remesa_pds_request request;
 uint32_t relocation_count,reserved;
 struct remesa_pds_relocation relocations[REMESA_PDS_RELOCATIONS];
};
struct remesa_pds_encoding_status {
 enum remesa_encoding_evidence instruction_width,alignment,opcode;
 enum remesa_encoding_evidence task_control,usse_address,constant_address;
 enum remesa_encoding_evidence register_banks,launch_semantics;
};
/* Caller supplies already-owned symbolic storage; this module acquires no
 * GEM references and does not independently verify PFNs, pins or VA mappings.
 * Exact program identity, operation/generation and bounds are revalidated.
 * All outputs unchanged on failure. */
int remesa_pds_prepare(const struct remesa_usse_program *,const struct remesa_pds_request *,struct remesa_pds_plan *);
/* Derive exact disjoint windows transactionally. This does not assert a
 * hardware data/code prefix, instruction width or receiver fetch rule. */
int remesa_pds_request_from_block(const struct remesa_pds_block_request *,struct remesa_pds_request *);
int remesa_pds_validate(const struct remesa_usse_program *,const struct remesa_pds_plan *);
/* Fresh trusted caller metadata must match the immutable request exactly. This
 * does not independently establish PFNs or verify the metadata provider. */
int remesa_pds_validate_binding(const struct remesa_usse_program *,const struct remesa_pds_plan *,const struct remesa_pds_request *current);
/* Symbolic connection between an exact USE entry and a caller-selected DATA
 * location. Only address_bits[18:0] have the existing software fixup meaning;
 * no upper task-control bits, MOVS source, destination or consumption arity
 * are assigned. The four-byte location is an original host relocation policy,
 * NOT evidence of PDS data-bank layout, fetch width or task word count.
 * Neither this record nor the candidate CPU program may be uploaded/executed.
 */
struct remesa_pds_use_link {
 struct remesa_pds_request request;
 struct remesa_pds_binding program;
 struct remesa_use_entry_plan entry;
 enum remesa_ir_stage stage;
 uint32_t address_bits;
 uint64_t program_identity,data_byte_offset,data_address;
};
/* VS/FS in fixed order, one symbolic address space and operation epoch.
 * Checks all six selected CODE/DATA/USE windows and their complete allocation
 * identities, exact entry bounds, distinct concurrent USE bank settings and
 * four-byte DATA locations. Caller retains/authenticates every allocation;
 * these helpers acquire no references or executable mappings. Failure leaves
 * both outputs unchanged; output/input overlap and pointer wrap are rejected.
 */
int remesa_pds_prepare_use_links(const struct remesa_pds_plan plans[2],
 const struct remesa_pds_binding programs[2],const uint32_t banks[2],
 const uint64_t data_byte_offsets[2],struct remesa_pds_use_link links[2]);
/* Fresh source bindings and PDS plans must match the immutable link exactly.
 * Self-consistency is not authentication of a GPU mapping or task consumer. */
int remesa_pds_validate_use_links(const struct remesa_pds_plan plans[2],
 const struct remesa_pds_binding programs[2],const struct remesa_pds_use_link links[2]);
/* Original numerical preview of the retained CPU address/prefix convention.
 * It is NOT a complete hardware descriptor or PDS program. T+4 is absent;
 * bits25:24 of T+8 are unknown and are not silently assigned a value.
 * block_address and code_window_address preserve both competing fetch points;
 * neither is named the first PC. CODE/DATA must be disjoint windows of one
 * retained allocation. Limits/alignment ensure lossless CPU packing only.
 * No task-control, memory publication, GPU address authentication or execution.
 */
struct remesa_pds_record_preview {
 uint64_t operation,generation,owner_cookie;
 uint64_t allocation_cookie,mapping_generation;
 uint64_t block_address,prefix_bytes,code_window_address,code_bytes;
 uint64_t use_entry_address,use_program_bytes,data_fixup_address;
 uint32_t address_prefix_bits,address_prefix_known_mask,high_address_region;
 uint32_t use_address_bits;
};
/* Atomic VS/FS projections from already validated original symbolic links.
 * Output must be disjoint from every input. Unknown bits are retained as a mask,
 * never serialized to a command stream. Failure leaves both outputs unchanged.
 */
int remesa_pds_preview_record_pair(const struct remesa_pds_plan *plans,
 const struct remesa_pds_binding *programs,const struct remesa_pds_use_link *links,
 struct remesa_pds_record_preview *out);
/* Caller-owned accessible HOST DATA windows, never executable GPU mappings.
 * Binding must exactly match the retained symbolic DATA window. The byte
 * pointer starts at that window, not at the allocation base. Caller retains
 * storage and serializes access throughout the call. */
struct remesa_pds_host_data {
 struct remesa_pds_binding binding;
 void *bytes;
 size_t byte_length;
};
/* Borrowed immutable HOST DATA, valid only while its native token is live.
 * Symbolic binding is not an authenticated GPU mapping. */
struct remesa_pds_host_data_view {
 struct remesa_pds_binding binding;
 struct remesa_shader_storage_view storage;
};
/* Apply only the established lower19 CPU entry relocation in little endian.
 * Preserve every other bit and byte supplied by the original host builder.
 * This does not supply/validate task controls, DS routing, source arity, PDS
 * instructions, fetch or termination; the result remains non-executable.
 * Validate both stages before either write, with no allocation or callback.
 * HOST windows must be disjoint from each other and every input descriptor.
 * CODE/DATA may reside in separate bounded symbolic allocations. Entry
 * relocation does not require a T+8 descriptor preview or choose a first PC.
 * Any failure leaves both byte buffers unchanged. */
int remesa_pds_apply_host_entry_fixups(const struct remesa_pds_plan *plans,
 const struct remesa_pds_binding *programs,const struct remesa_pds_use_link *links,
 const struct remesa_pds_host_data *data);
/* Revalidate the exact lower19 entry fields already present in both HOST
 * DATA snapshots against their canonical live links. Read-only; no fixup,
 * output publication, task-control interpretation or hardware qualification.
 * Uses the same binding/bounds/alias checks as apply_host_entry_fixups and
 * permits separate CODE/DATA. Mismatch returns EPROTO; bytes stay unchanged. */
int remesa_pds_validate_host_entry_fixups(const struct remesa_pds_plan *plans,
 const struct remesa_pds_binding *programs,const struct remesa_pds_use_link *links,
 const struct remesa_pds_host_data *data);
void remesa_pds_encoding_status(struct remesa_pds_encoding_status *);
int remesa_pds_encode(const struct remesa_usse_program *,const struct remesa_pds_plan *,void *,size_t,size_t *written);
#endif
