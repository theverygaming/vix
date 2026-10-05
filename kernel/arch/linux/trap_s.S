#include <vix/arch/monitor.h>
.section .text
.global trap_entry
trap_entry:
    rdgsbase %rax
    pushq %rax
    rdfsbase %rax
    pushq %rax
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %r10
    pushq %r9
    pushq %r8
    pushq %rbp
    pushq %rsi
    pushq %rdx
    pushq %rbx

    mov %rsp, %rdi
    call trap_handler
    jmp trap_return_stack

.global trap_return
trap_return:
    mov %rdi, %rsp
.global trap_return_stack
trap_return_stack:
    popq %rbx
    popq %rdx
    popq %rsi
    popq %rbp
    popq %r8
    popq %r9
    popq %r10
    popq %r12
    popq %r13
    popq %r14
    popq %r15
    popq %rax
    wrfsbase %rax
    popq %rax
    wrgsbase %rax

    // pop interrupt code & metadata
    add $(5 * 8), %rsp

    // call monitor to return
    mov $MONITOR_CALL, %rax
    mov $MONITOR_CALL_TRAPRET, %rdi
    syscall
