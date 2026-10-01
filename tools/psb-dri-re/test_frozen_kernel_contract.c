#include "frozen_kernel_contract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(actual, expected) do { \
    int got_ = (actual); \
    if (got_ != (expected)) { \
        fprintf(stderr, "%s:%d: got %d, expected %d\n", \
                __FILE__, __LINE__, got_, (expected)); \
        return 1; \
    } \
} while (0)

static void valid_bos(struct sgx535_frozen_bo *bos)
{
    static const struct sgx535_frozen_bo expected[SGX535_BO_COUNT] = {
        {SGX535_BO_PDS, SGX535_DOMAIN_PDS, 0x20000, 0x20000000, 1},
        {SGX535_BO_USE, SGX535_DOMAIN_PDS, 0x8000, 0x20080000, 2},
        {SGX535_BO_VERTEX_TA, SGX535_DOMAIN_MMU, 0x1000, 0x40000000, 3},
        {SGX535_BO_BACKGROUND, SGX535_DOMAIN_RASTGEOM, 0x1000, 0x30000000, 4},
        {SGX535_BO_CONTROL, SGX535_DOMAIN_LOCAL, 0x1000, 0, 5},
        {SGX535_BO_COLOR, SGX535_DOMAIN_MMU, 0x1000, 0x40001000, 6},
        {SGX535_BO_SCENE_HW, SGX535_DOMAIN_MMU, 0x2000, 0x40002000, 7},
        {SGX535_BO_TA_PAGE_TABLE, SGX535_DOMAIN_MMU, 0x2000000, 0x42000000, 8},
        {SGX535_BO_TA_PARAMETER, SGX535_DOMAIN_RASTGEOM, 0x2000000, 0x31000000, 9},
        {SGX535_BO_XHW_COMM, SGX535_DOMAIN_LOCAL, 0x1000, 0, 10},
    };
    memcpy(bos, expected, sizeof(expected));
}

int main(void)
{
    static const sgx535_u32 scene_cookie[16] = {
        0, 0x10, 0x01004004, 0x01004004,
        0x10, 0x1001, 0x1f01f, 0x1000,
        0, 0x1300, 0x1350, 0x13e0,
        0, 0, 0, 0
    };
    struct sgx535_frozen_scene_info scene_info;
    struct sgx535_frozen_color_summary color_summary;
    sgx535_u8 color_bytes[4096] = {0};
    struct sgx535_frozen_ta_cookie ta_cookie;
    struct sgx535_frozen_reg_action ta_load[29];
    struct sgx535_frozen_reg_action ta_fire[31];
    struct sgx535_frozen_reg_action raster_fire[20];
    static const struct sgx535_frozen_reg_write expected_init[12] = {
        {0xca0, 0xc07c}, {0x13c, 0}, {0xa7c, 0}, {0xa80, 0},
        {0xa74, 0x05188200}, {0xa78, 0}, {0xaac, 0x44},
        {0xabc, 0xc}, {0x804, 0xffff}, {0xa00, 0x7c000},
        {0x630, 0}, {0xa58, 0}
    };
    struct sgx535_frozen_reg_write actual_init[12];
    struct sgx535_frozen_request request = {1, 1, 0, 0};
    struct sgx535_frozen_bo bos[SGX535_BO_COUNT];
    struct sgx535_frozen_session session = {SGX535_PHASE_EMPTY};
    struct sgx535_frozen_bootstrap service_boot = {0};
    struct sgx535_frozen_use_plan use_plan;
    struct sgx535_frozen_cpu_view views[SGX535_BO_COUNT] = {{0}};
    struct sgx535_frozen_command_pairs commands;
    sgx535_u32 page_counts[SGX535_BO_COUNT];
    sgx535_u32 mapped_pages[SGX535_BO_COUNT];
    size_t role;

    CHECK(sgx535_frozen_rev121_init_writes(0x00010202, actual_init, 12),
          SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_rev121_init_writes(0x00010201, actual_init, 11),
          SGX535_FROZEN_BAD_COUNT);
    CHECK(sgx535_frozen_rev121_init_writes(0x00010201, actual_init, 12),
          SGX535_FROZEN_OK);
    CHECK(memcmp(actual_init, expected_init, sizeof(expected_init)), 0);

    CHECK(sgx535_frozen_scene_info32(NULL), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_scene_info32(&scene_info), SGX535_FROZEN_OK);
    CHECK(scene_info.width, 32);
    CHECK(scene_info.height, 32);
    CHECK(scene_info.bo_size, 0x1420);
    CHECK(scene_info.clear_page_start, 0);
    CHECK(scene_info.clear_page_count, 1);
    CHECK(memcmp(scene_info.cookie, scene_cookie, sizeof(scene_cookie)), 0);
    color_bytes[(8U * 32U + 8U) * 4U] = 0xff;
    color_bytes[(9U * 32U + 9U) * 4U] = 0xff;
    CHECK(sgx535_frozen_summarize_color(color_bytes, 4095,
                                         &color_summary),
          SGX535_FROZEN_BAD_SIZE);
    CHECK(sgx535_frozen_summarize_color(color_bytes, 4096,
                                         &color_summary),
          SGX535_FROZEN_OK);
    CHECK(color_summary.nonzero_pixels, 2);
    CHECK(color_summary.row_nonzero[8], 1);
    CHECK(color_summary.row_nonzero[9], 1);
    CHECK(color_summary.row_nonzero[10], 0);
    {
        struct sgx535_frozen_xhw_bind_fire_wire wire;
        valid_bos(bos);
        CHECK(sgx535_frozen_xhw_bind_fire_wire(bos, SGX535_BO_COUNT,
              0x80000000, 0, 0, &wire), SGX535_FROZEN_OK);
        CHECK(wire.words[0], 2);
        CHECK(wire.words[4 + 7], 0x1000);
        CHECK(wire.words[20], 1);
        CHECK(wire.words[21], 0);
        CHECK(wire.words[22], 0x40002000);
        CHECK(wire.words[23], 0);
        CHECK(wire.words[24], 4);
        CHECK(wire.words[26], 0);
        CHECK(sgx535_frozen_xhw_bind_fire_wire(bos, SGX535_BO_COUNT,
              0x80000000, 1, 0, &wire), SGX535_FROZEN_OK);
        CHECK(wire.words[23], 1);
        CHECK(wire.words[24], 15);
        {
            struct sgx535_frozen_bo saved = bos[SGX535_BO_SCENE_HW];
            bos[SGX535_BO_SCENE_HW] = bos[SGX535_BO_TA_PAGE_TABLE];
            bos[SGX535_BO_TA_PAGE_TABLE] = saved;
            CHECK(sgx535_frozen_xhw_bind_fire_wire(bos, SGX535_BO_COUNT,
                  0x80000000, 1, 0, &wire), SGX535_FROZEN_OK);
            CHECK(wire.words[22], 0x40002000);
        }
        CHECK(sgx535_frozen_xhw_bind_fire_wire(bos, SGX535_BO_COUNT,
              0x80000000, 2, 0, &wire), SGX535_FROZEN_BAD_REQUEST);
        CHECK(sgx535_frozen_xhw_bind_fire_wire(bos, SGX535_BO_COUNT,
              0x80000000, 1, 1, &wire), SGX535_FROZEN_BAD_REQUEST);
    }

    for (role = 0; role <= SGX535_BO_COLOR; role++) {
        struct sgx535_frozen_requirement requirement;
        CHECK(sgx535_frozen_get_requirement(role, &requirement), 0);
        views[role].bytes = malloc(requirement.size);
        if (!views[role].bytes)
            return 1;
        views[role].length = requirement.size;
        memset(views[role].bytes, 0xa5, requirement.size);
    }
    {
        sgx535_u8 *use_bytes = views[SGX535_BO_USE].bytes;
        views[SGX535_BO_USE].bytes = views[SGX535_BO_PDS].bytes + 0x100;
        CHECK(sgx535_frozen_initialize_user_images(views), SGX535_FROZEN_ALIAS);
        CHECK(views[SGX535_BO_PDS].bytes[0], 0xa5);
        views[SGX535_BO_USE].bytes = use_bytes;
    }
    CHECK(sgx535_frozen_initialize_user_images(views), SGX535_FROZEN_OK);
    CHECK(views[SGX535_BO_PDS].bytes[0x190], 0x45);
    CHECK(views[SGX535_BO_PDS].bytes[0x193], 0x07);
    CHECK(views[SGX535_BO_PDS].bytes[0x197], 0xaf);
    CHECK(views[SGX535_BO_PDS].bytes[0x160 + 0x08], 0);
    CHECK(views[SGX535_BO_COLOR].bytes[0], 0);
    CHECK(sgx535_frozen_extract_commands(&views[SGX535_BO_CONTROL],
                                          &commands), SGX535_FROZEN_OK);
    CHECK(commands.raster[0], 0x400);
    CHECK(commands.raster[50], 0xa64);
    CHECK(commands.ta[0], 0x204);
    CHECK(commands.ta[12], 0x238);
    {
        struct sgx535_frozen_reg_action ta_steps[8];
        CHECK(sgx535_frozen_ta_schedule_plan(&commands, ta_steps, 8),
              SGX535_FROZEN_OK);
        CHECK(ta_steps[0].offset, 0x204);
        CHECK(ta_steps[6].offset, 0x238);
        CHECK(ta_steps[7].kind, SGX535_REG_WMB);
        CHECK(sgx535_frozen_ta_schedule_plan(&commands, ta_steps, 7),
              SGX535_FROZEN_BAD_REQUEST);
        commands.ta[0] = 0x80;
        CHECK(sgx535_frozen_ta_schedule_plan(&commands, ta_steps, 8),
              SGX535_FROZEN_BAD_REGISTER_LIST);
        commands.ta[0] = 0x204;
    }
    {
        struct sgx535_frozen_reg_action raster_steps[29];
        CHECK(sgx535_frozen_raster_schedule_plan(&commands, raster_steps, 29),
              SGX535_FROZEN_OK);
        CHECK(raster_steps[0].kind, SGX535_REG_WRITE);
        CHECK(raster_steps[0].offset, 0x80);
        CHECK(raster_steps[0].value, 0x20);
        CHECK(raster_steps[1].offset, 0x80);
        CHECK(raster_steps[1].value, 0);
        CHECK(raster_steps[2].offset, 0x400);
        CHECK(raster_steps[2].value == commands.raster[1], 1);
        CHECK(raster_steps[27].offset, 0xa64);
        CHECK(raster_steps[28].kind, SGX535_REG_WMB);
        CHECK(sgx535_frozen_raster_schedule_plan(&commands, raster_steps, 28),
              SGX535_FROZEN_BAD_REQUEST);
        commands.raster[0] = 0x80;
        CHECK(sgx535_frozen_raster_schedule_plan(&commands, raster_steps, 29),
              SGX535_FROZEN_BAD_REGISTER_LIST);
        commands.raster[0] = 0x400;
    }
    views[SGX535_BO_CONTROL].bytes[0] = 0xff;
    CHECK(sgx535_frozen_extract_commands(&views[SGX535_BO_CONTROL],
                                          &commands),
          SGX535_FROZEN_BAD_REGISTER_LIST);
    views[SGX535_BO_CONTROL].bytes[0] = 0;
    for (role = 0; role <= SGX535_BO_COLOR; role++)
        free(views[role].bytes);

    CHECK(sgx535_frozen_validate_request(&request, sizeof(request)), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_validate_request(NULL, sizeof(request)), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_validate_request(&request, sizeof(request) - 1), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_validate_request(&request, sizeof(request) + 1), SGX535_FROZEN_BAD_REQUEST);
    request.abi_version = 2;
    CHECK(sgx535_frozen_validate_request(&request, sizeof(request)), SGX535_FROZEN_BAD_REQUEST);
    request.abi_version = 1;
    request.operation = 2;
    CHECK(sgx535_frozen_validate_request(&request, sizeof(request)), SGX535_FROZEN_BAD_REQUEST);
    request.operation = 1;
    request.flags = 1;
    CHECK(sgx535_frozen_validate_request(&request, sizeof(request)), SGX535_FROZEN_BAD_REQUEST);
    request.flags = 0;
    request.reserved = 1;
    CHECK(sgx535_frozen_validate_request(&request, sizeof(request)), SGX535_FROZEN_BAD_REQUEST);

    valid_bos(bos);
    CHECK(sgx535_frozen_ta_cookie(bos, SGX535_BO_COUNT, 0x80000000,
                                  &ta_cookie), SGX535_FROZEN_OK);
    CHECK(ta_cookie.info_bo_size, 0x620000);
    CHECK(ta_cookie.words[0], 0x42000000);
    CHECK(ta_cookie.words[1], 0x01000000);
    CHECK(ta_cookie.words[2], 0x2000);
    CHECK(ta_cookie.words[3], 0x1a00);
    CHECK(ta_cookie.words[6], 0x19c0);
    CHECK(ta_cookie.words[10], 0x1000);
    CHECK(ta_cookie.words[11], 0x2fff);
    CHECK(ta_cookie.words[12], 0x2ffe);
    CHECK(sgx535_frozen_ta_load_plan(bos, SGX535_BO_COUNT, 0x80000000,
                                     ta_load, 28), SGX535_FROZEN_BAD_COUNT);
    CHECK(sgx535_frozen_ta_load_plan(bos, SGX535_BO_COUNT, 0x80000000,
                                     ta_load, 29), SGX535_FROZEN_OK);
    CHECK(ta_load[0].kind, SGX535_REG_WRITE);
    CHECK(ta_load[0].offset, 0x618);
    CHECK(ta_load[0].value, 0x42000000);
    CHECK(ta_load[1].offset, 0x61c);
    CHECK(ta_load[1].value, 0x2fff1000);
    CHECK(ta_load[8].offset, 0x684);
    CHECK(ta_load[8].value, 1);
    CHECK(ta_load[13].offset, 0x680);
    CHECK(ta_load[17].offset, 0x688);
    CHECK(ta_load[21].offset, 0x690);
    CHECK(ta_load[22].kind, SGX535_REG_POLL_SET);
    CHECK(ta_load[22].offset, 0x118);
    CHECK(ta_load[22].mask, 7);
    CHECK(ta_load[23].kind, SGX535_REG_WRITE);
    CHECK(ta_load[23].offset, 0x114);
    CHECK(ta_load[24].kind, SGX535_REG_POLL_CLEAR);
    CHECK(ta_load[25].offset, 0x6a8);
    CHECK(ta_load[25].value, 1);
    CHECK(ta_load[26].kind, SGX535_REG_POLL_SET);
    CHECK(ta_load[26].offset, 0x12c);
    CHECK(ta_load[26].mask, 0x400000);
    CHECK(ta_load[27].offset, 0x134);
    CHECK(ta_load[28].kind, SGX535_REG_POLL_CLEAR);
    CHECK(sgx535_frozen_ta_fire_plan(bos, SGX535_BO_COUNT, 0x80000000,
                                     ta_fire, 30), SGX535_FROZEN_BAD_COUNT);
    CHECK(sgx535_frozen_ta_fire_plan(bos, SGX535_BO_COUNT, 0x80000000,
                                     ta_fire, 31), SGX535_FROZEN_OK);
    CHECK(ta_fire[0].offset, 0x220);
    CHECK(ta_fire[0].value, 0x40002000);
    CHECK(ta_fire[6].offset, 0x208);
    CHECK(ta_fire[6].value, 0x01004004);
    CHECK(ta_fire[11].offset, 0x21c);
    CHECK(ta_fire[11].value, 0x40003000);
    CHECK(ta_fire[13].kind, SGX535_REG_POLL_SET);
    CHECK(ta_fire[13].mask, 0x100a40);
    CHECK(ta_fire[16].offset, 0xc90);
    CHECK(ta_fire[16].value, 0x30000000);
    CHECK(ta_fire[18].offset, 0x138);
    CHECK(ta_fire[18].mask, 0x44);
    CHECK(ta_fire[25].offset, 0x804);
    CHECK(ta_fire[25].value, 0x1000ffff);
    CHECK(ta_fire[26].mask, 0x4000000);
    CHECK(ta_fire[29].offset, 0xa08);
    CHECK(ta_fire[30].offset, 0x200);
    CHECK(ta_fire[30].value, 1);
    CHECK(sgx535_frozen_raster_fire_plan(bos, SGX535_BO_COUNT,
                                         0x80000000, raster_fire, 19),
          SGX535_FROZEN_BAD_COUNT);
    CHECK(sgx535_frozen_raster_fire_plan(bos, SGX535_BO_COUNT,
                                         0x80000000, raster_fire, 20),
          SGX535_FROZEN_OK);
    CHECK(raster_fire[0].offset, 0x630);
    CHECK(raster_fire[0].value, 3);
    CHECK(raster_fire[4].offset, 0x408);
    CHECK(raster_fire[4].value, 0x40003000);
    CHECK(raster_fire[5].offset, 0xad4);
    CHECK(raster_fire[6].kind, SGX535_REG_POLL_SET);
    CHECK(raster_fire[6].mask, 0x44);
    CHECK(raster_fire[13].offset, 0x804);
    CHECK(raster_fire[13].value, 0x1000ffff);
    CHECK(raster_fire[17].offset, 0x43c);
    CHECK(raster_fire[18].offset, 0xa08);
    CHECK(raster_fire[19].offset, 0x428);
    bos[SGX535_BO_TA_PARAMETER].gpu_va = 0x31001000;
    CHECK(sgx535_frozen_ta_cookie(bos, SGX535_BO_COUNT, 0x80000000,
                                  &ta_cookie), SGX535_FROZEN_BAD_ALIGNMENT);
    valid_bos(bos);
    CHECK(sgx535_frozen_bootstrap_begin(&service_boot, 0x00010201), 0);
    CHECK(sgx535_frozen_bootstrap_observe(&service_boot, SGX535_BOOT_INIT_WRITES_RETURN, 0), 0);
    CHECK(sgx535_frozen_bootstrap_observe(&service_boot, SGX535_BOOT_XHW_INIT_REPLY, 0), 0);
    CHECK(sgx535_frozen_bootstrap_observe(&service_boot, SGX535_BOOT_TA_INFO_REPLY, 0), 0);
    CHECK(sgx535_frozen_bootstrap_observe(&service_boot, SGX535_BOOT_SCENE_INFO_REPLY, 0), 0);
    CHECK(sgx535_frozen_bootstrap_observe(&service_boot, SGX535_BOOT_LOAD_KICKS, 0x1f), 0);
    CHECK(sgx535_frozen_bootstrap_observe(&service_boot, SGX535_BOOT_LOAD_STATUS2, 7), 0);
    CHECK(sgx535_frozen_bootstrap_observe(&service_boot, SGX535_BOOT_INITEND_STATUS, 0x400000), 0);
    CHECK(sgx535_frozen_bootstrap_observe(&service_boot, SGX535_BOOT_TA_LOAD_REPLY, 0), 0);
    CHECK(sgx535_frozen_bootstrap_observe(&service_boot, SGX535_BOOT_SCENE_VALIDATED, 0), 0);
    for (role = 0; role < SGX535_BO_COUNT; role++) {
        page_counts[role] = bos[role].size >> 12;
        mapped_pages[role] = bos[role].domain == SGX535_DOMAIN_LOCAL
            ? 0 : page_counts[role];
    }
    CHECK(sgx535_frozen_validate_publication_pages(bos, SGX535_BO_COUNT,
            0x80000000, page_counts, mapped_pages), SGX535_FROZEN_OK);
    mapped_pages[SGX535_BO_USE]--;
    CHECK(sgx535_frozen_validate_publication_pages(bos, SGX535_BO_COUNT,
            0x80000000, page_counts, mapped_pages), SGX535_FROZEN_BAD_OWNER);
    mapped_pages[SGX535_BO_USE]++;
    mapped_pages[SGX535_BO_CONTROL] = 1;
    CHECK(sgx535_frozen_validate_publication_pages(bos, SGX535_BO_COUNT,
            0x80000000, page_counts, mapped_pages), SGX535_FROZEN_BAD_OWNER);
    mapped_pages[SGX535_BO_CONTROL] = 0;
    CHECK(sgx535_frozen_plan_use_bases(bos, SGX535_BO_COUNT,
                                       0x80000000, &use_plan), 0);
    CHECK(use_plan.assigned_count, 2);
    CHECK(use_plan.by_data_master[1].reg, 3);
    CHECK(use_plan.by_data_master[0].reg, 4);
    CHECK(use_plan.by_data_master[1].base, 0x20080000);
    CHECK(use_plan.by_data_master[0].base, 0x20080000);
    CHECK(use_plan.by_data_master[1].register_offset, 0x0a18);
    CHECK(use_plan.by_data_master[0].register_offset, 0x0a1c);
    CHECK(use_plan.by_data_master[1].register_word, 0x02401000);
    CHECK(use_plan.by_data_master[0].register_word, 0x00401000);
    bos[1].gpu_va = 0x2007ffff;
    CHECK(sgx535_frozen_plan_use_bases(bos, SGX535_BO_COUNT,
                                       0x80000000, &use_plan),
          SGX535_FROZEN_BAD_ALIGNMENT);
    valid_bos(bos);
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_validate_bos(NULL, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_COUNT);
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT - 1, 0x80000000), SGX535_FROZEN_BAD_COUNT);
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0), SGX535_FROZEN_BAD_MMU_END);
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000001), SGX535_FROZEN_BAD_MMU_END);

    valid_bos(bos); bos[0].size--;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_SIZE);
    valid_bos(bos); bos[0].domain = SGX535_DOMAIN_MMU;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_DOMAIN);
    valid_bos(bos); bos[1].gpu_va++;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_ALIGNMENT);
    valid_bos(bos); bos[0].gpu_va = 0x40000000;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_ADDRESS);
    valid_bos(bos); bos[4].gpu_va = 0x1000;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_ADDRESS);
    valid_bos(bos); bos[7].gpu_va = 0x7ff00000;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_ADDRESS);
    valid_bos(bos); bos[7].gpu_va = 0x100000000ULL;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_ADDRESS);
    valid_bos(bos); bos[5].gpu_va = bos[2].gpu_va;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_ALIAS);
    valid_bos(bos); bos[5].owner_token = bos[2].owner_token;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_OWNER);
    valid_bos(bos); bos[5].owner_token = 0;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_OWNER);
    valid_bos(bos); bos[5].role = SGX535_BO_COUNT;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_ROLE);
    valid_bos(bos); bos[5].role = SGX535_BO_PDS;
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_ROLE);

    valid_bos(bos);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_FIRE_POSSIBLE), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_begin(&session, &request, sizeof(request), bos,
                                      SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_REQUEST);
    request.reserved = 0;
    CHECK(sgx535_frozen_session_begin(&session, &request, sizeof(request), bos,
                                      SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_OK);
    CHECK(session.phase, SGX535_PHASE_BOS_VALIDATED);
    CHECK(sgx535_frozen_session_begin(&session, &request, sizeof(request), bos,
                                      SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_FIRE_POSSIBLE), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_HOST_IMAGE_FINAL), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_SERVICE_READY), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_CPU_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_TRANSLATIONS_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_DEVICE_MAINTAINED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_FIRE_POSSIBLE), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_SERVICE_READY), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_service_ready(&session, &service_boot, 0), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_service_ready(&session, &service_boot, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_enter_fire(&session, 0, 1), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_enter_fire(&session, 42, 0), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_enter_fire(&session, 42, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_enter_fire(&session, 43, 1), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_RETIRED), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_observe(&session, 41, SGX535_EVENT_TA_FINISHED), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_observe(&session, 42, SGX535_EVENT_PIXELBE_END_RENDER), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_begin_raster(&session, 42), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_observe(&session, 42, SGX535_EVENT_TA_FINISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_observe(&session, 42, SGX535_EVENT_DPM_3D_MEM_FREE), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_begin_raster(&session, 41), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_begin_raster(&session, 42), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_begin_raster(&session, 42), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_observe(&session, 42, SGX535_EVENT_DPM_3D_MEM_FREE), SGX535_FROZEN_OK);
    CHECK(session.phase, SGX535_PHASE_FIRE_POSSIBLE);
    CHECK(sgx535_frozen_session_observe(&session, 42, SGX535_EVENT_PIXELBE_END_RENDER), SGX535_FROZEN_OK);
    CHECK(session.phase, SGX535_PHASE_SCENE_COMPLETED);
    CHECK(sgx535_frozen_session_retire(&session), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_HOST_IMAGE_FINAL), SGX535_FROZEN_BAD_REQUEST);

    session.phase = SGX535_PHASE_EMPTY;
    CHECK(sgx535_frozen_session_begin(&session, &request, sizeof(request), bos,
                                      SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_abort(&session), SGX535_FROZEN_OK);
    CHECK(session.phase, SGX535_PHASE_ABORTED_BEFORE_SUBMIT);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_HOST_IMAGE_FINAL), SGX535_FROZEN_BAD_REQUEST);

    session.phase = SGX535_PHASE_EMPTY;
    CHECK(sgx535_frozen_session_begin(&session, &request, sizeof(request), bos,
                                      SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_HOST_IMAGE_FINAL), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_CPU_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_TRANSLATIONS_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_DEVICE_MAINTAINED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_service_ready(&session, &service_boot, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_enter_fire(&session, 99, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_timeout(&session), SGX535_FROZEN_OK);
    CHECK(session.phase, SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(sgx535_frozen_session_enter_fire(&session, 100, 1), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_observe(&session, 99, SGX535_EVENT_TA_FINISHED), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_abort(&session), SGX535_FROZEN_BAD_REQUEST);
    CHECK(session.phase, SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_RETIRED), SGX535_FROZEN_BAD_REQUEST);

    session.phase = SGX535_PHASE_EMPTY;
    CHECK(sgx535_frozen_session_begin(&session, &request, sizeof(request), bos,
                                      SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_HOST_IMAGE_FINAL), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_CPU_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_TRANSLATIONS_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_DEVICE_MAINTAINED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_service_ready(&session, &service_boot, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_enter_fire(&session, 100, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_observe_status(&session, 100, 1U << 24, 0, 1), SGX535_FROZEN_OK);
    CHECK(session.phase, SGX535_PHASE_FIRE_POSSIBLE);
    CHECK(session.ta_memory_free_seen, 1U);
    {
        struct sgx535_frozen_session repeated_memory_free = session;
        CHECK(sgx535_frozen_session_observe_status(&repeated_memory_free,
              100, 1U << 24, 0, 1), SGX535_FROZEN_BAD_REQUEST);
        CHECK(repeated_memory_free.phase, SGX535_PHASE_HELD_AFTER_FAILURE);
    }
    CHECK(sgx535_frozen_session_observe_status(&session, 100, 1U << 13, 0, 0), SGX535_FROZEN_BAD_REQUEST);
    CHECK(sgx535_frozen_session_observe_status(&session, 100, 1U << 13, 0, 1), SGX535_FROZEN_OK);
    {
        struct sgx535_frozen_session duplicate = session;
        CHECK(sgx535_frozen_session_observe_status(&duplicate, 100,
              1U << 13, 0, 1), SGX535_FROZEN_BAD_REQUEST);
        CHECK(duplicate.phase, SGX535_PHASE_HELD_AFTER_FAILURE);
    }
    CHECK(sgx535_frozen_session_begin_raster(&session, 100), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_observe_status(&session, 100, (1U << 18) | 1U, 0, 1), SGX535_FROZEN_OK);
    CHECK(session.phase, SGX535_PHASE_SCENE_COMPLETED);

    session.phase = SGX535_PHASE_EMPTY;
    CHECK(sgx535_frozen_session_begin(&session, &request, sizeof(request), bos,
                                      SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_HOST_IMAGE_FINAL), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_CPU_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_TRANSLATIONS_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_DEVICE_MAINTAINED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_service_ready(&session, &service_boot, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_enter_fire(&session, 101, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_observe_status(&session, 101, 1U << 18, 0, 1), SGX535_FROZEN_BAD_REQUEST);
    CHECK(session.phase, SGX535_PHASE_HELD_AFTER_FAILURE);

    session.phase = SGX535_PHASE_EMPTY;
    CHECK(sgx535_frozen_session_begin(&session, &request, sizeof(request), bos,
                                      SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_HOST_IMAGE_FINAL), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_CPU_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_TRANSLATIONS_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_DEVICE_MAINTAINED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_service_ready(&session, &service_boot, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_enter_fire(&session, 100, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_observe_status(&session, 100, 0, 1U << 4, 1), SGX535_FROZEN_OK);
    CHECK(session.phase, SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(sgx535_frozen_session_retire(&session), SGX535_FROZEN_BAD_REQUEST);

    session.phase = SGX535_PHASE_EMPTY;
    CHECK(sgx535_frozen_session_begin(&session, &request, sizeof(request), bos,
                                      SGX535_BO_COUNT, 0x80000000), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_HOST_IMAGE_FINAL), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_CPU_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_TRANSLATIONS_PUBLISHED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_advance(&session, SGX535_PHASE_DEVICE_MAINTAINED), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_service_ready(&session, &service_boot, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_enter_fire(&session, 102, 1), SGX535_FROZEN_OK);
    CHECK(sgx535_frozen_session_observe_status(&session, 102,
          (1U << 13) | (1U << 23), 0, 1), SGX535_FROZEN_BAD_REQUEST);
    CHECK(session.phase, SGX535_PHASE_HELD_AFTER_FAILURE);

    {
        struct sgx535_frozen_bootstrap boot = {0};
        CHECK(sgx535_frozen_bootstrap_begin(&boot, 0x00010202), SGX535_FROZEN_BAD_REQUEST);
        CHECK(sgx535_frozen_bootstrap_begin(&boot, 0x00010201), SGX535_FROZEN_OK);
        CHECK(sgx535_frozen_bootstrap_ready(&boot), 0);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_TA_INFO_REPLY, 0),
              SGX535_FROZEN_BAD_REQUEST);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_INIT_WRITES_RETURN, 0), 0);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_XHW_INIT_REPLY, 0), 0);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_TA_INFO_REPLY, 0), 0);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_SCENE_INFO_REPLY, 0), 0);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_LOAD_KICKS, 0x1e),
              SGX535_FROZEN_BAD_REQUEST);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_LOAD_KICKS, 0x1f), 0);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_LOAD_STATUS2, 3),
              SGX535_FROZEN_BAD_REQUEST);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_LOAD_STATUS2, 7), 0);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_INITEND_STATUS, 0),
              SGX535_FROZEN_BAD_REQUEST);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_INITEND_STATUS, 0x400000), 0);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_TA_LOAD_REPLY, 0), 0);
        CHECK(sgx535_frozen_bootstrap_ready(&boot), 0);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_SCENE_VALIDATED, 0), 0);
        CHECK(sgx535_frozen_bootstrap_ready(&boot), 1);
        CHECK(sgx535_frozen_bootstrap_observe(&boot, SGX535_BOOT_SCENE_VALIDATED, 0),
              SGX535_FROZEN_BAD_REQUEST);
    }

    return 0;
}
