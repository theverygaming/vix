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

isr_common:
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
    add esp, 8              ; remove error code and interrupt number
    iret