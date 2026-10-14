/* SPDX-License-Identifier: MIT
 * Original host semantic schema. All enum values and limits below belong to
 * reMESA's CPU-side IR; none are SGX opcodes, registers or hardware limits.
 * Float32 bit interpretation and stage/IO identities come from licensed Mesa
 * 25.2 NIR interfaces through remesa_nir.c. No proprietary shader bytes.
 */
#ifndef REMESA_SHADER_IR_H
#define REMESA_SHADER_IR_H
#include <stddef.h>
#include <stdint.h>
struct nir_shader;
#define REMESA_SHADER_IR_VALUES 16u
#define REMESA_SHADER_IR_INSTRUCTIONS 20u
#define REMESA_SHADER_IR_SCHEMA 1u
#define REMESA_SHADER_IR_NO_REGISTER UINT32_MAX

enum remesa_ir_stage { REMESA_IR_VERTEX=1, REMESA_IR_FRAGMENT=2 };
enum remesa_ir_type { REMESA_IR_FLOAT32=1 };
enum remesa_ir_op {
 REMESA_IR_INPUT=1, REMESA_IR_CONSTANT, REMESA_IR_COPY,
 REMESA_IR_COMPOSE4, REMESA_IR_POSITION, REMESA_IR_COLOR
};
struct remesa_ir_operand { uint32_t reg, channel; };
struct remesa_ir_instruction {
 enum remesa_ir_op op;
 uint32_t dst, components;
 enum remesa_ir_type type;
 struct remesa_ir_operand src[4];
 uint32_t bits[4];
 uint32_t input_slot;
};
struct remesa_shader_ir {
 uint32_t schema;
 enum remesa_ir_stage stage;
 uint32_t count;
 struct remesa_ir_instruction instructions[REMESA_SHADER_IR_INSTRUCTIONS];
};
/* Strict recognized semantic profiles only. No dead nodes/forward references,
 * malformed fields, extra exports or unsupported arithmetic. */
int remesa_shader_ir_validate(const struct remesa_shader_ir *);
/* Validated immutable semantic profile extraction; output unchanged on error. */
int remesa_shader_ir_vertex_input(const struct remesa_shader_ir *,unsigned *input_slot);
int remesa_shader_ir_fragment_color(const struct remesa_shader_ir *,uint32_t *argb);
/* Normalizes independently admitted NIR semantics. Does not mutate NIR or
 * generate hardware instructions. On failure the destination is unchanged. */
int remesa_shader_ir_lower(const struct nir_shader *,struct remesa_shader_ir *);
/* Deterministic semantic dump, explicitly NOT a hardware disassembler.
 * needed includes the NUL. ENOSPC never writes a partial string. */
int remesa_shader_ir_dump(const struct remesa_shader_ir *,char *,size_t,size_t *needed);
/* Hardware encoding remains UNKNOWN. A valid IR returns ENOTSUP and never
 * modifies bytes or written. Malformed IR is rejected before that boundary. */
int remesa_shader_ir_encode_sgx535(const struct remesa_shader_ir *,void *,size_t,size_t *written);
#endif
