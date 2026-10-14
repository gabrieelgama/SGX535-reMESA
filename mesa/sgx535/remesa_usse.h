/* SPDX-License-Identifier: MIT
 * Original scalar, virtual-register scheduling IR. The name USSE denotes the
 * intended future backend, NOT established SGX instruction encodings. All
 * enum IDs, sizes and limits below are host schema definitions, not hardware.
 * Evidence: licensed Mesa/NIR float32 semantics and remesa_shader_ir profile
 * validation. Hardware bit fields, register banks, widths and alignments UNKNOWN.
 */
#ifndef REMESA_USSE_H
#define REMESA_USSE_H
#include "remesa_shader_ir.h"
#define REMESA_USSE_SCHEMA 1u
#define REMESA_USSE_VALUES 64u
#define REMESA_USSE_INSTRUCTIONS 128u
#define REMESA_USSE_NO_VALUE UINT32_MAX
#define REMESA_USSE_HOST_HEADER_BYTES 16u
#define REMESA_USSE_HOST_INSTRUCTION_BYTES 24u

enum remesa_usse_op {
 REMESA_USSE_INPUT=1, REMESA_USSE_CONSTANT, REMESA_USSE_MOVE,
 REMESA_USSE_EXPORT_POSITION, REMESA_USSE_EXPORT_COLOR
};
struct remesa_usse_instruction {
 uint32_t op, dst, src, channel, bits, reserved;
};
struct remesa_usse_program {
 uint32_t schema;
 enum remesa_ir_stage stage;
 uint32_t count;
 struct remesa_usse_instruction instructions[REMESA_USSE_INSTRUCTIONS];
};
enum remesa_encoding_evidence { REMESA_ENCODING_UNKNOWN=0 };
struct remesa_usse_encoding_status {
 enum remesa_encoding_evidence opcode, instruction_width, alignment;
 enum remesa_encoding_evidence register_banks, register_addressing, predicate;
 enum remesa_encoding_evidence execution_control, input_output, immediate;
};
/* Validation admits only identical float4 POSITION or existing endpoint COLOR
 * semantic profiles. Dense SSA, forward refs, unused nodes/storage rejected. */
int remesa_usse_validate(const struct remesa_usse_program *);
/* Pure COPY/COMPOSE4 aliases and exact-bit constants are shared after strict
 * IR validation. Equivalent chains produce identical scalar plans without
 * new values; repeated color channels share one virtual constant definition.
 * Existing host MOVE records remain valid; no hardware MOV is encoded. */
int remesa_usse_lower(const struct remesa_shader_ir *,struct remesa_usse_program *);
/* Original LE host format. Never submit these bytes as hardware program data.
 * Serialization/deserialization is transactional; every output unchanged on error. */
int remesa_usse_serialize_host(const struct remesa_usse_program *,void *,size_t,size_t *written);
int remesa_usse_deserialize_host(const void *,size_t,struct remesa_usse_program *);
/* Noncryptographic semantic identity, useful only for local correlation.
 * NOT authenticity, operation authorization or evidence of GPU completion. */
int remesa_usse_identity(const struct remesa_usse_program *,uint64_t *identity);
void remesa_usse_encoding_status(struct remesa_usse_encoding_status *);
/* No independently admitted hardware fields currently exist. Valid programs
 * return ENOTSUP; output bytes and written are never modified. */
int remesa_usse_encode(const struct remesa_usse_program *,void *,size_t,size_t *written);
/* Restricted CPU-writer format, independently reconstructed in the original
 * candidate-mov-codec-session. These numeric classes are NOT qualified hardware
 * banks. Eight serialized bytes are not a proven GPU fetch width. control11
 * and control18 reproduce independent CPU flag contributions; neither is a
 * qualified hardware termination contract. Both default to zero in candidates.
 * No function below authorizes upload to executable memory or submission. */
#define REMESA_USSE_CPU_MOVE_BYTES 8u
#define REMESA_USSE_VERTEX_CANDIDATE_BYTES (4u * REMESA_USSE_CPU_MOVE_BYTES)
struct remesa_usse_cpu_register { uint32_t class_id, number; };
struct remesa_usse_cpu_move {
 struct remesa_usse_cpu_register source, destination;
 uint32_t control11, control18;
};
int remesa_usse_cpu_move_encode(const struct remesa_usse_cpu_move *,
 void *,size_t,size_t *written);
int remesa_usse_cpu_move_decode(const void *,size_t,struct remesa_usse_cpu_move *);
/* Explicit numeric input/output assignments, supplied for offline candidates
 * only. No implicit class-to-attribute/output mapping is made. All destinations
 * must be distinct and disjoint from all inputs: the straight-line copy order
 * must not destroy an unread input. Hardware aliasing across classes UNKNOWN. */
struct remesa_usse_vertex_mapping {
 struct remesa_usse_cpu_register input[4], output[4];
};
int remesa_usse_vertex_mapping_validate(const struct remesa_usse_vertex_mapping *);
int remesa_usse_vertex_candidate(const struct remesa_usse_program *,
 const struct remesa_usse_vertex_mapping *,void *,size_t,size_t *written);
#endif
