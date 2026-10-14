/* SPDX-License-Identifier: MIT
 * Original bounds tests for public kernel software address arithmetic.
 * No task/instruction bytes, hardware access or executor admission.
 */
#include "remesa_program_registers.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static unsigned checks;
#define CHECK(x) do { assert(x); ++checks; } while (0)

static void test_address(void)
{
 const uint64_t addresses[] = {0, 128, 0x80000, 0x10000000, 0xffffff80};
 const enum remesa_use_data_master masters[] = {
  REMESA_USE_VERTEX, REMESA_USE_PIXEL, REMESA_USE_EDM
 };
 for (unsigned i = 0; i < sizeof(addresses)/sizeof(addresses[0]); ++i) {
  for (unsigned j = 0; j < sizeof(masters)/sizeof(masters[0]); ++j) {
   uint32_t word = 0xdeadbeef;
   uint64_t address = UINT64_MAX;
   enum remesa_use_data_master master = REMESA_USE_VERTEX;
   CHECK(!remesa_use_base_pack_address(addresses[i], masters[j], &word));
   /* Division is an independent arithmetic expectation. */
   CHECK(word == addresses[i] / 128 + (uint32_t)masters[j] * 33554432u);
   CHECK(!remesa_use_base_unpack_address(word, &address, &master));
   CHECK(address == addresses[i] && master == masters[j]);
  }
 }
 uint32_t word = 0xdeadbeef;
 CHECK(remesa_use_base_pack_address(1, REMESA_USE_PIXEL, &word) == EINVAL);
 CHECK(word == 0xdeadbeef);
 CHECK(remesa_use_base_pack_address(UINT64_C(0x100000000), REMESA_USE_PIXEL,
                                   &word) == ERANGE && word == 0xdeadbeef);
 CHECK(remesa_use_base_pack_address(0, (enum remesa_use_data_master)2,
                                   &word) == EINVAL && word == 0xdeadbeef);
 CHECK(remesa_use_base_pack_address(0, REMESA_USE_PIXEL, NULL) == EINVAL);
 uint64_t address = UINT64_MAX;
 enum remesa_use_data_master master = REMESA_USE_PIXEL;
 const uint32_t bad[] = {UINT32_MAX, 2u << 25, 1u << 27};
 for (unsigned i = 0; i < sizeof(bad)/sizeof(bad[0]); ++i) {
  CHECK(remesa_use_base_unpack_address(bad[i], &address, &master) == EINVAL);
  CHECK(address == UINT64_MAX && master == REMESA_USE_PIXEL);
 }
 CHECK(remesa_use_base_unpack_address(0, NULL, &master) == EINVAL);
 CHECK(remesa_use_base_unpack_address(0, &address, NULL) == EINVAL);
 union {uint64_t address; enum remesa_use_data_master master;} alias;
 memset(&alias, 0xa5, sizeof(alias));
 const uint64_t before = alias.address;
 CHECK(remesa_use_base_unpack_address(0, &alias.address, &alias.master) == EINVAL);
 CHECK(alias.address == before);
}

static void rejected(struct remesa_use_entry_request r, int expected)
{
 struct remesa_use_entry_plan out, before;
 memset(&out, 0xa5, sizeof(out)); before = out;
 CHECK(remesa_use_entry_prepare(&r, &out) == expected);
 CHECK(!memcmp(&out, &before, sizeof(out)));
}

static void test_entry(void)
{
 const struct remesa_use_entry_request initial = {
  .allocation_address = 0x10000000, .allocation_bytes = 4096,
  .entry_offset = 0x140, .program_bytes = 8, .bank = 3,
  .data_master = REMESA_USE_PIXEL
 };
 struct remesa_use_entry_plan plan;
 CHECK(!remesa_use_entry_prepare(&initial, &plan));
 CHECK(plan.base_address == 0x10000000 && plan.entry_address == 0x10000140);
 CHECK(plan.program_end == 0x10000148 && plan.entry_byte_offset == 0x140);
 CHECK(plan.bank == 3 && plan.register_offset == 0xa18);
 CHECK(plan.register_word == 0x2200000);
 struct remesa_use_entry_request r = initial;
 const uint64_t offsets[] = {0, 1, 127, 128, 0x140, 4088};
 for (unsigned i = 0; i < sizeof(offsets)/sizeof(offsets[0]); ++i) {
  r.entry_offset = offsets[i];
  CHECK(!remesa_use_entry_prepare(&r, &plan));
  CHECK(plan.entry_address == r.allocation_address + offsets[i]);
  CHECK(plan.entry_byte_offset == offsets[i] && plan.program_end == plan.entry_address + 8);
 }
 /* Byte arithmetic does not certify a one-byte-aligned GPU instruction. */
 r = initial; r.bank = 15;
 CHECK(!remesa_use_entry_prepare(&r, &plan) && plan.register_offset == 0xa48);
 r.bank = 16; rejected(r, ERANGE);
 r = initial; r.data_master = REMESA_USE_EDM; rejected(r, EINVAL);
 r.data_master = (enum remesa_use_data_master)2; rejected(r, EINVAL);
 r = initial; r.allocation_bytes = 0; rejected(r, EINVAL);
 r = initial; r.program_bytes = 0; rejected(r, EINVAL);
 r = initial; r.entry_offset = 4097; rejected(r, ERANGE);
 r = initial; r.entry_offset = 4090; rejected(r, ERANGE);
 r = initial; r.entry_offset = UINT64_MAX; rejected(r, ERANGE);
 r = initial; r.program_bytes = UINT64_MAX; rejected(r, ERANGE);
 r = initial; r.allocation_bytes = UINT64_MAX; rejected(r, ERANGE);
 r = initial; r.allocation_address = UINT64_C(0x100000000); rejected(r, ERANGE);
 r = initial; r.allocation_address = UINT32_MAX - 100; rejected(r, ERANGE);
 r = initial; r.allocation_address = 0x1007fff0; r.entry_offset = 0;
 r.program_bytes = 15;
 CHECK(!remesa_use_entry_prepare(&r, &plan));
 CHECK(plan.entry_byte_offset == 0x7fff0 && plan.program_end == 0x1007ffff);
 r.program_bytes = 16; rejected(r, ERANGE); /* exact window end is excluded */
 r.program_bytes = 17; rejected(r, ERANGE);
 /* Relocation size=2 would incorrectly admit a 16-byte body here. */
 r.program_bytes = 2;
 CHECK(!remesa_use_entry_prepare(&r, &plan));
 r.program_bytes = 16; rejected(r, ERANGE);
 /* Same allocation, next window: base must move, not truncate the offset. */
 r.entry_offset = 16; r.program_bytes = 8;
 CHECK(!remesa_use_entry_prepare(&r, &plan));
 CHECK(plan.base_address == 0x10080000 && !plan.entry_byte_offset);
 r = initial; r.allocation_address = 0xfffff000; r.entry_offset = 4088;
 rejected(r, ERANGE); /* historical window-end addition would wrap */
 struct remesa_use_entry_plan before;
 memset(&plan, 0xa5, sizeof(plan)); before = plan;
 CHECK(remesa_use_entry_prepare(NULL, &plan) == EINVAL);
 CHECK(!memcmp(&plan, &before, sizeof(plan)));
 CHECK(remesa_use_entry_prepare(&initial, NULL) == EINVAL);
 union {struct remesa_use_entry_request request; struct remesa_use_entry_plan plan;} alias;
 alias.request = initial;
 CHECK(!remesa_use_entry_prepare(&alias.request, &alias.plan));
 CHECK(alias.plan.entry_address == 0x10000140 && alias.plan.program_end == 0x10000148);
}

static void fixup_rejected(const struct remesa_use_entry_plan *plan, int error)
{
 uint32_t bits = 0xdeadbeef;
 CHECK(remesa_use_entry_preview_fixup(plan, &bits) == error);
 CHECK(bits == 0xdeadbeef);
}

static void test_fixup(void)
{
 struct remesa_use_entry_request request = {
  .allocation_address = 0x10000000, .allocation_bytes = 0x80000,
  .program_bytes = 8, .bank = 3, .data_master = REMESA_USE_PIXEL
 };
 /* Boundary cases for the two independently packed offset portions. These
  * expectations use division/modulo rather than the implementation shifts. */
 const uint32_t offsets[] = {0, 16, 0x140, 0x7ff0, 0x8000, 0x8010,
                             0x10000, 0x40000, 0x7ffe0, 0x7fff0};
 struct remesa_use_entry_plan plan;
 for (unsigned i = 0; i < sizeof(offsets)/sizeof(offsets[0]); ++i) {
  request.entry_offset = offsets[i];
  request.bank = i % 2 ? 15 : 0;
  request.data_master = i % 2 ? REMESA_USE_VERTEX : REMESA_USE_PIXEL;
  uint32_t bits = UINT32_MAX;
  struct remesa_use_entry_fixup_fields decoded;
  CHECK(!remesa_use_entry_prepare(&request, &plan));
  CHECK(!remesa_use_entry_preview_fixup(&plan, &bits));
  CHECK(bits == request.bank + (offsets[i] / 32768) * 16 +
                ((offsets[i] / 16) % 2048) * 256);
  CHECK(!remesa_use_entry_decode_fixup(bits, &decoded));
  CHECK(decoded.bank == request.bank && decoded.entry_byte_offset == offsets[i]);
  CHECK(plan.base_address + decoded.entry_byte_offset == plan.entry_address);
 }
 request.entry_offset = 0x140; request.bank = 3;
 request.data_master = REMESA_USE_PIXEL;
 CHECK(!remesa_use_entry_prepare(&request, &plan));
 const struct remesa_use_entry_plan initial = plan;
 uint32_t bits = UINT32_MAX;
 CHECK(!remesa_use_entry_preview_fixup(&plan, &bits) && bits == 0x1403);
 for (unsigned low = 1; low < 16; ++low) {
  request.entry_offset = 0x140 + low;
  CHECK(!remesa_use_entry_prepare(&request, &plan));
  fixup_rejected(&plan, EINVAL); /* previously valid arithmetic, lossy fixup */
 }
 plan = initial; plan.bank = 16; fixup_rejected(&plan, ERANGE);
 plan = initial; plan.register_offset += 4; fixup_rejected(&plan, EINVAL);
 plan = initial; plan.register_word ^= 1; fixup_rejected(&plan, EINVAL);
 plan = initial; plan.base_address += 128; fixup_rejected(&plan, EINVAL);
 plan = initial; plan.entry_byte_offset += 16; fixup_rejected(&plan, EINVAL);
 plan = initial; plan.entry_address = plan.base_address - 1;
 fixup_rejected(&plan, EINVAL);
 plan = initial; plan.program_end = plan.entry_address; fixup_rejected(&plan, ERANGE);
 plan = initial; plan.program_end = UINT64_MAX; fixup_rejected(&plan, ERANGE);
 plan = initial; plan.program_end = plan.base_address + 0x80000;
 fixup_rejected(&plan, ERANGE);
 plan = initial; plan.register_word |= 1u << 27; fixup_rejected(&plan, EINVAL);
 plan = initial; plan.register_word = (uint32_t)(plan.base_address / 128) | (3u << 25);
 fixup_rejected(&plan, ENOTSUP);
 plan = initial; plan.entry_byte_offset = 0x80000;
 plan.entry_address = plan.base_address + 0x80000; plan.program_end = plan.entry_address + 8;
 fixup_rejected(&plan, ERANGE);
 plan = initial; plan.base_address = 0xfff80000;
 plan.register_word = (uint32_t)(plan.base_address / 128) | (1u << 25);
 plan.entry_address = plan.base_address + 0x140; plan.program_end = plan.entry_address + 8;
 fixup_rejected(&plan, ERANGE);
 CHECK(remesa_use_entry_preview_fixup(NULL, &bits) == EINVAL);
 CHECK(remesa_use_entry_preview_fixup(&initial, NULL) == EINVAL);
 /* No upper control field may silently become part of an entry. */
 struct remesa_use_entry_fixup_fields decoded = {0xa5a5a5a5, 0x5a5a5a5a};
 const struct remesa_use_entry_fixup_fields before = decoded;
 for (unsigned bit = 19; bit < 32; ++bit) {
  CHECK(remesa_use_entry_decode_fixup(0x1403 | (1u << bit), &decoded) == EINVAL);
  CHECK(!memcmp(&decoded, &before, sizeof(decoded)));
 }
 CHECK(remesa_use_entry_decode_fixup(0, NULL) == EINVAL);
 union {struct remesa_use_entry_plan plan; uint32_t bits;} alias;
 alias.plan = initial;
 CHECK(!remesa_use_entry_preview_fixup(&alias.plan, &alias.bits));
 CHECK(alias.bits == 0x1403);
}

static void pair_rejected(const struct remesa_use_entry_request r[2],int expected)
{
 struct remesa_use_entry_plan out[2],before[2];
 memset(out,0xa5,sizeof(out));memcpy(before,out,sizeof(out));
 CHECK(remesa_use_entry_pair_prepare(r,out)==expected);
 CHECK(!memcmp(out,before,sizeof(out)));
}
static void test_pair(void)
{
 const struct remesa_use_entry_request initial[2]={
  {.allocation_address=0x10000000,.allocation_bytes=4096,.program_bytes=32,
   .bank=0,.data_master=REMESA_USE_VERTEX},
  {.allocation_address=0x10000000,.allocation_bytes=4096,.entry_offset=32,
   .program_bytes=8,.bank=15,.data_master=REMESA_USE_PIXEL}
 };
 struct remesa_use_entry_request r[2];memcpy(r,initial,sizeof(r));
 struct remesa_use_entry_plan out[2];
 CHECK(!remesa_use_entry_pair_prepare(r,out));
 CHECK(out[0].program_end==out[1].entry_address&&out[1].program_end==0x10000028);
 CHECK(out[0].register_offset!=out[1].register_offset&&
       out[0].register_word!=out[1].register_word);
 uint64_t base;enum remesa_use_data_master master;
 CHECK(!remesa_use_base_unpack_address(out[0].register_word,&base,&master)&&
       base==out[0].base_address&&master==REMESA_USE_VERTEX);
 CHECK(!remesa_use_base_unpack_address(out[1].register_word,&base,&master)&&
       base==out[1].base_address&&master==REMESA_USE_PIXEL);
 for(unsigned i=0;i<2;i++) {
  uint32_t bits;struct remesa_use_entry_fixup_fields decoded;
  CHECK(!remesa_use_entry_preview_fixup(&out[i],&bits));
  CHECK(!remesa_use_entry_decode_fixup(bits,&decoded)&&decoded.bank==r[i].bank&&
        out[i].base_address+decoded.entry_byte_offset==out[i].entry_address);
 }
 for(unsigned bank=0;bank<16;bank++) {
  r[0].bank=r[1].bank=bank;pair_rejected(r,EBUSY);
 }
 memcpy(r,initial,sizeof(r));r[1].bank=16;pair_rejected(r,ERANGE);
 memcpy(r,initial,sizeof(r));r[1].program_bytes=0;pair_rejected(r,EINVAL);
 memcpy(r,initial,sizeof(r));r[1].entry_offset=4096;pair_rejected(r,ERANGE);
 memcpy(r,initial,sizeof(r));r[1].allocation_address=UINT64_MAX;pair_rejected(r,ERANGE);
 memcpy(r,initial,sizeof(r));r[1].allocation_address=0xfffff000;pair_rejected(r,ERANGE);
 memcpy(r,initial,sizeof(r));r[1].data_master=REMESA_USE_VERTEX;pair_rejected(r,EINVAL);
 memcpy(r,initial,sizeof(r));r[0].data_master=REMESA_USE_PIXEL;pair_rejected(r,EINVAL);
 memcpy(r,initial,sizeof(r));r[1].entry_offset=16;pair_rejected(r,EINVAL);
 memcpy(r,initial,sizeof(r));r[1].allocation_bytes=2048;pair_rejected(r,EINVAL);
 memcpy(r,initial,sizeof(r));r[1].allocation_address+=16;pair_rejected(r,EINVAL);
 memcpy(r,initial,sizeof(r));r[1].entry_offset=33;pair_rejected(r,EINVAL);
 memcpy(r,initial,sizeof(r));r[1].allocation_address=0x10080000;
 CHECK(!remesa_use_entry_pair_prepare(r,out)&&out[1].base_address==0x10080000);
 struct remesa_use_entry_plan before[2];memcpy(before,out,sizeof(out));
 CHECK(remesa_use_entry_pair_prepare(NULL,out)==EINVAL&&!memcmp(before,out,sizeof(out)));
 CHECK(remesa_use_entry_pair_prepare(r,NULL)==EINVAL);
 CHECK(remesa_use_entry_pair_prepare((void *)(UINTPTR_MAX-15),out)==EINVAL&&
       !memcmp(before,out,sizeof(out)));
 CHECK(remesa_use_entry_pair_prepare(r,(void *)(UINTPTR_MAX-15))==EINVAL);
 union {struct remesa_use_entry_request request[2];struct remesa_use_entry_plan plan[2];} alias;
 memcpy(alias.request,initial,sizeof(initial));
 CHECK(!remesa_use_entry_pair_prepare(alias.request,alias.plan));
 CHECK(alias.plan[0].entry_address==0x10000000&&alias.plan[1].entry_address==0x10000020);
}

int main(void)
{
 test_address(); test_entry(); test_fixup();test_pair();
 printf("PASS %u USE address/entry bounds checks; zero hardware execution\n", checks);
 return 0;
}
