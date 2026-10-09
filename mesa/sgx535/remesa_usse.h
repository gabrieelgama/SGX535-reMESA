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
#endif
