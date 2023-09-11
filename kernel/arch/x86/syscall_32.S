.section .text

.global syscall_entry
syscall_entry:
    cli

    push $0 // error code
    push $0 // interrupt number

    push %ds
    push %es
    push %fs
    push %gs

    push %eax
    push %ebx
    push %ecx
    push %edx
    push %esi
    push %edi
    
    push %ebp

    push %esp
    mov $(2 * 8), %ax // use kernel data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    call syscallHandler
    add $4, %esp

    pop %ebp

    pop %edi
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx
    pop %eax

    pop %gs
    pop %fs
    pop %es
    pop %ds

    add $8, %esp // error code + interrupt number

    iretl
