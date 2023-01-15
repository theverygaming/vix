[BITS 32]
%include "../../kernel/include/config.inc"
section .entry
align 8
header_start:
    dd 0xe85250d6                ; magic number
    dd 0                         ; architecture -> 0 = 32-bit protected mode i386
    dd header_end - header_start ; header length

    ; checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    align 8

    dw 2 ; type = 2(address tag)
    dw 0 ; flags
    dd 24 ; size
    dd header_start ; header_addr
    dd -1 ; load_addr -> -1 means must be loaded from beginning
    dd 0 ; load_end_addr -> 0 means .data and .text is whole image
    dd (KERNEL_PHYS_ADDRESS + KERNEL_MEMORY_END_OFFSET) ; bss_end_addr

    align 8

    dw 3 ; type = 3(entry address tag)
    dw 0 ; flags
    dd 12 ; size
    dd _start ; entry point

    align 8

    dw 5 ; type = 5(framebuffer tag)
    dw 0 ; flags
    dd 20 ; size
    dd 800 ; width
    dd 600 ; height
    dd 24 ; depth

    align 8

    dw 6 ; type = 6(Module alignment tag) -> force modules to be page aligned
    dw 0 ; flags
    dd 8 ; size = 8

    align 8

    ; required end tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

global _start
extern startup_cpp
_start:
    mov esp, KERNEL_PHYS_ADDRESS + KERNEL_START_STACK_POINTER_OFFSET
    cmp eax, 0x36D76289
    jne .multiboot_issue
    push ebx ; multiboot 2 info structure pointer

    ; check for CPUID
    pushfd
    pushfd
    xor dword [esp],0x00200000
    popfd
    pushfd
    mov eax, [esp]
    cmp eax, [esp+4]
    je .no_cpuid
    popfd
    popfd

    call startup_cpp
    mov eax, kernelcall
    call puts

    pop ebx ; multiboot 2 info structure pointer
    mov esp, KERNEL_VIRT_ADDRESS + KERNEL_START_STACK_POINTER_OFFSET
    push ebx ; multiboot 2 info structure pointer
    call KERNEL_VIRT_ADDRESS
.multiboot_issue:
    mov eax, multiboot_issue
    call puts
    jmp .die
.no_cpuid:
    mov eax, cpuid_issue
    call puts
    jmp .die
.die:
    cli
    hlt
    jmp .die

section .text
global puts_c
puts_c: ; output: eax->pointer to null termination
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    call puts
    pop ebp
    ret

puts: ; input: eax->pointer to null-terminated string, output: eax->pointer to null termination
    cmp byte [eax], 0
    je .end
    push eax
    mov al, [eax]
    ;out 0xE9, al
    call putc_vidmem
    pop eax
    inc eax
    jmp puts
.end:
    ret

putc_vidmem: ; char in al
    cmp al, 0x0A ; is this LF?
    je .linefeed
    cmp al, 0x0D ; is this CR?
    je .carriagereturn
    cmp dword [vidmemCharCounter], (80 * 25)
    jae .resetcounter
.printchar:
    push eax
    mov ah, 0x07 ; grey on black background
    push ebx
    mov ebx, [vidmemCharCounter]
    mov [0xB8000 + (ebx * 2)], ax
    pop ebx
    inc dword [vidmemCharCounter] ; increment char counter
    pop eax
    jmp .end
.linefeed:
    add dword [vidmemCharCounter], 80
    jmp .end
.carriagereturn:
    push eax
    push ebx
    push edx
    mov eax, [vidmemCharCounter]
    mov edx, 0
    mov ebx, 80
    div ebx ; remainder in edx
    sub dword [vidmemCharCounter], edx
    pop edx
    pop ebx
    pop eax
    jmp .end
.resetcounter:
    push eax
    mov dword [vidmemCharCounter], 0
    pop eax
    jmp .printchar
.end:
    ret

section .data
vidmemCharCounter dd 0
kernelcall db "calling kernel",0x0A, 0x0D, 0x0
multiboot_issue db "multiboot issue",0x0A, 0x0D, 0x0
cpuid_issue db "cpuid required",0x0A, 0x0D, 0x0
