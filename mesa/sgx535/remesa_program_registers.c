/* SPDX-License-Identifier: MIT
 * Independently authored arithmetic over public register facts from Linux
 * v5.10 (commit 2c85ebc57b3e1817b6ce1a6b703928e113a90442), psb_reg.h.
 * Also: public GPLv2 Poulsbo kernel 4.42.0, psb_regman.c's register programming
 * and byte-window contract. See docs/phase9/USE-ENTRY-CONTRACT.md.
 * Those sources are credited for numerical/software field facts;
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
#define USE_ALIGNMENT_SHIFT 7u
#define USE_WINDOW_BYTES UINT64_C(0x80000)
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

int remesa_use_base_pack_address(uint64_t address,
                               enum remesa_use_data_master master,
                               uint32_t *word)
{
 if (!word || !master_valid(master)) return EINVAL;
 if (address > UINT32_MAX) return ERANGE;
 if (address % (UINT64_C(1) << USE_ALIGNMENT_SHIFT)) return EINVAL;
 const struct remesa_use_base_fields fields = {
  .address_field = (uint32_t)(address >> USE_ALIGNMENT_SHIFT),
  .data_master = master
 };
 return remesa_use_base_pack(&fields, word);
}

int remesa_use_base_unpack_address(uint32_t word, uint64_t *address,
                                 enum remesa_use_data_master *master)
{
 struct remesa_use_base_fields fields;
 if (!address || !master) return EINVAL;
 /* The two outputs must be independent: copying one over the other would
  * otherwise yield a successful call with corrupted metadata. */
 const uintptr_t a = (uintptr_t)address, m = (uintptr_t)master;
 if (sizeof(*address) > UINTPTR_MAX - a || sizeof(*master) > UINTPTR_MAX - m ||
     (a < m + sizeof(*master) && m < a + sizeof(*address))) return EINVAL;
 const int ret = remesa_use_base_unpack(word, &fields);
 if (ret) return ret;
 *address = (uint64_t)fields.address_field << USE_ALIGNMENT_SHIFT;
 *master = fields.data_master;
 return 0;
}

int remesa_use_entry_prepare(const struct remesa_use_entry_request *request,
                            struct remesa_use_entry_plan *out)
{
 if (!request || !out) return EINVAL;
 const struct remesa_use_entry_request r = *request;
 struct remesa_use_entry_plan plan = {0};
 if (r.data_master != REMESA_USE_VERTEX && r.data_master != REMESA_USE_PIXEL)
  return EINVAL;
 if (!r.allocation_bytes || !r.program_bytes) return EINVAL;
 if (r.allocation_address > UINT32_MAX ||
     r.allocation_bytes > (UINT64_C(1) << 32) - r.allocation_address ||
     r.entry_offset > r.allocation_bytes ||
     r.program_bytes > r.allocation_bytes - r.entry_offset) return ERANGE;
 plan.entry_address = r.allocation_address + r.entry_offset;
 plan.program_end = plan.entry_address + r.program_bytes;
 plan.base_address = plan.entry_address - plan.entry_address % USE_WINDOW_BYTES;
 /* The archived manager evaluates window endpoints in uint32_t. Do not plan
  * its final wrapping window, even if a wider host could represent that sum. */
 if (plan.base_address > UINT32_MAX - USE_WINDOW_BYTES) return ERANGE;
 const uint64_t relative = plan.entry_address - plan.base_address;
 /* Strictly less, matching psb_use_reg_reusable; equality is rejected. This
  * conservative software policy is not a claim about the silicon limit. */
 if (r.program_bytes >= USE_WINDOW_BYTES - relative) return ERANGE;
 plan.entry_byte_offset = (uint32_t)relative;
 plan.bank = r.bank;
 int ret = remesa_use_base_offset(plan.bank, &plan.register_offset);
 if (ret) return ret;
 ret = remesa_use_base_pack_address(plan.base_address, r.data_master,
                                  &plan.register_word);
 if (ret) return ret;
 *out = plan;
 return 0;
}

int remesa_use_entry_pair_prepare(const struct remesa_use_entry_request requests[2],
                                 struct remesa_use_entry_plan entries[2])
{
 if (!requests || !entries ||
     sizeof(*requests) * 2 > UINTPTR_MAX - (uintptr_t)requests ||
     sizeof(*entries) * 2 > UINTPTR_MAX - (uintptr_t)entries) return EINVAL;
 /* Copy both inputs before publishing either output, including aliasing calls. */
 const struct remesa_use_entry_request r[2] = {requests[0], requests[1]};
 struct remesa_use_entry_plan plan[2] = {{0}};
 if (r[0].data_master != REMESA_USE_VERTEX ||
     r[1].data_master != REMESA_USE_PIXEL) return EINVAL;
 for (unsigned i = 0; i < 2; ++i) {
  int ret = remesa_use_entry_prepare(&r[i], &plan[i]);
  if (ret) return ret;
  uint32_t address_bits;
  ret = remesa_use_entry_preview_fixup(&plan[i], &address_bits);
  if (ret) return ret;
 }
 /* This plan retains both stage settings at once. Reprogramming a shared
  * bank between tasks would require a separate, qualified scheduling contract. */
 if (plan[0].register_offset == plan[1].register_offset) return EBUSY;
 /* The original stage bodies use disjoint storage. Allocation extents may
  * coincide exactly (one backing with two windows), otherwise must not overlap. */
 const uint64_t end[2] = {r[0].allocation_address + r[0].allocation_bytes,
                          r[1].allocation_address + r[1].allocation_bytes};
 if (r[0].allocation_address < end[1] && r[1].allocation_address < end[0] &&
     (r[0].allocation_address != r[1].allocation_address || end[0] != end[1]))
  return EINVAL;
 if (plan[0].entry_address < plan[1].program_end &&
     plan[1].entry_address < plan[0].program_end) return EINVAL;
 entries[0] = plan[0];
 entries[1] = plan[1];
 return 0;
}

int remesa_use_entry_preview_fixup(const struct remesa_use_entry_plan *entry,
                                 uint32_t *out)
{
 if (!entry || !out) return EINVAL;
 const struct remesa_use_entry_plan p = *entry;
 uint64_t base;
 enum remesa_use_data_master master;
 uint32_t register_offset;
 int ret = remesa_use_base_unpack_address(p.register_word, &base, &master);
 if (ret) return ret;
 if (master != REMESA_USE_VERTEX && master != REMESA_USE_PIXEL) return ENOTSUP;
 ret = remesa_use_base_offset(p.bank, &register_offset);
 if (ret) return ret;
 if (base != p.base_address || p.register_offset != register_offset ||
     p.base_address % USE_WINDOW_BYTES || p.entry_address < p.base_address ||
     p.entry_address - p.base_address != p.entry_byte_offset) return EINVAL;
 if (p.base_address > UINT32_MAX - USE_WINDOW_BYTES ||
     p.entry_byte_offset >= USE_WINDOW_BYTES ||
     p.program_end <= p.entry_address ||
     p.program_end - p.base_address >= USE_WINDOW_BYTES) return ERANGE;
 /* These low bits are absent from the observed software representation.
  * Reject their loss, without asserting a physical instruction alignment. */
 if (p.entry_byte_offset % 16u) return EINVAL;
 const uint32_t offset = p.entry_byte_offset;
 const uint32_t bits = p.bank | (((offset >> 15) & 15u) << 4) |
                       (((offset >> 4) & 2047u) << 8);
 *out = bits;
 return 0;
}

int remesa_use_entry_decode_fixup(uint32_t bits,
                                struct remesa_use_entry_fixup_fields *out)
{
 if (!out || (bits & ~UINT32_C(0x7ffff))) return EINVAL;
 const struct remesa_use_entry_fixup_fields fields = {
  .bank = bits & 15u,
  .entry_byte_offset = ((bits & UINT32_C(0xf0)) << 11) |
                       ((bits & UINT32_C(0x7ff00)) >> 4)
 };
 *out = fields;
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
  .instruction_fields_known = 0, .use_va_packing_observed = 1
 };
 *out = evidence;
}
