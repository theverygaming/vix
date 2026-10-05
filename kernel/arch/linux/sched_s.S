// extern "C" void core_sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::thread *prev, struct sched::thread *next);
.section .text
.global core_sched_switch
core_sched_switch:
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
