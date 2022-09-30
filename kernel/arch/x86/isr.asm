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
    ; we store all registers in a struct at KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET], eax
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 4], ebx
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 8], ecx
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 12], edx
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 16], esi
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 20], edi
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 24], esp
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 28], ebp
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 32], ds
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 34], es
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 36], fs
    mov [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 38], gs
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
    cmp dword [esp + (4 * 12)], 0x80 ; only do malloc for syscalls
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
    call memcpy32f
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
    mov ds, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 32]
    mov es, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 34]
    mov fs, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 36]
    mov gs, [KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET + 38]
    sti
    iret

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
