section .text

global syscall
syscall:
    pusha
    mov eax, [esp+(4*8)+(4*1)]
    mov ebx, [esp+(4*8)+(4*2)]
    mov ecx, [esp+(4*8)+(4*3)]
    mov edx, [esp+(4*8)+(4*4)]
    mov esi, [esp+(4*8)+(4*5)]
    mov edi, [esp+(4*8)+(4*6)]
    mov ebp, [esp+(4*8)+(4*7)]
    int 0x80
    mov [esp-(4*1)], eax
    popa
    mov eax, [esp-(4*1)-(4*8)]
    ret