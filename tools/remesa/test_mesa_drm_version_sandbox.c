/* SPDX-License-Identifier: MIT */
/* SIMULATED BPF interpreter: captures the actual production filter via a
 * fake prctl. It never installs seccomp, opens a device, or invokes an ioctl.
 * Real kernel enforcement must be tested separately with production selftest.
 */
#define _GNU_SOURCE
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#define prctl model_prctl
#define getuid model_getuid
#define geteuid model_geteuid
#define ioctl model_ioctl
#define syscall model_syscall
#define execv model_execv
#define fprintf model_fprintf
#define main version_sandbox_program_main
#include "mesa_drm_version_sandbox.c"
#undef main
#undef prctl
#undef getuid
#undef geteuid
#undef ioctl
#undef syscall
#undef execv
#undef fprintf

static struct sock_filter captured[128];
static unsigned captured_len, privileged_stage, fail_stage, checks;
static unsigned prctl_calls, exec_calls, close_calls;
static uid_t simulated_uid = 1000, simulated_euid = 1000;
static int simulated_version_errno = EBADF, simulated_close_error;
static int simulated_uring_errno = EPERM;
static jmp_buf simulated_exec;

uid_t model_getuid(void) { return simulated_uid; }
uid_t model_geteuid(void) { return simulated_euid; }
int model_fprintf(FILE *stream, const char *format, ...)
{
   va_list args;
   va_start(args, format);
   fputs("SIMULATED_MAIN_OUTPUT: ", stream);
   int count = vfprintf(stream, format, args);
   va_end(args);
   return count;
}
int model_ioctl(int fd, unsigned long request, ...)
{
   assert(fd == -1); /* No actual ioctl: every production selftest FD is invalid. */
   errno = request == DRM_IOCTL_VERSION ? simulated_version_errno : EPERM;
   return -1;
}
long model_syscall(long number, ...)
{
#ifdef SYS_io_uring_setup
   if (number == SYS_io_uring_setup) { errno = simulated_uring_errno; return -1; }
#endif
#ifdef SYS_close_range
   if (number == SYS_close_range) {
      va_list args;
      va_start(args, number);
      assert(va_arg(args, unsigned) == 3u);
      assert(va_arg(args, unsigned) == ~0u);
      assert(va_arg(args, unsigned) == 0u);
      va_end(args);
      close_calls++;
      if (simulated_close_error) { errno = simulated_close_error; return -1; }
      return 0;
   }
#endif
   assert(!"unexpected syscall in simulated production main");
   return -1;
}
int model_execv(const char *path, char *const argv[])
{
   assert(close_calls == 1 && !simulated_close_error);
   assert(!strcmp(path, "/qualified/offline-client-fixture"));
   assert(!strcmp(argv[0], path));
   exec_calls++;
   longjmp(simulated_exec, 1); /* Model successful exec; no program is executed. */
}

int model_prctl(int option, ...)
{
   va_list args;
   va_start(args, option);
   prctl_calls++;
   if (option == PR_SET_NO_NEW_PRIVS) {
      assert(va_arg(args, int) == 1);
      privileged_stage = 1;
   } else {
      assert(option == PR_SET_SECCOMP);
      assert(privileged_stage == 1);
      assert(va_arg(args, int) == SECCOMP_MODE_FILTER);
      struct sock_fprog *program = va_arg(args, struct sock_fprog *);
      assert(program && program->len && program->len <= 128);
      if (fail_stage != 2) {
         captured_len = program->len;
         memcpy(captured, program->filter, captured_len * sizeof(captured[0]));
      }
      privileged_stage = 2;
   }
   va_end(args);
   if (fail_stage == privileged_stage) {
      errno = EACCES;
      return -1;
   }
   return 0;
}

static uint32_t evaluate(const struct seccomp_data *data)
{
   uint32_t accumulator = 0;
   unsigned pc = 0;
   assert(captured_len);
   for (unsigned step = 0; step < captured_len; step++) {
      assert(pc < captured_len);
      struct sock_filter instruction = captured[pc++];
      switch (instruction.code) {
      case BPF_LD | BPF_W | BPF_ABS:
         assert(instruction.k <= sizeof(*data) - sizeof(accumulator));
         memcpy(&accumulator, (const unsigned char *)data + instruction.k, sizeof(accumulator));
         break;
      case BPF_JMP | BPF_JEQ | BPF_K:
         pc += accumulator == instruction.k ? instruction.jt : instruction.jf;
         break;
      case BPF_JMP | BPF_JSET | BPF_K:
         pc += (accumulator & instruction.k) ? instruction.jt : instruction.jf;
         break;
      case BPF_RET | BPF_K:
         return instruction.k;
      default:
         assert(!"unsupported instruction in actual captured production filter");
      }
   }
   assert(!"unterminated captured filter");
   return 0;
}

static unsigned target_arch(void)
{
#if defined(__i386__)
   return AUDIT_ARCH_I386;
#elif defined(__x86_64__)
   return AUDIT_ARCH_X86_64;
#elif defined(__aarch64__)
   return AUDIT_ARCH_AARCH64;
#else
#error Unsupported model architecture
#endif
}

static void expect(unsigned arch, int nr, uint64_t request, uint32_t decision)
{
   struct seccomp_data data = {.nr = nr, .arch = arch};
   data.args[0] = UINT64_MAX; /* Invalid fd in the model; no syscall occurs. */
   data.args[1] = request;
   assert(evaluate(&data) == decision);
   checks++;
}

static void denied_syscall(int nr)
{
   expect(target_arch(), nr, DRM_IOCTL_VERSION, SECCOMP_RET_ERRNO | EPERM);
}

int main(void)
{
   /* Installation errors must propagate without pretending a filter exists. */
   fail_stage = 1;
   assert(install() == EACCES && !captured_len);
   checks++;
   fail_stage = 2;
   privileged_stage = 0;
   assert(install() == EACCES && !captured_len);
   checks++;
   fail_stage = privileged_stage = 0;
   assert(install() == 0 && captured_len && privileged_stage == 2);
   checks++;

   unsigned arch = target_arch();
   expect(arch, SYS_ioctl, DRM_IOCTL_VERSION, SECCOMP_RET_ALLOW);
   /* Linux's ioctl request argument is unsigned int. High bits do not change
    * the request executed by the kernel and are not an alternative command. */
   expect(arch, SYS_ioctl, UINT64_C(0x1234567800000000) | DRM_IOCTL_VERSION, SECCOMP_RET_ALLOW);
   for (unsigned bit = 0; bit < 32; bit++)
      expect(arch, SYS_ioctl, DRM_IOCTL_VERSION ^ (UINT32_C(1) << bit), SECCOMP_RET_ERRNO | EPERM);
   for (unsigned request = 0; request < 256; request++) {
      unsigned commands[] = {_IO('d', request), _IOR('d', request, unsigned long),
                             _IOW('d', request, unsigned long), _IOWR('d', request, unsigned long)};
      for (unsigned index = 0; index < sizeof(commands) / sizeof(commands[0]); index++)
         expect(arch, SYS_ioctl, commands[index], commands[index] == DRM_IOCTL_VERSION ?
                SECCOMP_RET_ALLOW : SECCOMP_RET_ERRNO | EPERM);
   }
   unsigned foreign[] = {0, AUDIT_ARCH_I386, AUDIT_ARCH_X86_64, AUDIT_ARCH_AARCH64, arch ^ 0x80000000U};
   for (unsigned index = 0; index < sizeof(foreign) / sizeof(foreign[0]); index++)
      if (foreign[index] != arch)
         expect(foreign[index], SYS_ioctl, DRM_IOCTL_VERSION, SECCOMP_RET_KILL_PROCESS);
   /* An x32 request is killed in every model. On a non-x86_64 target this is
    * foreign-architecture coverage; only x86_64 tests the same-arch JSET. */
   expect(AUDIT_ARCH_X86_64, SYS_ioctl | 0x40000000, DRM_IOCTL_VERSION, SECCOMP_RET_KILL_PROCESS);
#ifdef SYS_io_uring_setup
   denied_syscall(SYS_io_uring_setup);
#endif
#ifdef SYS_io_uring_enter
   denied_syscall(SYS_io_uring_enter);
#endif
#ifdef SYS_io_uring_register
   denied_syscall(SYS_io_uring_register);
#endif
#ifdef SYS_ptrace
   denied_syscall(SYS_ptrace);
#endif
#ifdef SYS_process_vm_writev
   denied_syscall(SYS_process_vm_writev);
#endif
#ifdef SYS_pidfd_getfd
   denied_syscall(SYS_pidfd_getfd);
#endif
   expect(arch, SYS_read, 0, SECCOMP_RET_ALLOW);
   expect(arch, SYS_write, 0, SECCOMP_RET_ALLOW);
   struct seccomp_data data = {.nr = SYS_ioctl, .arch = arch};
   data.args[0] = DRM_IOCTL_VERSION;
   data.args[1] = 0;
   data.args[2] = DRM_IOCTL_VERSION;
   assert(evaluate(&data) == (SECCOMP_RET_ERRNO | EPERM));
   checks++;
   /* Production main branch models: no libc/kernel/device execution. */
   char *launch[] = {"sandbox", "/qualified/offline-client-fixture", NULL};
   unsigned before = prctl_calls;
   simulated_uid = 0;
   assert(version_sandbox_program_main(2, launch) == 2 && prctl_calls == before);
   checks++;
   simulated_uid = 1000; simulated_euid = 0;
   assert(version_sandbox_program_main(2, launch) == 2 && prctl_calls == before);
   checks++;
   simulated_euid = 1000;
   simulated_version_errno = EPERM; /* Outer policy blocks VERSION: fail closed. */
   assert(version_sandbox_program_main(2, launch) == 2 && exec_calls == 0 && close_calls == 0);
   checks++;
   simulated_version_errno = EBADF;
#ifdef SYS_io_uring_setup
   simulated_uring_errno = ENOSYS; /* Outer environment differs: still STOP. */
   assert(version_sandbox_program_main(2, launch) == 2 && exec_calls == 0 && close_calls == 0);
   checks++;
   simulated_uring_errno = EPERM;
#endif
   simulated_close_error = ENOSYS;
   assert(version_sandbox_program_main(2, launch) == 2 && exec_calls == 0);
   checks++;
   simulated_close_error = 0; close_calls = 0;
#ifdef SYS_close_range
   if (!setjmp(simulated_exec)) {
      version_sandbox_program_main(2, launch);
      assert(!"qualified model launch must reach simulated exec");
   }
   assert(exec_calls == 1 && close_calls == 1);
   checks++;
#endif
   printf("SIMULATED_PRODUCTION_BPF_MODEL=PASS; checks=%u; arch=0x%x; DRM_VERSION=0x%lx; no_seccomp_or_DRM_execution\n",
          checks, arch, (unsigned long)DRM_IOCTL_VERSION);
   return 0;
}
