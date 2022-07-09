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

    ; we want to switch to another stack location so we have to copy everything over to the new location
    push eax
    push ebx
    push ecx
    cmp dword [esp + (4 * 9)], 0x80 ; only do malloc for syscalls
    jne .isnosys
    call isr_alloc_stack ; returns allocated address in eax
    jmp .idfk
    .isnosys:
    mov eax, KERNEL_VIRT_ADDRESS + KERNEL_ISR_STACK_POINTER_OFFSET - 64
    .idfk:
    mov edx, eax ; move returned value to edx, we will need it later
    sub eax, 64 ; subtract 64 from returned address because we copy data to the new stack
    mov ebx, esp ; source
    add ebx, 12
    mov ecx, 16 ; size
    call memcpy32
    pop ecx
    pop ebx
    pop eax
    
    sub edx, 64
    mov esp, edx
    mov ebp, edx
    add edx, 64
    .skipmalloc:

    push edx
    push esp                ; pass pointer to stack to C
    call i686_ISR_Handler

    cmp dword [esp + (4 * 11)], 0x80
    jne .skipfree
    push eax
    call isr_free_stack
    .skipfree:
    


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




; in the future change this to simply use rep movs because S P E E D
memcpy32: ; Arguments: eax->dest, ebx->source, ecx->element count in 32bits
    push eax
    push ebx
    push ecx
    push edx
.cp:
    dec ecx
    cmp ecx, -1 ; if ecx = -1 we copied everything
    je .end
    mov edx, [ebx]
    mov [eax], edx
    add eax, 4 ; add 4 bytes to dest & source
    add ebx, 4
    jmp .cp
.end:
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret