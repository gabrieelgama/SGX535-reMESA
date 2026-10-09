/* SPDX-License-Identifier: MIT
 * Original host launch/relocation requirements. These are NOT SGX PDS
 * instructions, task-control words, relocation bitfields or proven VA limits.
 * Code/data resources below are symbolic future program storage requirements;
 * host USSE serialization must never be uploaded as a shader program.
 */
#ifndef REMESA_PDS_H
#define REMESA_PDS_H
#include "remesa_usse.h"
#define REMESA_PDS_SCHEMA 1u
#define REMESA_PDS_RELOCATIONS 2u
/* IDs and slot count are original host plan definitions. */
enum remesa_pds_role { REMESA_PDS_CODE=1,REMESA_PDS_DATA=2 };
enum remesa_pds_relocation_kind { REMESA_PDS_SYMBOLIC_ADDRESS=1 };
struct remesa_pds_binding {
 uint64_t allocation_cookie,owner_cookie,mapping_generation;
 uint64_t address,offset,bytes,allocation_bytes;
};
struct remesa_pds_request {
 uint64_t operation,generation,owner_cookie;
 struct remesa_pds_binding code,data;
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
int remesa_pds_validate(const struct remesa_usse_program *,const struct remesa_pds_plan *);
/* Fresh trusted caller metadata must match the immutable request exactly. This
 * does not independently establish PFNs or verify the metadata provider. */
int remesa_pds_validate_binding(const struct remesa_usse_program *,const struct remesa_pds_plan *,const struct remesa_pds_request *current);
void remesa_pds_encoding_status(struct remesa_pds_encoding_status *);
int remesa_pds_encode(const struct remesa_usse_program *,const struct remesa_pds_plan *,void *,size_t,size_t *written);
#endif
