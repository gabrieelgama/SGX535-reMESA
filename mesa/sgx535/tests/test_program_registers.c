/* SPDX-License-Identifier: MIT
 * Independent arithmetic tests for public MMIO fields, not ISA validation.
 * Values are sourced via the module's documented Linux register evidence.
 * No device access, proprietary shader data or execution is performed.
 */
#include "remesa_program_registers.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned long checks;
#define CHECK(x) do { assert(x); ++checks; } while (0)

static void test_use(void)
{
 const uint32_t addresses[] = {0, 1, 127, 128, 0x1000000, 0x1fffffe, 0x1ffffff};
 const unsigned masters[] = {0, 1, 3};
 for (unsigned m = 0; m < 3; ++m) {
  for (unsigned a = 0; a < sizeof(addresses)/sizeof(addresses[0]); ++a) {
   struct remesa_use_base_fields f = {addresses[a], (enum remesa_use_data_master)masters[m]}, decoded;
   uint32_t word = UINT32_MAX;
   CHECK(!remesa_use_base_pack(&f, &word));
   CHECK(word == addresses[a] + masters[m] * UINT32_C(33554432));
   CHECK(!remesa_use_base_unpack(word, &decoded));
   CHECK(decoded.address_field == f.address_field && decoded.data_master == f.data_master);
   /* Raw field remains raw; its value must not be treated as a byte VA. */
   struct remesa_use_base_fields overlap = f;
   CHECK(!remesa_use_base_pack(&overlap, &overlap.address_field));
   CHECK(overlap.address_field == word);
   CHECK(!remesa_use_base_unpack(overlap.address_field, &overlap));
   CHECK(overlap.address_field == f.address_field && overlap.data_master == f.data_master);
  }
 }
 uint32_t word = 0x12345678;
 struct remesa_use_base_fields f = {0, REMESA_USE_VERTEX};
 CHECK(remesa_use_base_pack(NULL, &word) == EINVAL && word == 0x12345678);
 CHECK(remesa_use_base_pack(&f, NULL) == EINVAL);
 const int bad_masters[] = {-1, 2, 4, 255};
 for (unsigned i = 0; i < sizeof(bad_masters)/sizeof(bad_masters[0]); ++i) {
  f.data_master = (enum remesa_use_data_master)bad_masters[i];
  CHECK(remesa_use_base_pack(&f, &word) == EINVAL && word == 0x12345678);
 }
 f.data_master = REMESA_USE_PIXEL;
 const uint32_t bad_addresses[] = {0x2000000, 0x80000000, UINT32_MAX};
 for (unsigned i = 0; i < sizeof(bad_addresses)/sizeof(bad_addresses[0]); ++i) {
  f.address_field = bad_addresses[i];
  CHECK(remesa_use_base_pack(&f, &word) == ERANGE && word == 0x12345678);
 }
 struct remesa_use_base_fields sentinel = {0x76543210, REMESA_USE_EDM}, out;
 for (unsigned bit = 27; bit < 32; ++bit) {
  out = sentinel;
  CHECK(remesa_use_base_unpack(UINT32_C(1) << bit, &out) == EINVAL);
  CHECK(!memcmp(&out, &sentinel, sizeof(out)));
 }
 out = sentinel;
 CHECK(remesa_use_base_unpack(UINT32_C(2) * 33554432, &out) == EINVAL);
 CHECK(!memcmp(&out, &sentinel, sizeof(out)));
 CHECK(remesa_use_base_unpack(0, NULL) == EINVAL);
 for (uint32_t bank = 0; bank < 16; ++bank) {
  word = UINT32_MAX;
  CHECK(!remesa_use_base_offset(bank, &word));
  CHECK(word == 2572 + 4 * bank);
 }
 const uint32_t bad_banks[] = {16, 17, UINT32_MAX};
 for (unsigned i = 0; i < sizeof(bad_banks)/sizeof(bad_banks[0]); ++i) {
  word = 0x12345678;
  CHECK(remesa_use_base_offset(bad_banks[i], &word) == ERANGE && word == 0x12345678);
 }
 CHECK(remesa_use_base_offset(0, NULL) == EINVAL);
}

static void test_pds(void)
{
 const uint64_t addresses[] = {0, 0x100000, 0x20000000, 0x80000000, 0xfff00000};
 for (unsigned i = 0; i < sizeof(addresses)/sizeof(addresses[0]); ++i) {
  uint32_t word = UINT32_MAX;
  uint64_t decoded = UINT64_MAX;
  CHECK(!remesa_pds_base_pack(addresses[i], &word));
  CHECK(word == addresses[i]); /* upstream raw 0x20000000 stays raw */
  CHECK(!remesa_pds_base_unpack(word, &decoded) && decoded == addresses[i]);
  union {uint64_t raw; uint32_t word;} overlap;
  overlap.raw = addresses[i];
  CHECK(!remesa_pds_base_pack(overlap.raw, &overlap.word));
  CHECK(overlap.word == addresses[i]);
  CHECK(!remesa_pds_base_unpack(overlap.word, &overlap.raw));
  CHECK(overlap.raw == addresses[i]);
 }
 uint32_t word = 0x12345678;
 uint64_t decoded = UINT64_C(0xabcdef0123456789);
 CHECK(remesa_pds_base_pack(0, NULL) == EINVAL);
 CHECK(remesa_pds_base_unpack(0, NULL) == EINVAL);
 const uint64_t wide[] = {UINT64_C(0x100000000), UINT64_C(0x100100000), UINT64_MAX};
 for (unsigned i = 0; i < sizeof(wide)/sizeof(wide[0]); ++i)
  CHECK(remesa_pds_base_pack(wide[i], &word) == ERANGE && word == 0x12345678);
 for (unsigned bit = 0; bit < 20; ++bit) {
  uint32_t bad = UINT32_C(0x20000000) + (UINT32_C(1) << bit);
  CHECK(remesa_pds_base_pack(bad, &word) == EINVAL && word == 0x12345678);
  CHECK(remesa_pds_base_unpack(bad, &decoded) == EINVAL && decoded == UINT64_C(0xabcdef0123456789));
 }
 CHECK(remesa_pds_base_offset() == 2744);
}

static void test_properties(void)
{
 /* Fixed-seed bounded input exploration. Independent arithmetic model uses
  * division/remainder rather than mirroring implementation bit operations. */
 uint32_t state = UINT32_C(0x721309af);
 for (unsigned i = 0; i < 65536; ++i) {
  state = state * UINT32_C(1664525) + UINT32_C(1013904223);
  uint32_t raw = state, address = raw % UINT32_C(33554432);
  unsigned master = raw / UINT32_C(33554432);
  struct remesa_use_base_fields out = {0x76543210, REMESA_USE_EDM}, before = out;
  int ret = remesa_use_base_unpack(raw, &out);
  int valid = master == 0 || master == 1 || master == 3;
  CHECK(ret == (valid ? 0 : EINVAL));
  if (valid) {
   CHECK(out.address_field == address && (unsigned)out.data_master == master);
   uint32_t back = UINT32_MAX;
   CHECK(!remesa_use_base_pack(&out, &back) && back == raw);
  } else CHECK(!memcmp(&out, &before, sizeof(out)));

  uint32_t word = 0x12345678;
  struct remesa_use_base_fields f = {address, (enum remesa_use_data_master)(i % 4)};
  ret = remesa_use_base_pack(&f, &word);
  CHECK(ret == (i % 4 == 2 ? EINVAL : 0));
  CHECK(word == (i % 4 == 2 ? UINT32_C(0x12345678) : address + (i % 4) * UINT32_C(33554432)));
  uint64_t pds = ((uint64_t)raw / UINT64_C(1048576)) * UINT64_C(1048576), unpacked = UINT64_MAX;
  CHECK(!remesa_pds_base_pack(pds, &word) && word == pds);
  CHECK(!remesa_pds_base_unpack(word, &unpacked) && unpacked == pds);
  word = 0x12345678;
  int pds_valid = raw % UINT32_C(1048576) == 0;
  ret = remesa_pds_base_pack(raw, &word);
  CHECK(ret == (pds_valid ? 0 : EINVAL));
  CHECK(word == (pds_valid ? raw : UINT32_C(0x12345678)));
 }
}

int main(void)
{
 test_use(); test_pds(); test_properties();
 struct remesa_program_register_evidence e;
 memset(&e, 0xa5, sizeof(e));
 remesa_program_register_evidence(&e);
 CHECK(e.use_banks == 16 && e.use_address_bits == 25 && e.use_data_master_shift == 25);
 CHECK(e.declared_use_alignment_shift == 7 && e.declared_pds_alignment_shift == 20);
 CHECK(!e.use_va_packing_verified && !e.rev121_verified && !e.instruction_fields_known);
 remesa_program_register_evidence(NULL);
 printf("PASS %lu public register-field checks; zero hardware execution; no ISA qualification\n", checks);
 return 0;
}
