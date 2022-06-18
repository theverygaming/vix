[bits 32]

extern i686_ISR_Handler

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

%include "isrs.inc"

%include "../config.inc"

isr_common:
    cli
    ; we store all registers in a struct at KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET], eax
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 4], ebx
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 8], ecx
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 12], edx
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 16], esi
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 20], edi
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 24], esp
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 28], ebp
    add dword [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 24], 8
    pusha

    xor eax, eax
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp                ; pass pointer to stack to C
    call i686_ISR_Handler
    add esp, 4

    pop eax                 ; restore old segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    ;add esp, 8              ; remove error code and interrupt number
    mov eax, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET]
    mov ebx, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 4]
    mov ecx, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 8]
    mov edx, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 12]
    mov esi, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 16]
    mov edi, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 20]
    mov esp, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 24]
    mov ebp, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 28]
    sti
    iret