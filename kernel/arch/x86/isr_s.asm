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

%include "arch/x86/isrs.inc"

isr_common:
    push ds
    push es
    push fs
    push gs

    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    
    push ebp

    mov ax, 2 * 8        ; use kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp            ; pass pointer to stack to C, so we can access all the pushed information
    call i686_ISR_Handler
    add esp, 4

    pop ebp

    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax

    pop gs
    pop fs
    pop es
    pop ds

    add esp, 8 ; error code + interrupt number
    
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
