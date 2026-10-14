/* SPDX-License-Identifier: MIT */
#include "remesa/shader_request.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef REMESA_SHADER_TEST_ALLOC
static int fail_allocation;
void *__real_calloc(size_t count, size_t bytes);
void *__wrap_calloc(size_t count, size_t bytes);
void *__wrap_calloc(size_t count, size_t bytes)
{
    return fail_allocation ? NULL : __real_calloc(count, bytes);
}
#endif

static unsigned checks;
static void check(int condition) { ++checks; assert(condition); }
static int valid(const struct remesa_shader_request *r)
{
    return remesa_shader_request_validate(r, 19, 7) == REMESA_SHADER_REQUEST_OK;
}

int main(void);
int main(void)
{
    /* Original synthetic CPU data, deliberately not an SGX instruction stream. */
    unsigned char vertex[13], fragment[56], pds_vertex[8], pds_fragment[12];
    unsigned char largest[REMESA_SHADER_RESOURCE_BYTES];
    struct remesa_shader_blob blobs[REMESA_SHADER_RESOURCE_COUNT] = {{0}};
    struct remesa_shader_request request, original, out;
    unsigned i;

    check(sizeof(struct remesa_shader_resource) == 32);
    check(sizeof(struct remesa_shader_request) == REMESA_SHADER_REQUEST_BYTES);
    check(offsetof(struct remesa_shader_request, owner) == 16);
    check(offsetof(struct remesa_shader_request, generation) == 24);
    check(offsetof(struct remesa_shader_request, resources) == 64);
    check(offsetof(struct remesa_shader_request, data) == 192);
    for (i = 0; i < sizeof(vertex); ++i) vertex[i] = (unsigned char)(i + 1);
    for (i = 0; i < sizeof(fragment); ++i) fragment[i] = (unsigned char)(i + 51);
    memset(pds_vertex, 0x31, sizeof(pds_vertex));
    memset(pds_fragment, 0x62, sizeof(pds_fragment));
    memset(largest, 0x93, sizeof(largest));

    /* A fragment research body is allowed as incomplete CPU storage. */
    blobs[1] = (struct remesa_shader_blob){fragment, sizeof(fragment), 8,
                                          REMESA_SHADER_CPU_CANDIDATE};
    check(remesa_shader_request_build(&request, 19, 7, blobs) ==
          REMESA_SHADER_REQUEST_OK);
    check(valid(&request));
    check(request.resource_mask == 2 && request.byte_count == 56);
    check(request.resources[1].kind == REMESA_SHADER_FRAGMENT_USSE &&
          request.resources[1].state == REMESA_SHADER_CPU_CANDIDATE);
    check(!memcmp(request.data, fragment, sizeof(fragment)));
    original = request;
    fragment[0] ^= 0xff;
    check(request.data[0] == original.data[0]);
    fragment[0] ^= 0xff;
    request.action = REMESA_SHADER_EXECUTE;
    check(remesa_shader_request_validate(&request, 19, 7) ==
          REMESA_SHADER_REQUEST_UNSUPPORTED);
    request = original;

    /* Mixed source classifications are stored distinctly, never qualified. */
    blobs[0] = (struct remesa_shader_blob){vertex, sizeof(vertex), 4,
                                          REMESA_SHADER_ENCODED_UNQUALIFIED};
    blobs[2] = (struct remesa_shader_blob){pds_vertex, sizeof(pds_vertex), 16,
                                          REMESA_SHADER_ENCODED_UNQUALIFIED};
    blobs[3] = (struct remesa_shader_blob){pds_fragment, sizeof(pds_fragment), 4,
                                          REMESA_SHADER_CPU_CANDIDATE};
    check(remesa_shader_request_build(&request, 19, 7, blobs) ==
          REMESA_SHADER_REQUEST_OK);
    check(valid(&request));
    check(request.resource_mask == 15 && request.byte_count == 100);
    check(request.resources[0].offset == 0 && request.resources[1].offset == 16 &&
          request.resources[2].offset == 80 && request.resources[3].offset == 88);
    check(!memcmp(request.data, vertex, sizeof(vertex)));
    check(!memcmp(request.data + 16, fragment, sizeof(fragment)));
    check(!memcmp(request.data + 80, pds_vertex, sizeof(pds_vertex)));
    check(!memcmp(request.data + 88, pds_fragment, sizeof(pds_fragment)));
    original = request;

#define REJECT(change) do { request = original; change; check(!valid(&request)); } while (0)
    REJECT(request.version = 0);
    REJECT(request.version = REMESA_SHADER_ABI_VERSION + 1);
    REJECT(request.struct_bytes--);
    REJECT(request.struct_bytes++);
    REJECT(request.flags = 1);
    REJECT(request.owner = 20);
    REJECT(request.generation = 8);
    REJECT(request.owner = 0);
    REJECT(request.generation = 0);
    for (i = 0; i < 6; ++i) REJECT(request.reserved[i] = 1);
    REJECT(request.action = 0);
    REJECT(request.action = 3);
    REJECT(request.byte_count = 0);
    REJECT(request.byte_count = REMESA_SHADER_DATA_BYTES + 1);
    REJECT(request.byte_count = 99);
    REJECT(request.byte_count = 101);
    REJECT(request.resource_mask = 0);
    REJECT(request.resource_mask = 16);
    REJECT(request.resource_mask = 14); /* stale descriptor in absent slot */
    REJECT(request.resources[0].kind = REMESA_SHADER_FRAGMENT_USSE);
    REJECT(request.resources[0].state = 0);
    REJECT(request.resources[0].state = 3); /* no "qualified" state */
    REJECT(request.resources[0].length = 0);
    REJECT(request.resources[0].length = REMESA_SHADER_RESOURCE_BYTES + 1);
    REJECT(request.resources[0].length = 0xffffffffu);
    REJECT(request.resources[0].alignment = 0);
    REJECT(request.resources[0].alignment = 3);
    REJECT(request.resources[0].alignment = REMESA_SHADER_MAX_ALIGNMENT * 2);
    REJECT(request.resources[0].offset = 1);
    REJECT(request.resources[0].offset = 0xfffffffcu);
    REJECT(request.resources[1].offset = 8); /* overlap */
    REJECT(request.resources[1].offset = 24); /* later overlap */
    for (i = 0; i < 3; ++i) REJECT(request.resources[1].reserved[i] = 1);
    REJECT(request.data[13] = 1); /* inter-resource padding */
    REJECT(request.data[72] = 1);
    REJECT(request.data[100] = 1); /* unused tail */
    REJECT(request.data[REMESA_SHADER_DATA_BYTES - 1] = 1);
    request = original;
    check(remesa_shader_request_validate(NULL, 19, 7) ==
          REMESA_SHADER_REQUEST_INVALID);
    check(remesa_shader_request_validate(&request, 0, 7) ==
          REMESA_SHADER_REQUEST_INVALID);
    check(remesa_shader_request_validate(&request, 19, 0) ==
          REMESA_SHADER_REQUEST_INVALID);
    request.action = REMESA_SHADER_EXECUTE;
    check(remesa_shader_request_validate(&request, 19, 7) ==
          REMESA_SHADER_REQUEST_UNSUPPORTED);

    /* Exact per-resource/total host limits and overflow-safe bounds. */
    memset(blobs, 0, sizeof(blobs));
    blobs[0] = (struct remesa_shader_blob){largest, sizeof(largest), 2048,
                                          REMESA_SHADER_CPU_CANDIDATE};
    blobs[1] = blobs[0];
    check(remesa_shader_request_build(&request, 19, 7, blobs) ==
          REMESA_SHADER_REQUEST_OK);
    check(request.byte_count == REMESA_SHADER_DATA_BYTES && valid(&request));
    original = request;
    memset(&out, 0x86, sizeof(out));
    request = out;
#define BUILD_FAIL() do { check(remesa_shader_request_build(&out, 19, 7, blobs) != \
        REMESA_SHADER_REQUEST_OK); check(!memcmp(&out, &request, sizeof(out))); } while (0)
    blobs[1].alignment = 4096;
    BUILD_FAIL();
    blobs[1].alignment = 2048;
    blobs[2] = (struct remesa_shader_blob){vertex, 1, 1,
                                          REMESA_SHADER_CPU_CANDIDATE};
    BUILD_FAIL();
    memset(&blobs[2], 0, sizeof(blobs[2]));
    blobs[0].length = 0xffffffffu;
    BUILD_FAIL();
    blobs[0].length = sizeof(largest);
    blobs[0].bytes = NULL;
    BUILD_FAIL();
    blobs[0].bytes = largest;
    blobs[0].state = 3;
    BUILD_FAIL();
    blobs[0].state = REMESA_SHADER_CPU_CANDIDATE;
    blobs[0].alignment = 3;
    BUILD_FAIL();
    blobs[0].alignment = 2048;
    blobs[0].bytes = out.data;
    BUILD_FAIL();
    memset(blobs, 0, sizeof(blobs));
    BUILD_FAIL();
    blobs[0].bytes = vertex;
    BUILD_FAIL(); /* zero length with a live pointer */
    blobs[0] = (struct remesa_shader_blob){vertex, sizeof(vertex), 4,
                                          REMESA_SHADER_CPU_CANDIDATE};
    check(remesa_shader_request_build(NULL, 19, 7, blobs) ==
          REMESA_SHADER_REQUEST_INVALID);
    check(remesa_shader_request_build(&out, 19, 7, NULL) ==
          REMESA_SHADER_REQUEST_INVALID);
    check(remesa_shader_request_build(&out, 0, 7, blobs) ==
          REMESA_SHADER_REQUEST_INVALID);
    check(remesa_shader_request_build(&out, 19, 0, blobs) ==
          REMESA_SHADER_REQUEST_INVALID);
    check(!memcmp(&out, &request, sizeof(out)));
    check(remesa_shader_request_build(&out, 19, 7,
          (const struct remesa_shader_blob *)(const void *)&out) ==
          REMESA_SHADER_REQUEST_INVALID);
    check(!memcmp(&out, &request, sizeof(out)));
#ifdef REMESA_SHADER_TEST_ALLOC
    fail_allocation = 1;
    check(remesa_shader_request_build(&out, 19, 7, blobs) ==
          REMESA_SHADER_REQUEST_NOMEM);
    check(!memcmp(&out, &request, sizeof(out)));
    fail_allocation = 0;
#endif
    check(valid(&original));

    /* Alignment is relative to separately prepared host storage, not a PC. */
    for (i = 1; i <= REMESA_SHADER_MAX_ALIGNMENT; i *= 2) {
        blobs[0].alignment = i;
        check(remesa_shader_request_build(&request, 19, 7, blobs) ==
              REMESA_SHADER_REQUEST_OK);
        check(valid(&request));
    }
    printf("PASS %u original shader transport bounds/ownership/ABI checks; no ioctl or GPU\n", checks);
    return 0;
}
