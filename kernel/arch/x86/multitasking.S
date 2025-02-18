#include <vix/config.h>

// extern "C" void x86_sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::task *prev, struct sched::task *next);
.section .text
.global x86_sched_switch
x86_sched_switch:
#ifdef CONFIG_ENABLE_KERNEL_32
    movl 4(%esp), %eax // old
    movl 8(%esp), %ecx // new

    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %ebx
    pushfl

    movl %esp, (%eax)
    movl %ecx, %esp

    popfl
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
