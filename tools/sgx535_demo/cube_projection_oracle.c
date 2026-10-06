/* Independent CPU matrix/float32 packing oracle. No SGX or device API. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
static int matrix(double m[4][4])
{
    unsigned i,j;
    for(i=0;i<4;i++)for(j=0;j<4;j++)
        if(scanf("%lf",&m[i][j])!=1 || !isfinite(m[i][j]))return 0;
    return 1;
}
static int apply(double m[4][4], const double in[4], double out[4])
{
    unsigned i,j;
    for(i=0;i<4;i++) {
        out[i]=0;
        for(j=0;j<4;j++)out[i]+=m[i][j]*in[j];
        if(!isfinite(out[i]))return 0;
    }
    return 1;
}
int main(void)
{
    unsigned count,frame,vertex,k;
    if(scanf("%u",&count)!=1 || count!=5)return 2;
    for(frame=0;frame<count;frame++) {
        double m[4][4],v[4][4],p[4][4];
        if(!matrix(m)||!matrix(v)||!matrix(p))return 2;
        for(vertex=0;vertex<8;vertex++) {
            double object[4],world[4],view[4],clip[4];float packed[8];
            for(k=0;k<4;k++)if(scanf("%lf",&object[k])!=1 || !isfinite(object[k]))return 2;
            if(!apply(m,object,world)||!apply(v,world,view)||!apply(p,view,clip)||clip[3]<=1e-12)return 2;
            packed[0]=(float)((clip[0]/clip[3]+1)*16);
            packed[1]=(float)((1-clip[1]/clip[3])*16);
            packed[2]=(float)((clip[2]/clip[3]+1)/2);
            for(k=3;k<8;k++)packed[k]=1;
            for(k=0;k<8;k++) {
                uint32_t word;unsigned n;
                if(!isfinite(packed[k]))return 2;
                memcpy(&word,&packed[k],sizeof(word));
                for(n=0;n<4;n++)if(putchar((int)((word>>(8*n))&255))==EOF)return 3;
            }
        }
    }
    return ferror(stdout)?3:0;
}
