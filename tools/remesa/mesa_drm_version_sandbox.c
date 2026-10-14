/* SPDX-License-Identifier: MIT */
/* Loader-only live diagnosis: DRM_IOCTL_VERSION is the sole allowed ioctl.
 * No GPU/GEM/modeset ioctl, no admission reset. The selected program and its
 * dependency hashes must be independently qualified before deployment. */
#define _GNU_SOURCE
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <drm.h>
static int install(void)
{
#if defined(__i386__)
 const unsigned arch=AUDIT_ARCH_I386;
#elif defined(__x86_64__)
 const unsigned arch=AUDIT_ARCH_X86_64;
#elif defined(__aarch64__)
 const unsigned arch=AUDIT_ARCH_AARCH64;
#else
 return ENOTSUP;
#endif
#if defined(__i386__) || defined(__x86_64__) || defined(__aarch64__)
 struct sock_filter code[]={
  BPF_STMT(BPF_LD|BPF_W|BPF_ABS,offsetof(struct seccomp_data,arch)),
  BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,arch,1,0),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_KILL_PROCESS),
  BPF_STMT(BPF_LD|BPF_W|BPF_ABS,offsetof(struct seccomp_data,nr)),
#if defined(__x86_64__)
  BPF_JUMP(BPF_JMP|BPF_JSET|BPF_K,0x40000000U,0,1),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_KILL_PROCESS),
#endif
#ifdef SYS_io_uring_setup
  BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,SYS_io_uring_setup,0,1),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EPERM),
#endif
#ifdef SYS_io_uring_enter
  BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,SYS_io_uring_enter,0,1),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EPERM),
#endif
#ifdef SYS_io_uring_register
  BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,SYS_io_uring_register,0,1),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EPERM),
#endif
#ifdef SYS_ptrace
  BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,SYS_ptrace,0,1),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EPERM),
#endif
#ifdef SYS_process_vm_writev
  BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,SYS_process_vm_writev,0,1),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EPERM),
#endif
#ifdef SYS_pidfd_getfd
  BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,SYS_pidfd_getfd,0,1),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EPERM),
#endif
  BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,SYS_ioctl,1,0),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ALLOW),
  BPF_STMT(BPF_LD|BPF_W|BPF_ABS,offsetof(struct seccomp_data,args[1])),
  BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,DRM_IOCTL_VERSION,1,0),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EPERM),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ALLOW),
 };
 struct sock_fprog program={.len=sizeof(code)/sizeof(code[0]),.filter=code};
 if(prctl(PR_SET_NO_NEW_PRIVS,1,0,0,0))return errno;
 if(prctl(PR_SET_SECCOMP,SECCOMP_MODE_FILTER,&program))return errno;
 return 0;
#endif
}
int main(int argc,char **argv)
{
 if(argc<2||getuid()==0||geteuid()==0)return 2;
 int error=install();if(error){fprintf(stderr,"seccomp: %d\n",error);return 2;}
 /* Invalid fd ensures these qualification calls reach no device. */
 errno=0;
 if(ioctl(-1,DRM_IOCTL_VERSION,NULL)!=-1||errno!=EBADF) {
  fprintf(stderr,"VERSION policy selftest failed: errno=%d, expected EBADF; no client executed\n",errno);return 2;
 }
 unsigned rejected=0;
 for(unsigned request=0;request<256;request++) {
  unsigned long command=_IOWR('d',request,unsigned long);
  if(command==DRM_IOCTL_VERSION)continue;
  errno=0;if(ioctl(-1,command,NULL)!=-1||errno!=EPERM) {
   fprintf(stderr,"Forbidden ioctl policy selftest failed: request=%lu errno=%d; no client executed\n",command,errno);return 2;
  }
  rejected++;
 }
#ifdef SYS_io_uring_setup
 errno=0;if(syscall(SYS_io_uring_setup,0,NULL)!=-1||errno!=EPERM) {
  fprintf(stderr,"io_uring policy selftest failed: errno=%d, expected EPERM; no client executed\n",errno);return 2;
 }
#endif
 fprintf(stderr,"DRM_VERSION_ONLY_SANDBOX=ENFORCED; forbidden_requests=%u; SGX_IOCTLS=DENIED\n",rejected);
 if(argc==2&&!strcmp(argv[1],"--selftest"))return 0;
 if(argv[1][0]!='/')return 2;
#ifdef SYS_close_range
 /* Never carry an inherited device or io_uring descriptor into the client. */
 if(syscall(SYS_close_range,3u,~0u,0u)){perror("close_range");return 2;}
#else
 return 2;
#endif
 execv(argv[1],argv+1);perror("execv");return 2;
}
