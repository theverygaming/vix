.section .text
.global _kentry
_kentry:
    mov (%rsp), %rdi // argc
    lea 8(%rsp), %rsi // argv
    lea 16(%rsp,%rdi,8), %rdx // envp

    movabs $_kentry_c, %rax
    call *%rax

    // exit(1)
    mov $60, %rax
    mov $2, %rdi
    syscall
