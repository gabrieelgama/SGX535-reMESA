/* SPDX-License-Identifier: MIT
 * Original codec for publicly described gma500 MMIO fields, NOT shader words.
 * Evidence: docs/phase9/sgx535-launch-field-evidence-20261009.json.
 * There is deliberately no MMIO access, program upload or dispatch interface.
 */
#ifndef REMESA_PROGRAM_REGISTERS_H
#define REMESA_PROGRAM_REGISTERS_H
#include <stdint.h>

enum remesa_use_data_master {
 REMESA_USE_VERTEX = 0, REMESA_USE_PIXEL = 1, REMESA_USE_EDM = 3
};
struct remesa_use_base_fields {
 uint32_t address_field;
 enum remesa_use_data_master data_master;
};
struct remesa_program_register_evidence {
 /* Publication facts, not physical SGX535 rev121 qualification. */
 uint32_t use_banks, use_address_bits, use_data_master_shift;
 uint32_t declared_use_alignment_shift, declared_pds_alignment_shift;
 uint32_t use_va_packing_verified, rev121_verified, instruction_fields_known;
 /* A public historical CPU call site, distinct from physical qualification. */
 uint32_t use_va_packing_observed;
};

/* All outputs remain unchanged on failure. USE address_field is the raw
 * 25-bit field: it is NOT a byte VA. The separate address helpers implement
 * a public historical CPU packing call; physical interpretation remains unknown.
 * EDM is a public field value, not an admitted rendering execution profile.
 */
int remesa_use_base_pack(const struct remesa_use_base_fields *, uint32_t *word);
int remesa_use_base_unpack(uint32_t word, struct remesa_use_base_fields *);
int remesa_use_base_offset(uint32_t bank, uint32_t *offset);
/* Public Poulsbo kernel software uses base_address >> 7 in the register field.
 * These helpers encode that observed CPU operation, not a qualified SGX PC.
 * Base alignment is 128 bytes; the software window below is stricter.
 * EDM remains representable here, but is rejected for rendering entry plans. */
int remesa_use_base_pack_address(uint64_t base_address,
                               enum remesa_use_data_master, uint32_t *word);
int remesa_use_base_unpack_address(uint32_t word, uint64_t *base_address,
                                 enum remesa_use_data_master *);

struct remesa_use_entry_request {
 /* One already retained GPU-address allocation. These fields do not authenticate
  * its provider, mapping or bank ownership. Byte lengths are supplied by the
  * original encoder, never inferred from historical task metadata. */
 uint64_t allocation_address, allocation_bytes, entry_offset, program_bytes;
 uint32_t bank;
 enum remesa_use_data_master data_master;
};
struct remesa_use_entry_plan {
 uint64_t base_address, entry_address, program_end;
 uint32_t bank, register_offset, register_word, entry_byte_offset;
};
/* Checked original planning of the public kernel's 512 KiB software window.
 * Requires exact program bounds and uses that kernel's strict end predicate.
 * No instruction alignment, task serialization, PDS fetch, pin, MMIO or submit.
 * Input/output may alias; failure leaves output unchanged. */
int remesa_use_entry_prepare(const struct remesa_use_entry_request *,
                            struct remesa_use_entry_plan *);
/* Atomic VS/PX pair, in that order, using the existing software entry rules.
 * Each entry must be losslessly representable by the observed fixup fields.
 * A single register cannot retain both different data-master words: distinct
 * banks are required for this simultaneous plan. Stage byte ranges must be
 * disjoint; allocations may be identical or disjoint, not partially aliased.
 * This is not a bank lease,
 * MMIO operation, physical register-bank mapping or execution qualification.
 * No data/code framing or termination is inferred. Input/output may overlap;
 * both outputs are unchanged on failure, including failure of the second stage.
 */
int remesa_use_entry_pair_prepare(const struct remesa_use_entry_request requests[2],
                                 struct remesa_use_entry_plan entries[2]);
struct remesa_use_entry_fixup_fields {
 uint32_t bank, entry_byte_offset;
};
/* Analysis-only projection of the observed, ordered CPU relocation fields.
 * Requires a self-consistent entry plan and lossless 16-byte offset packing.
 * Produces ONLY the lower 19 address/bank bits, not a task/control word. No
 * MOVS, DOUTU, hardware alignment, ownership or fetch semantics are certified.
 * Callers still authenticate the allocation and bank; native token preview
 * supplies exact owned bounds. Input/output may alias; failures are atomic. */
int remesa_use_entry_preview_fixup(const struct remesa_use_entry_plan *,
                                 uint32_t *address_bits);
/* Rejects all upper control bits rather than assigning or discarding their
 * unknown meaning. Decodes software fields only; no execution admission. */
int remesa_use_entry_decode_fixup(uint32_t address_bits,
                                struct remesa_use_entry_fixup_fields *);
/* PDS base is a raw 32-bit, 1 MiB-aligned register address, corroborated by
 * upstream initialization. This does not qualify mappings, address reach,
 * PDS instruction alignment or a launch program. Address zero is a valid
 * field value here, not permission to execute a null-address program.
 */
int remesa_pds_base_pack(uint64_t raw_base, uint32_t *word);
int remesa_pds_base_unpack(uint32_t word, uint64_t *raw_base);
uint32_t remesa_pds_base_offset(void);
void remesa_program_register_evidence(struct remesa_program_register_evidence *);
#endif
