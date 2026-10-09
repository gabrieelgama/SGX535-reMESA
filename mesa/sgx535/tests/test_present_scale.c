/* SPDX-License-Identifier: MIT
 * Reuse simulated Gallium/XCB fixtures. No renderer or physical server. */
#define main remesa_existing_xcb_fixture_test
#include "test_xcb.c"
#undef main
#include "remesa_present_scale.h"
int main(void)
{
 struct pipe_screen *s=(void *)pixels;struct pipe_context c={.screen=s,.texture_map=map,.texture_unmap=unmap};
 struct pipe_resource r={.screen=s,.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,.width0=32,.height0=32,.depth0=1,.array_size=1};
 r.reference.count=1;active=&r;
 for(unsigned y=0;y<32;y++)for(unsigned x=0;x<32;x++){
  unsigned char *p=pixels+(y*32+x)*4;p[0]=x;p[1]=y;p[2]=x^y;p[3]=255;
 }
 unsigned char output[48*264];memset(output,0xa5,sizeof(output));
 struct remesa_present_scale_receipt receipt;
 assert(!remesa_present_scaled_snapshot(&c,&r,output,264,sizeof(output),64,48,REMESA_PRESENT_FIT,&receipt));
 assert(receipt.content_x==8&&receipt.content_y==0&&receipt.content_width==48&&receipt.content_height==48&&receipt.source_bytes==4096&&receipt.cpu_output_bytes==64*48*4);
 for(unsigned y=0;y<48;y++)for(unsigned x=0;x<64;x++){
  const unsigned char *p=output+y*264+x*4;
  if(x<8||x>=56)assert(!p[0]&&!p[1]&&!p[2]&&p[3]==255);
  else {unsigned sx=(x-8)*32/48,sy=y*32/48;assert(p[0]==sx&&p[1]==sy&&p[2]==(sx^sy)&&p[3]==255);}
 }
 for(unsigned y=0;y<48;y++)for(unsigned x=256;x<264;x++)assert(output[y*264+x]==0xa5);
 assert(!remesa_present_scaled_snapshot(&c,&r,output,12,sizeof(output),3,2,REMESA_PRESENT_STRETCH,&receipt));
 assert(output[0]==0&&output[4]==10&&output[8]==21&&output[13]==16);
 memset(output,0xa5,sizeof(output));unsigned maps=map_count;
 state=1;assert(remesa_present_scaled_snapshot(&c,&r,output,264,sizeof(output),64,48,REMESA_PRESENT_FIT,&receipt)==EAGAIN&&map_count==maps);
 state=2;assert(remesa_present_scaled_snapshot(&c,&r,output,264,sizeof(output),64,48,REMESA_PRESENT_FIT,&receipt)==EIO);state=0;
 fail=7;assert(remesa_present_scaled_snapshot(&c,&r,output,264,sizeof(output),64,48,REMESA_PRESENT_FIT,&receipt)==EIO);fail=0;
 for(unsigned i=0;i<sizeof(output);i++)assert(output[i]==0xa5);
 maps=map_count;
 assert(remesa_present_scaled_snapshot(&c,&r,output,255,sizeof(output),64,48,REMESA_PRESENT_FIT,&receipt)==EINVAL);
 assert(remesa_present_scaled_snapshot(&c,&r,output,264,1,64,48,REMESA_PRESENT_FIT,&receipt)==ENOSPC);
 assert(remesa_present_scaled_snapshot(&c,&r,output,SIZE_MAX,SIZE_MAX,64,48,REMESA_PRESENT_FIT,&receipt)==EOVERFLOW);
 assert(remesa_present_scaled_snapshot(&c,&r,output,264,sizeof(output),0,48,REMESA_PRESENT_FIT,&receipt)==EINVAL);
 assert(remesa_present_scaled_snapshot(&c,&r,output,264,sizeof(output),64,48,(enum remesa_present_scale_mode)99,&receipt)==EINVAL);
 assert(map_count==maps&&!queries&&!puts_&&r.reference.count==1);
 puts("PASS CPU nearest-neighbor stretch/fit, letterbox/padding, exact pixels, failed publication unchanged, overflow/bounds; no GPU or X server");return 0;
}
