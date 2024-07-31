#include <vix/config.h>

// extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);
.section .text
.global sched_switch
sched_switch:
#ifdef CONFIG_ENABLE_KERNEL_32
    movl 4(%esp), %eax // old
    movl 8(%esp), %ecx // new

    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %ebx

    movl %esp, (%eax)
    movl %ecx, %esp

    popl %ebx
    popl %esi
    popl %edi
    popl %ebp
    ret
#endif
#ifdef CONFIG_ENABLE_KERNEL_64
    pushq %rbp
    pushq %rbx
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12

    movq %rsp, (%rdi)
    movq %rsi, %rsp

    popq %r12
    popq %r13
    popq %r14
    popq %r15
    popq %rbx
    popq %rbp
    ret
#endif
