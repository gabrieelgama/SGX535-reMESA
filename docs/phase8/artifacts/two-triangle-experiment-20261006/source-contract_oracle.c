/* CPU only. Full backing/relocation oracle; never opens a device. */
#include "frozen_kernel_contract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(x) do { if (!(x)) { fprintf(stderr,"oracle failure line %d\n",__LINE__); return 1; } } while (0)
int main(int argc, char **argv)
{
    struct sgx535_frozen_bo b[SGX535_BO_COUNT];
    struct sgx535_frozen_cpu_view v[SGX535_BO_COUNT]={{0}};
    const sgx535_u64 addresses[]={0x20000000,0x20080000,0x40000000,0x30000000,0,0x40001000,0x40002000,0x42000000,0x31000000,0};
    const sgx535_u32 regs[]={3,4};
    unsigned int i;
    int relocated=argc==2 && strcmp(argv[1],"relocated")==0;
    REQUIRE(argc==1 || relocated);
    for(i=0;i<SGX535_BO_COUNT;i++) {
        struct sgx535_frozen_requirement r;
        REQUIRE(sgx535_frozen_get_requirement(i,&r)==0);
        b[i]=(struct sgx535_frozen_bo){i,r.domain,r.size,addresses[i],i+1};
        if(i<6) {
            v[i].bytes=malloc((size_t)r.size); v[i].length=r.size;
            REQUIRE(v[i].bytes); memset(v[i].bytes,0xa5,(size_t)r.size);
        }
    }
    REQUIRE(sgx535_frozen_initialize_user_images(v)==0);
    /* Invalid size must fail before changing even the first allocation. */
    v[5].length--;
    REQUIRE(sgx535_frozen_initialize_user_images(v)!=0);
    v[5].length++;
    if(relocated) {
        sgx535_u8 *copy=malloc((size_t)v[0].length);
        REQUIRE(copy); memcpy(copy,v[0].bytes,(size_t)v[0].length);
        b[2].gpu_va++;
        REQUIRE(sgx535_frozen_apply_relocations(b,SGX535_BO_COUNT,0x80000000,v,regs)!=0);
        REQUIRE(memcmp(copy,v[0].bytes,(size_t)v[0].length)==0);
        b[2].gpu_va--; free(copy);
        REQUIRE(sgx535_frozen_apply_relocations(b,SGX535_BO_COUNT,0x80000000,v,regs)==0);
    }
    for(i=0;i<6;i++) {
        REQUIRE(fwrite(v[i].bytes,1,(size_t)v[i].length,stdout)==v[i].length);
        free(v[i].bytes);
    }
    return 0;
}
