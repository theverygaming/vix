[BITS 32]
%include "../config.inc"
section .entry
global _start
extern startup_cpp
_start:
    mov esp, KERNEL_PHYS_ADDRESS + KERNEL_START_STACK_POINTER_OFFSET
    call startup_cpp
    mov eax, kernelcall
    call puts
    mov esp, KERNEL_VIRT_ADDRESS + KERNEL_START_STACK_POINTER_OFFSET
    call KERNEL_VIRT_ADDRESS
.die:
    cli
    hlt
    jmp .die

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
    out 0xE9, al
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