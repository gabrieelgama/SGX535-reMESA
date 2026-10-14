/* SPDX-License-Identifier: MIT
 * Original raw draw range contract. No encoding or GPU work. */
#ifdef NDEBUG
#error Assertions are required
#endif
#include "remesa_draw.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks;
#define CHECK(x) do { assert(x); ++checks; } while (0)
#ifdef REMESA_DRAW_SNAPSHOT_FAULT_TESTS
/* Compile with --wrap=calloc,--wrap=malloc,--wrap=free. Faults are scoped to
 * one snapshot creation; unrelated fixture/libc allocation is not counted. */
enum snapshot_fault { SNAPSHOT_FAIL_CALLOC, SNAPSHOT_FAIL_MALLOC };
static enum snapshot_fault allocation_fault;
static bool fault_enabled;
static unsigned calloc_calls, malloc_calls, free_calls, fault_live_allocations;
void *__real_calloc(size_t, size_t);
void *__real_malloc(size_t);
void __real_free(void *);
void *__wrap_calloc(size_t, size_t);
void *__wrap_malloc(size_t);
void __wrap_free(void *);
void *__wrap_calloc(size_t count, size_t bytes)
{
    if (!fault_enabled) return __real_calloc(count, bytes);
    ++calloc_calls;
    if (allocation_fault == SNAPSHOT_FAIL_CALLOC) return NULL;
    void *result = __real_calloc(count, bytes);
    if (result) ++fault_live_allocations;
    return result;
}
void *__wrap_malloc(size_t bytes)
{
    if (!fault_enabled) return __real_malloc(bytes);
    ++malloc_calls;
    if (allocation_fault == SNAPSHOT_FAIL_MALLOC) return NULL;
    void *result = __real_malloc(bytes);
    if (result) ++fault_live_allocations;
    return result;
}
void __wrap_free(void *pointer)
{
    if (fault_enabled && pointer) {
        assert(fault_live_allocations);
        --fault_live_allocations;
        ++free_calls;
    }
    __real_free(pointer);
}
static void inject_allocation_fault(enum snapshot_fault fault)
{
    assert(!fault_enabled && !fault_live_allocations);
    allocation_fault = fault;
    calloc_calls = malloc_calls = free_calls = 0;
    fault_enabled = true;
}
#endif
static void rejected(const struct remesa_shader_draw_inputs *in, int error)
{
    struct remesa_draw_plan out, before;
    memset(&out, 0xa5, sizeof(out)); before=out;
    CHECK(remesa_shader_draw_inputs_validate(in,&out)==error);
    CHECK(!memcmp(&out,&before,sizeof(out)));
}
int main(void)
{
    unsigned char vertices[128]={0};
    uint16_t indices[]={999,1,2,3};
    struct remesa_shader_draw_inputs in={
        .vertex_data=vertices,.vertex_bytes=80,.vertex_stride=24,
        .vertex_offset=4,.element_offset=4,.count=3,
        .position_format=PIPE_FORMAT_R32G32B32_FLOAT,
        .color_format=PIPE_FORMAT_B8G8R8A8_UNORM,
        .color_width=32,.color_height=32,.color_stride=136,
        .color_bytes=31*136+128,
        .viewport={.scale={16,16,.5f},.translate={16,16,.5f}}
    }, good=in, altered;
    struct remesa_draw_plan plan;
    CHECK(!remesa_shader_draw_inputs_validate(&in,&plan));
    CHECK(plan.primitive_count==1&&plan.min_vertex==0&&plan.max_vertex==2&&
          plan.vertex_end_byte==68&&!plan.index_first_byte&&!plan.index_end_byte);
    in.vertex_bytes=68;CHECK(!remesa_shader_draw_inputs_validate(&in,&plan));
    in.vertex_bytes=67;rejected(&in,EINVAL);in=good;
    in.index_data=indices;in.index_bytes=sizeof(indices);in.index_size=2;
    in.start=1;in.index_bias=-1;
    CHECK(!remesa_shader_draw_inputs_validate(&in,&plan));
    CHECK(plan.min_vertex==0&&plan.max_vertex==2&&plan.index_first_byte==2&&
          plan.index_end_byte==8&&plan.vertex_end_byte==68);
    struct remesa_shader_draw_inputs indexed=in;
    in.index_bytes=7;rejected(&in,EINVAL);in=indexed;
    in.index_bias=-2;rejected(&in,EINVAL);in=indexed;
    in.index_bias=1;rejected(&in,EINVAL);in=indexed;
    in.start=UINT32_MAX;rejected(&in,EINVAL);in=indexed;
    in.index_size=3;rejected(&in,EINVAL);in=indexed;
    in.index_data=NULL;rejected(&in,EINVAL);in=indexed;
    in.index_data=(const void *)(uintptr_t)(UINTPTR_MAX-2);rejected(&in,EOVERFLOW);
    in=indexed;
    /* Unaligned snapshots are valid host storage; memcpy performs the read. */
    unsigned char unaligned[sizeof(indices)+1];memcpy(unaligned+1,indices,sizeof(indices));
    in.index_data=unaligned+1;CHECK(!remesa_shader_draw_inputs_validate(&in,&plan));
    uint8_t byte_indices[]={1,2,3};in=indexed;in.start=0;
    in.index_size=1;in.index_data=byte_indices;in.index_bytes=sizeof(byte_indices);
    CHECK(!remesa_shader_draw_inputs_validate(&in,&plan));
    uint32_t word_indices[]={1,2,3};in.index_size=4;
    in.index_data=word_indices;in.index_bytes=sizeof(word_indices);
    CHECK(!remesa_shader_draw_inputs_validate(&in,&plan));
    word_indices[2]=UINT32_MAX;in.index_bias=1;rejected(&in,EINVAL);
    in=good;in.start=UINT32_MAX;rejected(&in,EINVAL);
    in=good;in.vertex_stride=UINT32_MAX;rejected(&in,EINVAL);
    in=good;in.vertex_offset=UINT32_MAX;in.element_offset=UINT32_MAX;rejected(&in,EINVAL);
    in=good;in.vertex_data=(const void *)(uintptr_t)(UINTPTR_MAX-2);rejected(&in,EOVERFLOW);
    in=good;in.index_data=indices;rejected(&in,EINVAL);
    in=good;in.index_bytes=1;rejected(&in,EINVAL);
    in=good;in.index_bias=1;rejected(&in,EINVAL);
    in=good;in.color_bytes--;rejected(&in,EINVAL);
    in=good;in.color_stride=127;rejected(&in,EINVAL);
    in=good;in.color_width=0;rejected(&in,EINVAL);
    in=good;in.color_height=UINT32_MAX;in.color_stride=UINT32_MAX;
    in.color_bytes=UINT32_MAX;
    rejected(&in,SIZE_MAX==UINT32_MAX?EOVERFLOW:EINVAL);
    in=good;in.scissor_enabled=true;in.scissor=(struct pipe_scissor_state){.minx=3,.miny=4,.maxx=16,.maxy=24};
    CHECK(!remesa_shader_draw_inputs_validate(&in,&plan));
    in.scissor.minx=17;rejected(&in,EINVAL);in.scissor.minx=3;
    in.scissor.maxy=33;rejected(&in,EINVAL);
    in=good;in.scissor.maxx=32;rejected(&in,EINVAL);
    in=good;in.viewport.scale[0]=NAN;rejected(&in,EINVAL);
    in=good;in.viewport.translate[2]=INFINITY;rejected(&in,EINVAL);
    in=good;in.viewport.swizzle_x=1;rejected(&in,ENOTSUP);
    in=good;in.position_format=PIPE_FORMAT_R32_UINT;rejected(&in,ENOTSUP);
    in=good;in.color_format=PIPE_FORMAT_R8G8B8A8_UNORM;rejected(&in,ENOTSUP);
    in=good;in.vertex_stride=0;rejected(&in,ENOTSUP);
    in=good;in.position_input_slot=1;rejected(&in,ENOTSUP);
    in=good;in.count=0;rejected(&in,EINVAL);
    in=good;in.count=4;rejected(&in,EINVAL);
    in=good;in.count=(REMESA_DRAW_MAX_INDICES/3+1)*3;rejected(&in,ENOTSUP);
    in=good;in.cull_face=PIPE_FACE_FRONT_AND_BACK+1;rejected(&in,ENOTSUP);
    CHECK(remesa_shader_draw_inputs_validate(NULL,&plan)==EINVAL);
    CHECK(remesa_shader_draw_inputs_validate(&good,NULL)==EINVAL);
    /* A float2 fetch omits z/w storage. No physical register semantics follow. */
    altered=good;altered.position_format=PIPE_FORMAT_R32G32_FLOAT;altered.vertex_bytes=64;
    CHECK(!remesa_shader_draw_inputs_validate(&altered,&plan)&&plan.vertex_end_byte==64);
    altered=good;altered.position_format=PIPE_FORMAT_R32G32B32A32_FLOAT;altered.vertex_bytes=72;
    CHECK(!remesa_shader_draw_inputs_validate(&altered,&plan)&&plan.vertex_end_byte==72);
    /* Original raw bytes/state survive source lease end and mutation. */
    struct remesa_shader_draw_snapshot *snapshot=NULL,*busy;
    struct remesa_shader_draw_inputs owned, saved_owned;
    struct remesa_draw_plan owned_plan, saved_plan;
    for(unsigned i=0;i<sizeof(vertices);i++)vertices[i]=(unsigned char)i;
    in=indexed;
    CHECK(!remesa_shader_draw_snapshot_create(&in,&snapshot)&&snapshot);
    CHECK(!remesa_shader_draw_snapshot_inspect(snapshot,&owned,&owned_plan));
    CHECK(owned.vertex_data!=vertices && owned.index_data!=indices);
    CHECK(owned.vertex_bytes==68 && owned.index_bytes==8);
    CHECK(!memcmp(owned.vertex_data,vertices,68)&&!memcmp(owned.index_data,indices,8));
    CHECK(owned.vertex_stride==24&&owned.start==1&&owned.index_bias==-1);
    CHECK(owned.color_stride==136&&owned.color_bytes==good.color_bytes);
    CHECK(!memcmp(&owned.viewport,&good.viewport,sizeof(good.viewport)));
    unsigned char saved_vertices[68];memcpy(saved_vertices,vertices,68);
    memset(vertices,0xcc,sizeof(vertices));memset(indices,0xcc,sizeof(indices));
    CHECK(!memcmp(owned.vertex_data,saved_vertices,68));
    CHECK(!remesa_shader_draw_inputs_validate(&owned,&plan));
    CHECK(!memcmp(&plan,&owned_plan,sizeof(plan)));
    saved_owned=owned;saved_plan=owned_plan;
    CHECK(remesa_shader_draw_snapshot_inspect(snapshot,(void *)owned.vertex_data,&plan)==EINVAL);
    CHECK(remesa_shader_draw_snapshot_inspect(snapshot,&owned,(void *)&owned)==EINVAL);
    CHECK(!memcmp(&owned,&saved_owned,sizeof(owned)));
    CHECK(!memcmp(&owned_plan,&saved_plan,sizeof(owned_plan)));
    busy=snapshot;
    CHECK(remesa_shader_draw_snapshot_create(&owned,&busy)==EBUSY&&busy==snapshot);
    CHECK(remesa_shader_draw_snapshot_create(&good,(void *)&good)==EINVAL);
    /* Re-create from an existing borrow before freeing its owner. The new
     * snapshot must then remain valid without either original source lease. */
    struct remesa_shader_draw_snapshot *replacement=NULL;
    unsigned char saved_indices[8];memcpy(saved_indices,owned.index_data,sizeof(saved_indices));
    CHECK(!remesa_shader_draw_snapshot_create(&owned,&replacement)&&replacement);
    CHECK(!remesa_shader_draw_snapshot_inspect(replacement,&owned,&owned_plan));
    CHECK(owned.vertex_data!=saved_owned.vertex_data&&owned.index_data!=saved_owned.index_data);
    CHECK(owned.vertex_stride==saved_owned.vertex_stride&&owned.start==saved_owned.start&&
          owned.index_bias==saved_owned.index_bias);
    CHECK(!memcmp(&owned_plan,&saved_plan,sizeof(owned_plan)));
    remesa_shader_draw_snapshot_destroy(snapshot);snapshot=replacement;
    CHECK(!remesa_shader_draw_snapshot_inspect(snapshot,&owned,&owned_plan));
    CHECK(!memcmp(owned.vertex_data,saved_vertices,sizeof(saved_vertices))&&
          !memcmp(owned.index_data,saved_indices,sizeof(saved_indices)));
    CHECK(!remesa_shader_draw_inputs_validate(&owned,&plan));
    CHECK(!memcmp(&plan,&owned_plan,sizeof(plan)));
#ifdef REMESA_DRAW_SNAPSHOT_FAULT_TESTS
    struct remesa_shader_draw_snapshot *failed=NULL;
    inject_allocation_fault(SNAPSHOT_FAIL_CALLOC);
    CHECK(remesa_shader_draw_snapshot_create(&owned,&failed)==ENOMEM&&!failed);
    fault_enabled=false;
    CHECK(calloc_calls==1&&!malloc_calls&&!free_calls&&!fault_live_allocations);
    CHECK(!remesa_shader_draw_snapshot_inspect(snapshot,&owned,&owned_plan));
    CHECK(!memcmp(owned.vertex_data,saved_vertices,sizeof(saved_vertices))&&
          !memcmp(owned.index_data,saved_indices,sizeof(saved_indices)));
    inject_allocation_fault(SNAPSHOT_FAIL_MALLOC);
    CHECK(remesa_shader_draw_snapshot_create(&owned,&failed)==ENOMEM&&!failed);
    fault_enabled=false;
    CHECK(calloc_calls==1&&malloc_calls==1&&free_calls==1&&!fault_live_allocations);
    CHECK(!remesa_shader_draw_snapshot_inspect(snapshot,&owned,&owned_plan));
    CHECK(!memcmp(owned.vertex_data,saved_vertices,sizeof(saved_vertices))&&
          !memcmp(owned.index_data,saved_indices,sizeof(saved_indices)));
#endif
    remesa_shader_draw_snapshot_destroy(snapshot);snapshot=NULL;
    CHECK(!remesa_shader_draw_snapshot_create(&good,&snapshot));
    CHECK(!remesa_shader_draw_snapshot_inspect(snapshot,&owned,&plan));
    CHECK(!owned.index_data&&!owned.index_bytes&&owned.vertex_bytes==68);
    remesa_shader_draw_snapshot_destroy(snapshot);snapshot=NULL;
    unsigned char *large=malloc(REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES+1);
    CHECK(large);
    memset(large,0x3b,REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES+1);
    in=good;in.vertex_data=large;in.vertex_bytes=REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES+1;
    in.vertex_offset=REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES-63;
    CHECK(remesa_shader_draw_snapshot_create(&in,&snapshot)==E2BIG&&!snapshot);
    /* Each range is individually below the cap. The combined range, including
     * the unused initial index, is exactly one MiB and then one byte larger. */
    uint16_t cap_indices[]={999,1,2,3};
    in=indexed;in.vertex_data=large;in.vertex_bytes=REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES+1;
    in.index_data=cap_indices;in.index_bytes=sizeof(cap_indices);
    in.vertex_offset=REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES-68;in.element_offset=0;
    CHECK(!remesa_shader_draw_inputs_validate(&in,&plan)&&
          plan.vertex_end_byte+plan.index_end_byte==REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES);
    CHECK(!remesa_shader_draw_snapshot_create(&in,&snapshot)&&snapshot);
    CHECK(!remesa_shader_draw_snapshot_inspect(snapshot,&owned,&owned_plan));
    CHECK(owned.vertex_bytes+owned.index_bytes==REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES&&
          owned.index_bytes==sizeof(cap_indices));
    CHECK(!memcmp(owned.vertex_data,large,owned.vertex_bytes)&&
          !memcmp(owned.index_data,cap_indices,sizeof(cap_indices)));
    remesa_shader_draw_snapshot_destroy(snapshot);snapshot=NULL;
    ++in.vertex_offset;
    CHECK(!remesa_shader_draw_inputs_validate(&in,&plan)&&
          plan.vertex_end_byte+plan.index_end_byte==REMESA_SHADER_DRAW_SNAPSHOT_MAX_BYTES+1);
    CHECK(remesa_shader_draw_snapshot_create(&in,&snapshot)==E2BIG&&!snapshot);
    free(large);
    CHECK(remesa_shader_draw_snapshot_create(NULL,&snapshot)==EINVAL);
    CHECK(remesa_shader_draw_snapshot_create(&good,NULL)==EINVAL);
    CHECK(remesa_shader_draw_snapshot_inspect(NULL,&owned,&plan)==EINVAL);
    remesa_shader_draw_snapshot_destroy(NULL);
    printf("PASS %u original raw shader draw range checks; no GPU submissions\n",checks);
    return 0;
}
