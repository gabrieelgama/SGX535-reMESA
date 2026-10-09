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
};

/* All outputs remain unchanged on failure. USE address_field is the raw
 * 25-bit field: it is NOT a byte VA. The relation VA >> 7 is only inferred
 * from upstream's uncertain alignment comment and is not implemented here.
 * EDM is a public field value, not an admitted rendering execution profile.
 */
int remesa_use_base_pack(const struct remesa_use_base_fields *, uint32_t *word);
int remesa_use_base_unpack(uint32_t word, struct remesa_use_base_fields *);
int remesa_use_base_offset(uint32_t bank, uint32_t *offset);
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
