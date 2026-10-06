#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
int main(int argc,char **argv){
 unsigned char source[4096],out[320U*320U*4U];FILE *f;size_t x,y,j,k;
 if(argc!=2)return 2;f=fopen(argv[1],"rb");if(!f)return 3;
 if(fread(source,1,sizeof source,f)!=sizeof source||fgetc(f)!=EOF){fclose(f);return 4;}fclose(f);
 for(y=0;y<320;y++)for(x=0;x<320;x++){
  j=((y/10)*32+x/10)*4;k=(y*320+x)*4;
  out[k]=source[j];out[k+1]=source[j+1];out[k+2]=source[j+2];out[k+3]=0;
 }
 return fwrite(out,1,sizeof out,stdout)==sizeof out?0:5;
}
