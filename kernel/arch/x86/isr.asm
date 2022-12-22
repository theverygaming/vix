[bits 32]

extern i686_ISR_Handler
extern isr_alloc_stack
extern isr_free_stack

%macro ISR_NOERRORCODE 1

global i686_ISR%1:
i686_ISR%1:
    push 0              ; push dummy error code
    push %1             ; push interrupt number
    jmp isr_common

%endmacro

%macro ISR_ERRORCODE 1

global i686_ISR%1:
i686_ISR%1:
                        ; cpu pushes error code to stack
    push %1             ; push interrupt number
    jmp isr_common

%endmacro

%include "arch/x86/isrs.inc"

%include "include/config.inc"

isr_common:
    cli

    ; push old stack
    push esp
    add dword [esp], 8     ; remove error code and interrupt number

    ; our stack now
    ; esp_kernel, interrupt, error, eip, cs, eflags, esp_user, ss_user

    ; jmp .noswitchstack ; skip this for usermode testing
    ; do we have to swap stacks(are we coming from a user process?)
    cmp esp, (KERNEL_VIRT_ADDRESS + KERNEL_ISR_STACK_POINTER_OFFSET) - (8 * 4)
    je .noswitchstack

    ; switch stack
    push eax
    push ebx
    push ecx
    mov ecx, 10
    mov eax, (KERNEL_VIRT_ADDRESS + KERNEL_ISR_STACK_POINTER_OFFSET) - (10 * 4)
    lea ebx, [esp + (3 * 4)]
    call memcpy32f ; Arguments: eax->dest, ebx->source, ecx->element count in 32bits
    pop ecx
    pop ebx
    pop eax
    mov esp, (KERNEL_VIRT_ADDRESS + KERNEL_ISR_STACK_POINTER_OFFSET) - (10 * 4)

.noswitchstack:

    pusha               ; pushes in order: eax, ecx, edx, ebx, esp, ebp, esi, edi

    o16 push ds
    o16 push es
    o16 push fs
    o16 push gs

    mov ax, 0x10        ; use kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp            ; pass pointer to stack to C, so we can access all the pushed information
    call i686_ISR_Handler
    add esp, 4

    o16 pop gs              ; restore old segments
    o16 pop fs
    o16 pop es
    o16 pop ds

    popa                ; pop what we pushed with pusha
    pop esp
    sti
    iretd                ; will pop: cs, eip, eflags, ss, esp

memcpy32f: ; Arguments: eax->dest, ebx->source, ecx->element count in 32bits
    push ecx
    push esi
    push edi

    pushfd ; push eflags
    cld ; clear direction flag

    mov esi, ebx ; source
    mov edi, eax ; dest
    rep movsd

    popfd ; pop eflags

    pop edi
    pop esi
    pop ecx
    ret
