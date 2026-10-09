/* SPDX-License-Identifier: MIT */
#ifndef REMESA_LOADER_SANDBOX_H
#define REMESA_LOADER_SANDBOX_H
#include <errno.h>
#include <stddef.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
/* Loader-only isolation. No DRM ioctl can reach any installed kernel driver.
 * No fallback if the kernel cannot enforce this filter. */
static int remesa_loader_deny_ioctls(void)
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
  /* x32 shares the x86-64 audit architecture but uses another syscall ABI. */
  BPF_JUMP(BPF_JMP|BPF_JSET|BPF_K,0x40000000U,0,1),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_KILL_PROCESS),
#endif
  BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,SYS_ioctl,0,1),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EPERM),
  BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ALLOW),
 };
 struct sock_fprog program={.len=sizeof(code)/sizeof(code[0]),.filter=code};
 if(prctl(PR_SET_NO_NEW_PRIVS,1,0,0,0))return errno;
 if(prctl(PR_SET_SECCOMP,SECCOMP_MODE_FILTER,&program))return errno;
 return 0;
#endif
}
#endif
