/* SPDX-License-Identifier: MIT
 * Independent semantic snapshot review; no command stream or device access.
 */
#include "remesa/draw_packet.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do { assert(x); ++checks; } while (0)
int main(void)
{
    static struct remesa_draw_packet out, saved;
    static float positions[REMESA_DRAW_MAX_VERTICES][4];
    static uint16_t indices[REMESA_DRAW_MAX_INDICES];
    for (unsigned i=0; i<REMESA_DRAW_MAX_VERTICES; ++i) {
        positions[i][0]=(float)i;
        positions[i][1]=-(float)i; /* Offscreen XY is not clipping support. */
        positions[i][2]=.5f; positions[i][3]=1;
    }
    for (unsigned i=0; i<REMESA_DRAW_MAX_INDICES; ++i)
        indices[i]=(uint16_t)(i%REMESA_DRAW_MAX_VERTICES);
    CHECK(remesa_draw_packet_build(&out,(const float (*)[4])positions,
        REMESA_DRAW_MAX_VERTICES,indices,REMESA_DRAW_MAX_INDICES,0xff00ff00)==REMESA_PACKET_OK);
    CHECK(remesa_draw_packet_validate(&out)==REMESA_PACKET_OK);
    CHECK(out.indices[REMESA_DRAW_MAX_INDICES-1]==REMESA_DRAW_MAX_VERTICES-1);
    CHECK(out.positions[REMESA_DRAW_MAX_VERTICES-1][1]==-1023.0f);
    saved=out;
    indices[REMESA_DRAW_MAX_INDICES-1]=REMESA_DRAW_MAX_VERTICES;
    CHECK(remesa_draw_packet_build(&out,(const float (*)[4])positions,
        REMESA_DRAW_MAX_VERTICES,indices,REMESA_DRAW_MAX_INDICES,0)==REMESA_PACKET_INVALID);
    CHECK(!memcmp(&out,&saved,sizeof(out)));
    indices[REMESA_DRAW_MAX_INDICES-1]=REMESA_DRAW_MAX_VERTICES-1;
    CHECK(remesa_draw_packet_build(&out,(const float (*)[4])positions,
        REMESA_DRAW_MAX_VERTICES,indices,UINT32_MAX,0)==REMESA_PACKET_UNSUPPORTED);
    CHECK(!memcmp(&out,&saved,sizeof(out)));
    positions[REMESA_DRAW_MAX_VERTICES-1][0]=-INFINITY;
    CHECK(remesa_draw_packet_build(&out,(const float (*)[4])positions,
        REMESA_DRAW_MAX_VERTICES,indices,REMESA_DRAW_MAX_INDICES,0)==REMESA_PACKET_INVALID);
    CHECK(!memcmp(&out,&saved,sizeof(out)));
    positions[REMESA_DRAW_MAX_VERTICES-1][0]=1023;
    /* Small second snapshot must erase inactive old geometry, not leak it. */
    indices[0]=0; indices[1]=1; indices[2]=2;
    CHECK(remesa_draw_packet_build(&out,(const float (*)[4])positions,3,indices,3,0xffff00ff)==REMESA_PACKET_OK);
    CHECK(out.positions[3][3]==0 && out.indices[3]==0);
    positions[0][0]=-0.0f;
    CHECK(remesa_draw_packet_build(&out,(const float (*)[4])positions,3,indices,3,0)==REMESA_PACKET_OK);
    CHECK(signbit(out.positions[0][0]));
    out.vertex_count=REMESA_DRAW_MAX_VERTICES+1;
    CHECK(remesa_draw_packet_validate(&out)==REMESA_PACKET_UNSUPPORTED);
    out.vertex_count=3;out.indices[2]=3;
    CHECK(remesa_draw_packet_validate(&out)==REMESA_PACKET_INVALID);
    printf("%u independent draw packet review checks PASS; no hardware\n",checks);
    return 0;
}
