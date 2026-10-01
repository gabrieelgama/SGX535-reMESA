#include <stdio.h>
#ifndef __aarch64__
#error Native HOSTCC must target AArch64
#endif
int main(void) { puts("ARM64 native HOSTCC smoke PASS"); return 0; }
