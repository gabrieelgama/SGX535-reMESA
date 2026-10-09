/* SPDX-License-Identifier: MIT
 * Independently authored arithmetic over public register facts from Linux
 * v5.10 (commit 2c85ebc57b3e1817b6ce1a6b703928e113a90442), psb_reg.h.
 * That GPL-2.0-only source is credited for numerical hardware field facts;
 * no driver implementation, shader binary or instruction encoder is copied.
 * psb_drv.c's raw PDS_EXEC_BASE write corroborates the base representation.
 */
#include "remesa_program_registers.h"
#include <errno.h>
#include <stddef.h>

/* Every hardware constant below has a locator in the evidence document.
 * These are MMIO register fields; none is a USSE/PDS instruction encoding.
 */
#define USE_ADDRESS_MASK UINT32_C(0x01ffffff)
#define USE_DM_SHIFT 25u
#define USE_WORD_MASK UINT32_C(0x07ffffff)
#define USE_BASE_FIRST UINT32_C(0x0a0c)
#define USE_BANKS 16u
#define PDS_BASE_OFFSET UINT32_C(0x0ab8)
#define PDS_ALIGNMENT_SHIFT 20u
#define PDS_LOW_MASK ((UINT32_C(1) << PDS_ALIGNMENT_SHIFT) - 1u)

static int master_valid(enum remesa_use_data_master master)
{
 return master == REMESA_USE_VERTEX || master == REMESA_USE_PIXEL ||
        master == REMESA_USE_EDM;
}

int remesa_use_base_pack(const struct remesa_use_base_fields *fields,
                         uint32_t *word)
{
 if (!fields || !word || !master_valid(fields->data_master)) return EINVAL;
 if (fields->address_field > USE_ADDRESS_MASK) return ERANGE;
 /* Read all input before writing, so an overlapping output is harmless. */
 const uint32_t value = fields->address_field |
                        ((uint32_t)fields->data_master << USE_DM_SHIFT);
 *word = value;
 return 0;
}

int remesa_use_base_unpack(uint32_t word, struct remesa_use_base_fields *fields)
{
 if (!fields || (word & ~USE_WORD_MASK)) return EINVAL;
 struct remesa_use_base_fields decoded = {
  .address_field = word & USE_ADDRESS_MASK,
  .data_master = (enum remesa_use_data_master)(word >> USE_DM_SHIFT)
 };
 if (!master_valid(decoded.data_master)) return EINVAL;
 *fields = decoded;
 return 0;
}

int remesa_use_base_offset(uint32_t bank, uint32_t *offset)
{
 if (!offset) return EINVAL;
 if (bank >= USE_BANKS) return ERANGE;
 *offset = USE_BASE_FIRST + bank * 4u;
 return 0;
}

int remesa_pds_base_pack(uint64_t raw_base, uint32_t *word)
{
 if (!word) return EINVAL;
 if (raw_base > UINT32_MAX) return ERANGE;
 if (raw_base & PDS_LOW_MASK) return EINVAL;
 *word = (uint32_t)raw_base;
 return 0;
}

int remesa_pds_base_unpack(uint32_t word, uint64_t *raw_base)
{
 if (!raw_base || (word & PDS_LOW_MASK)) return EINVAL;
 *raw_base = word;
 return 0;
}

uint32_t remesa_pds_base_offset(void) { return PDS_BASE_OFFSET; }

void remesa_program_register_evidence(struct remesa_program_register_evidence *out)
{
 if (!out) return;
 const struct remesa_program_register_evidence evidence = {
  .use_banks = USE_BANKS, .use_address_bits = 25,
  .use_data_master_shift = USE_DM_SHIFT,
  .declared_use_alignment_shift = 7,
  .declared_pds_alignment_shift = PDS_ALIGNMENT_SHIFT,
  .use_va_packing_verified = 0, .rev121_verified = 0,
  .instruction_fields_known = 0
 };
 *out = evidence;
}
