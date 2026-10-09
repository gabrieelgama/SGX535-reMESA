/* SPDX-License-Identifier: MIT
 * Original host compilation contract. This is not an SGX program image.
 */
#ifndef REMESA_SHADER_PROGRAM_H
#define REMESA_SHADER_PROGRAM_H
#include "remesa_shader_ir.h"
#include "remesa_usse.h"
#include "remesa_pds.h"
#include "remesa_program_registers.h"

/* Host policy identifiers, not hardware version/register fields. */
#define REMESA_SHADER_PROGRAM_SCHEMA 1u
enum remesa_shader_program_status {
 REMESA_PROGRAM_SEMANTIC_ONLY = 1
};
struct remesa_shader_program {
 uint32_t schema;
 enum remesa_shader_program_status status;
 struct remesa_shader_ir semantic;
 struct remesa_usse_program usse;
};
struct remesa_shader_program_report {
 enum remesa_ir_stage stage;
 enum remesa_shader_program_status status;
 unsigned virtual_instructions;
 int binary_error;
 /* Public program-base field facts are reported separately from executable
  * instruction evidence. Their presence must never admit a GPU shader. */
 struct remesa_program_register_evidence register_evidence;
};
/* Owns by-value immutable semantic/lowering snapshots; no pointer into NIR.
 * Success means host lowering only. It never means executable GPU code.
 * Destination is unchanged on any error. */
int remesa_shader_program_prepare(const struct remesa_shader_ir *,
                                 struct remesa_shader_program *);
int remesa_shader_program_validate(const struct remesa_shader_program *);
int remesa_shader_program_report(const struct remesa_shader_program *,
                                struct remesa_shader_program_report *);
/* No partial output and no fallback to the historical program. An admitted
 * instruction format is required before bytes can ever be returned. */
int remesa_shader_program_encode(const struct remesa_shader_program *,
                                void *,size_t,size_t *);
/* Prepare only symbolic resource requirements. This does not pin resources,
 * authenticate mappings, or generate a GPU launch. Current ownership is
 * supplied afresh and checked again before any future emission. */
int remesa_shader_program_prepare_launch(const struct remesa_shader_program *,
 const struct remesa_pds_request *,struct remesa_pds_plan *);
int remesa_shader_program_encode_launch(const struct remesa_shader_program *,
 const struct remesa_pds_plan *,const struct remesa_pds_request *,
 void *,size_t,size_t *);
/* Explicit host ownership release; idempotent. Not GEM retirement. */
void remesa_shader_program_finish(struct remesa_shader_program *);
#endif
