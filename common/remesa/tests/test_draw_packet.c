/* SPDX-License-Identifier: MIT */
#include "remesa/draw_packet.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    struct remesa_draw_packet p, saved;
    float v[4][4] = {{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1},{24,24,.5f,1}};
    uint16_t ix[6] = {0,1,2,1,3,2};
    assert(remesa_draw_packet_build(&p,(const float (*)[4])v,4,ix,6,0xffff00ff)==REMESA_PACKET_OK);
    assert(remesa_draw_packet_validate(&p)==REMESA_PACKET_OK);
    assert(p.argb==0xffff00ff && p.indices[4]==3 && p.positions[3][0]==24);
    saved=p;
    v[0][0]=9; ix[0]=3;
    assert(!memcmp(&p,&saved,sizeof(p))); /* Deep snapshot. */
    ix[0]=4;
    assert(remesa_draw_packet_build(&p,(const float (*)[4])v,4,ix,6,0)==REMESA_PACKET_INVALID);
    assert(!memcmp(&p,&saved,sizeof(p)));
    ix[0]=0;v[0][0]=NAN;
    assert(remesa_draw_packet_build(&p,(const float (*)[4])v,4,ix,6,0)==REMESA_PACKET_INVALID);
    v[0][0]=INFINITY;
    assert(remesa_draw_packet_build(&p,(const float (*)[4])v,4,ix,6,0)==REMESA_PACKET_INVALID);
    v[0][0]=-5;v[0][3]=0;
    assert(remesa_draw_packet_build(&p,(const float (*)[4])v,4,ix,6,0)==REMESA_PACKET_UNSUPPORTED);
    v[0][3]=1;
    assert(remesa_draw_packet_build(&p,(const float (*)[4])v,4,ix,6,0)==REMESA_PACKET_OK);
    saved=p;
    assert(remesa_draw_packet_build(&p,(const float (*)[4])v,4,ix,5,0)==REMESA_PACKET_INVALID);
    assert(remesa_draw_packet_build(&p,(const float (*)[4])v,4,ix,0,0)==REMESA_PACKET_INVALID);
    assert(remesa_draw_packet_build(&p,(const float (*)[4])v,1025,ix,6,0)==REMESA_PACKET_UNSUPPORTED);
    assert(remesa_draw_packet_build(&p,p.positions,4,p.indices,6,0)==REMESA_PACKET_INVALID);
    assert(!memcmp(&p,&saved,sizeof(p)));
    p.width=33;
    assert(remesa_draw_packet_validate(&p)==REMESA_PACKET_UNSUPPORTED);
    p=saved;p.indices[1]=4;
    assert(remesa_draw_packet_validate(&p)==REMESA_PACKET_INVALID);
    puts("draw packet snapshot and failure contracts: PASS");
    return 0;
}
