.section .text
.global linux_syscall
linux_syscall:
    mov %rdi, %rax
    mov %rsi, %rdi
    mov %rdx, %rsi
    mov %rcx, %rdx
    mov %r8, %r10
    mov %r9, %r8
    mov 8(%rsp), %r9
    syscall
    // return in %rax, fits ABI
    ret

.global linux_signal_restorer
linux_signal_restorer:
    // rt_sigreturn
    mov $15, %rax
    syscall
