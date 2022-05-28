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
    cli
    ; we store all registers in a struct at 0x100043C
    mov [0x100043C], eax
    mov [0x100043C+4], ebx
    mov [0x100043C+8], ecx
    mov [0x100043C+12], edx
    mov [0x100043C+16], esi
    mov [0x100043C+20], edi
    mov [0x100043C+24], esp
    mov [0x100043C+28], ebp
    add dword [0x100043C+24], 8
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
    mov eax, [0x100043C]
    mov ebx, [0x100043C+4]
    mov ecx, [0x100043C+8]
    mov edx, [0x100043C+12]
    mov esi, [0x100043C+16]
    mov edi, [0x100043C+20]
    mov esp, [0x100043C+24]
    mov ebp, [0x100043C+28]
    sti
    add dword [0xB8314], 1
    iret