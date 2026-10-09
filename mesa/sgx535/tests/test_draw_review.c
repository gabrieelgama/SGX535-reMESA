/* SPDX-License-Identifier: MIT
 * Independent API/range edge review; does not submit or open a device. */
#include "remesa_draw.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>
static unsigned checks;
#define CHECK(e) do { assert(e); ++checks; } while(0)
int main(void)
{
    struct pipe_resource vb={.target=PIPE_BUFFER,.width0=112};
    struct pipe_resource ib={.target=PIPE_BUFFER,.width0=24};
    struct pipe_vertex_buffer vertices={.buffer.resource=&vb};
    struct pipe_vertex_element position={.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT,.src_stride=32};
    struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.index_size=4,.instance_count=1,.index.resource=&ib};
    struct pipe_draw_start_count_bias draw={.start=3,.count=3,.index_bias=-1};
    uint32_t data[]={UINT32_MAX,UINT32_MAX,UINT32_MAX,1,2,4};
    struct remesa_draw_plan plan;
#define RUN remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,data,sizeof(data),&plan)
    CHECK(RUN==REMESA_DRAW_VALIDATED);
    CHECK(plan.min_vertex==0 && plan.max_vertex==3 && plan.index_first_byte==12);
    info.index_bounds_valid=1; info.min_index=1; info.max_index=4;
    CHECK(RUN==REMESA_DRAW_VALIDATED);
    info.min_index=5; CHECK(RUN==REMESA_DRAW_INVALID);
    CHECK(plan.primitive_count==0 && plan.index_end_byte==0);
    info.index_bounds_valid=0;
    draw.start=0; draw.index_bias=0; position.src_stride=UINT32_MAX;
    vertices.buffer_offset=UINT32_MAX; position.src_offset=UINT16_MAX;
    CHECK(RUN==REMESA_DRAW_INVALID);
    position.src_stride=32; vertices.buffer_offset=0; position.src_offset=0;
    draw.start=3; draw.index_bias=-1;
    CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,data,23,&plan)==REMESA_DRAW_INVALID);
    CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,data,24,&plan)==REMESA_DRAW_VALIDATED);
    position.vertex_buffer_index=1; CHECK(RUN==REMESA_DRAW_UNSUPPORTED);
    position.vertex_buffer_index=0; position.dual_slot=1; CHECK(RUN==REMESA_DRAW_UNSUPPORTED);
    position.dual_slot=0; info.start_instance=1; CHECK(RUN==REMESA_DRAW_UNSUPPORTED);
    info.start_instance=0; info.mode=MESA_PRIM_TRIANGLE_STRIP; CHECK(RUN==REMESA_DRAW_UNSUPPORTED);
    printf("%u independent Gallium API/range checks PASS; no hardware\n",checks);
    return 0;
}
