#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#ifdef X11_ABI
#include <X11/Xlib.h>
#endif
int main(void) {
#ifdef X11_ABI
 printf("{\"Image.data\":%zu,\"Image.bits_per_pixel\":%zu,\"Image.red_mask\":%zu,\"Attributes.size\":%zu,\"Visual.size\":%zu}\n",offsetof(XImage,data),offsetof(XImage,bits_per_pixel),offsetof(XImage,red_mask),sizeof(XWindowAttributes),sizeof(Visual));
#else
 const unsigned pitch=5120, x=64, y=64; size_t size=(size_t)pitch*800;
 unsigned char *b=malloc(size); if(!b)return 1; memset(b,0x55,size);
 unsigned count=0;
 for(unsigned sy=0;sy<32;sy++) for(unsigned sx=0;sx<32;sx++){
   uint32_t argb=(sy>=8 && sy<=22 && sx>=8 && sx<=30-sy)?0xffff00ff:0;
   uint32_t rgb=argb & 0xffffff;
   size_t off=(size_t)(y+sy)*pitch+4*(x+sx);
   if(off+4>size)return 2;
   b[off]=(unsigned char)rgb; b[off+1]=(unsigned char)(rgb>>8);
   b[off+2]=(unsigned char)(rgb>>16); b[off+3]=0;
   if(argb)count++;
 }
 if(count!=120)return 3;
 if(fwrite(b,1,size,stdout)!=size)return 4;
 free(b);
#endif
 return 0;
}
