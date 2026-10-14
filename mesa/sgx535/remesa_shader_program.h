/* SPDX-License-Identifier: MIT
 * Original host compilation contract. This is not an SGX program image.
 */
#ifndef REMESA_SHADER_PROGRAM_H
#define REMESA_SHADER_PROGRAM_H
#include "remesa_shader_ir.h"
#include "remesa_usse.h"
#include "remesa_pds.h"
#include "remesa_pds_candidate.h"
#include "remesa_program_registers.h"
#include "remesa_shader_storage.h"

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
struct remesa_shader_pds_preview {
 enum remesa_ir_stage stage;
 uint64_t semantic_identity;
 struct remesa_pds_candidate_preview candidate;
};
/* A linked host semantic pair, not a GPU pipeline or executable program.
 * Identities are noncryptographic correlation keys, not resource ownership or
 * authentication. Current programs must be supplied again for revalidation. */
#define REMESA_SHADER_PIPELINE_SCHEMA 1u
struct remesa_shader_pipeline {
 uint32_t schema;
 enum remesa_shader_program_status status;
 uint64_t identity[2]; /* vertex, fragment */
 unsigned virtual_instructions[2];
 unsigned position_input_slot;
 uint32_t fragment_argb;
 int binary_error;
};
/* Links only the admitted position-passthrough and constant-color profiles.
 * Transactional, allocation-free, no NIR references or hardware bytes. */
int remesa_shader_pipeline_prepare(const struct remesa_shader_program *vertex,
 const struct remesa_shader_program *fragment,struct remesa_shader_pipeline *);
int remesa_shader_pipeline_revalidate(const struct remesa_shader_pipeline *,
 const struct remesa_shader_program *vertex,const struct remesa_shader_program *fragment);
/* Owned shader input for a draw backend. This carries the actual immutable
 * programs, rather than just their correlation keys or a constant color. A
 * backend must independently encode and qualify execution; this snapshot has
 * no GPU bytes, resource ownership or permission to use a historical shader.
 * It remains valid after the source shader objects and NIR are destroyed. */
struct remesa_shader_draw_programs {
 struct remesa_shader_program vertex,fragment;
 struct remesa_shader_pipeline pipeline;
};
/* Transactional, allocation-free; output/input overlap is rejected. */
int remesa_shader_draw_programs_prepare(const struct remesa_shader_program *vertex,
 const struct remesa_shader_program *fragment,struct remesa_shader_draw_programs *);
int remesa_shader_draw_programs_validate(const struct remesa_shader_draw_programs *);
/* Rebinding another source representation invalidates this handoff even if
 * its normalized lowering and correlation identity are unchanged. */
int remesa_shader_draw_programs_revalidate(const struct remesa_shader_draw_programs *,
 const struct remesa_shader_program *vertex,const struct remesa_shader_program *fragment);
/* Immutable preparation for the original native path. Owns the complete
 * semantic VS/FS pair and the independently serialized fragment CPU candidate.
 * Magenta/green prefer the one-record packed-constant candidate; the larger
 * float/PACK body remains available for the earlier yellow endpoint profile.
 * An explicit numeric VS candidate can be attached for CPU preparation only.
 * Qualified VS/PDS executable bytes remain absent.
 * This is host resource preparation, not upload or execution qualification. */
struct remesa_shader_image;
enum remesa_shader_candidate_form {
 REMESA_SHADER_PACKED_CONSTANT = 1,
 REMESA_SHADER_FLOAT_PACK_BODY = 2
};
struct remesa_shader_image_view {
 const struct remesa_shader_draw_programs *programs; /* borrowed, immutable */
 struct remesa_shader_storage_view fragment;
 struct remesa_shader_storage_view vertex; /* zero when no numeric mapping supplied */
 size_t vertex_byte_length; /* CPU candidate length, not executable storage */
 int execution_error;
 enum remesa_shader_candidate_form fragment_form; /* CPU grammar only */
};
/* Requires an empty output reference; unchanged on error. All source state is
 * copied and survives NIR, shader-state and source-pair destruction. */
int remesa_shader_image_prepare(const struct remesa_shader_draw_programs *,
 struct remesa_shader_image **);
/* Explicit offline numeric register assignment. Owns four independently
 * encoded CPU MOV records plus the fragment candidate. No default physical
 * bank mapping or termination is inferred. Execution still returns ENOTSUP. */
int remesa_shader_image_prepare_vertex_candidate(const struct remesa_shader_draw_programs *,
 const struct remesa_usse_vertex_mapping *,struct remesa_shader_image **);
int remesa_shader_image_validate(const struct remesa_shader_image *);
int remesa_shader_image_inspect(const struct remesa_shader_image *,
 struct remesa_shader_image_view *);
int remesa_shader_image_revalidate(const struct remesa_shader_image *,
 const struct remesa_shader_draw_programs *);
struct remesa_shader_request;
/* Allocation-free exact comparison of the CPU staging request to both owned
 * stages. Request owner/generation still need endpoint authentication. */
int remesa_shader_image_validate_request(const struct remesa_shader_image *,
 const struct remesa_shader_request *);
/* Preview public-kernel USE window arithmetic for the exact owned candidate
 * candidate. request.program_bytes must match its immutable encoded byte length;
 * the entry must be losslessly representable by the observed software fixups
 * (16-byte offset units). This is not a hardware alignment qualification.
 * VS requires an explicitly supplied numeric candidate. The allocation remains symbolic:
 * this neither authenticates a mapping nor promotes CPU records to executable
 * code. No task words or instructions are emitted. Output unchanged on error. */
int remesa_shader_image_preview_use_entry(const struct remesa_shader_image *,
 const struct remesa_use_entry_request *,struct remesa_use_entry_plan *);
/* No caller-supplied flag can qualify physical banks, PACK, termination or
 * PDS fetch. The current original candidate always returns ENOTSUP. */
int remesa_shader_image_validate_execution(const struct remesa_shader_image *);
void remesa_shader_image_destroy(struct remesa_shader_image *);
/* One atomic host preparation for a linked pair. Both requests describe one
 * caller-retained symbolic address space and operation epoch. All four selected
 * CODE/DATA windows must be disjoint; shared allocations must agree exactly on
 * mapping identity. No GEM reference, pin, upload or launch packet is created. */
struct remesa_shader_pipeline_launch {
 struct remesa_shader_pipeline semantic;
 struct remesa_pds_plan stage[2];
};
/* Owned semantic PDS pair plus symbolic references to exact original USE
 * bodies. No task/control payload, PDS instruction or execution state exists
 * in this preview. DATA slots and mapping metadata are supplied explicitly;
 * their hardware interpretation and provider authentication remain absent. */
struct remesa_shader_pds_links {
 struct remesa_shader_pipeline_launch launch;
 struct remesa_pds_use_link use[2];
};
int remesa_shader_pipeline_prepare_launch(const struct remesa_shader_program *vertex,
 const struct remesa_shader_program *fragment,const struct remesa_pds_request *vertex_request,
 const struct remesa_pds_request *fragment_request,struct remesa_shader_pipeline_launch *);
int remesa_shader_pipeline_revalidate_launch(const struct remesa_shader_pipeline_launch *,
 const struct remesa_shader_program *vertex,const struct remesa_shader_program *fragment,
 const struct remesa_pds_request *vertex_current,const struct remesa_pds_request *fragment_current);
/* Validates an owned host compilation, then previews caller-chosen candidate
 * PDS fields. This is NOT NIR-to-machine-code compilation or a launch plan.
 * No candidate is consumed by remesa_shader_program_encode[_launch]. */
int remesa_shader_program_preview_pds(const struct remesa_shader_program *,
 const struct remesa_pds_candidate_request *,struct remesa_shader_pds_preview *);
/* Owns by-value immutable semantic/lowering snapshots; no pointer into NIR.
 * Success means host lowering only. It never means executable GPU code.
 * Destination is unchanged on any error. */
int remesa_shader_program_prepare(const struct remesa_shader_ir *,
                                 struct remesa_shader_program *);
/* Specialize a synchronized uniform color into the same owned constant
 * fragment profile as NIR lowering. Only the existing opaque endpoint colors
 * are admitted; this does not compile a GPU uniform load or encode hardware.
 * Destination is unchanged on error. */
int remesa_shader_program_prepare_color(uint32_t argb,struct remesa_shader_program *);
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
