.section .text

.global syscall_entry
syscall_entry:
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

.global x86_interrupt_return
x86_interrupt_return:

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

// extern "C" void x86_load_cpu_full_ctx(struct arch::full_ctx *ctx);
.global x86_load_cpu_full_ctx
x86_load_cpu_full_ctx:
    movl 4(%esp), %esp
    jmp x86_interrupt_return
