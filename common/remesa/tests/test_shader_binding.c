/* SPDX-License-Identifier: MIT */
/* Synthetic CPU bytes test the backing contract, never instruction semantics. */
#include "remesa/shader_request.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned checks;
#define CHECK(c) do { assert(c);++checks; } while (0)
int main(void)
{
    unsigned char data[4][24] = {{0x19},{0x31},{0x45},{0x67}};
    struct remesa_shader_blob blobs[4];
    struct remesa_shader_request request;
    struct remesa_shader_bindings bindings, original, out;
    unsigned i;
    CHECK(sizeof(struct remesa_shader_binding) == 32);
    CHECK(sizeof(bindings) == REMESA_SHADER_BINDINGS_BYTES);
    CHECK(offsetof(struct remesa_shader_bindings, owner) == 16);
    CHECK(offsetof(struct remesa_shader_bindings, resources) == 32);
    for (i = 0; i < 4; ++i)
        blobs[i] = (struct remesa_shader_blob){data[i], 24, 8,
                                              REMESA_SHADER_CPU_CANDIDATE};
    CHECK(remesa_shader_request_build(&request, 12, 9, blobs) == REMESA_SHADER_REQUEST_OK);
    CHECK(remesa_shader_bindings_build(&bindings, &request, 12, 9, 32, 4096) ==
          REMESA_SHADER_REQUEST_OK);
    CHECK(remesa_shader_bindings_validate(&request, &bindings, 12, 9) == REMESA_SHADER_REQUEST_OK);
    for (i = 0; i < 4; ++i) {
        CHECK(bindings.resources[i].handle == 32 && bindings.resources[i].kind == i + 1);
        CHECK(bindings.resources[i].offset == 24 * i && bindings.resources[i].length == 24);
    }
    original = bindings;
#define REJECT(change) do { bindings=original;change; \
    CHECK(remesa_shader_bindings_validate(&request,&bindings,12,9)==REMESA_SHADER_REQUEST_INVALID); } while (0)
    REJECT(bindings.version = 0);
    REJECT(bindings.version++);
    REJECT(bindings.struct_bytes--);
    REJECT(bindings.resource_mask = 0);
    REJECT(bindings.resource_mask = 2);
    REJECT(bindings.resource_mask = 16);
    REJECT(bindings.reserved = 1);
    REJECT(bindings.owner = 13);
    REJECT(bindings.generation = 10);
    for (i = 0; i < 4; ++i) {
        REJECT(bindings.resources[i].handle = 0);
        REJECT(bindings.resources[i].kind = 0);
        REJECT(bindings.resources[i].state = REMESA_SHADER_ENCODED_UNQUALIFIED);
        REJECT(bindings.resources[i].length--);
        REJECT(bindings.resources[i].alignment = 0);
        REJECT(bindings.resources[i].offset++);
        REJECT(bindings.resources[i].offset = 0xfffffff8u);
        REJECT(bindings.resources[i].allocation_bytes = 0);
        REJECT(bindings.resources[i].reserved = 1);
    }
    REJECT(bindings.resources[1].offset = 0); /* same-object overlap */
    REJECT(bindings.resources[1].offset = 8); /* partial overlap */
    REJECT(bindings.resources[1].allocation_bytes = 8192); /* one ID, two extents */
    bindings = original;
    bindings.resources[1].handle = 64;
    bindings.resources[1].offset = 0; /* different objects may use equal offsets */
    CHECK(remesa_shader_bindings_validate(&request, &bindings, 12, 9) == REMESA_SHADER_REQUEST_OK);
    bindings = original;
    for (i = 0; i < 4; ++i) {
        bindings.resources[i].handle = 32 + i;
        bindings.resources[i].offset = 0xffffffe0u;
        bindings.resources[i].allocation_bytes = UINT32_MAX;
    }
    CHECK(remesa_shader_bindings_validate(&request, &bindings, 12, 9) == REMESA_SHADER_REQUEST_OK);
    bindings.resources[0].offset = 0xfffffff0u; /* subtraction must reject wrap */
    CHECK(remesa_shader_bindings_validate(&request, &bindings, 12, 9) == REMESA_SHADER_REQUEST_INVALID);
    out = original;
    CHECK(remesa_shader_bindings_build(&out, &request, 12, 9, 32, 95) == REMESA_SHADER_REQUEST_INVALID);
    CHECK(!memcmp(&out, &original, sizeof(out)));
    CHECK(remesa_shader_bindings_build(&out, &request, 12, 9, 0, 4096) == REMESA_SHADER_REQUEST_INVALID);
    CHECK(!memcmp(&out, &original, sizeof(out)));
    CHECK(remesa_shader_bindings_build((void *)&request, &request, 12, 9, 32, 4096) == REMESA_SHADER_REQUEST_INVALID);
    CHECK(remesa_shader_bindings_validate(&request, NULL, 12, 9) == REMESA_SHADER_REQUEST_INVALID);
    CHECK(remesa_shader_bindings_validate(NULL, &original, 12, 9) == REMESA_SHADER_REQUEST_INVALID);
    CHECK(remesa_shader_bindings_validate(&request, &original, 13, 9) == REMESA_SHADER_REQUEST_INVALID);
    CHECK(remesa_shader_bindings_validate(&request, &original, 12, 10) == REMESA_SHADER_REQUEST_INVALID);
    /* All four populated resources still grant no execution authorization. */
    request.action = REMESA_SHADER_EXECUTE;
    CHECK(remesa_shader_bindings_validate(&request, &original, 12, 9) == REMESA_SHADER_REQUEST_UNSUPPORTED);
    CHECK(remesa_shader_bindings_build(&out, &request, 12, 9, 32, 4096) == REMESA_SHADER_REQUEST_UNSUPPORTED);
    CHECK(!memcmp(&out, &original, sizeof(out)));
    memset(blobs, 0, sizeof(blobs));
    blobs[1] = (struct remesa_shader_blob){data[1], 8, 8, REMESA_SHADER_CPU_CANDIDATE};
    CHECK(remesa_shader_request_build(&request, 12, 10, blobs) == REMESA_SHADER_REQUEST_OK);
    CHECK(remesa_shader_bindings_build(&bindings, &request, 12, 10, 64, 4096) == REMESA_SHADER_REQUEST_OK);
    CHECK(bindings.resource_mask == 2 && !bindings.resources[0].handle &&
          bindings.resources[1].length == 8 && !bindings.resources[2].handle);
    bindings.resources[0].handle = 32;
    CHECK(remesa_shader_bindings_validate(&request, &bindings, 12, 10) == REMESA_SHADER_REQUEST_INVALID);
    printf("PASS: %u program backing binding/ABI checks; no ioctl or GPU\n", checks);
    return 0;
}
