/* i386 cdecl: pointer at 4(%esp); one 32-bit load from that pointer. */
    .text
    .globl sgx535_load32_once
    .type sgx535_load32_once, @function
sgx535_load32_once:
    movl 4(%esp), %eax
    movl (%eax), %eax
    ret
    .size sgx535_load32_once, .-sgx535_load32_once
    .section .note.GNU-stack,"",@progbits
