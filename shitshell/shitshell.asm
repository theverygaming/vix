section .text
global  _start
_start:
    mov eax, 4 ; sys_write(stdout, hello, len)
    mov ebx, 1
    mov ecx, message
    mov edx, length
    int 80h

shell_loop:
    call read_stdin
    call spawn_process_and_wait_for_child
    jmp shell_loop

spawn_process_and_wait_for_child:
    mov eax, 2 ; sys_fork(0)
    mov ebx, 0
    int 80h
    cmp eax, 0
    je child
    mov ebx, eax
    mov eax, 7 ; sys_waitpid(ebx, 0, 0)
    mov ecx, 0
    mov edx, 0
    int 80h
    ret
child:
    mov eax, 11 ; sys_execve(stdinstr, 0, 0)
    mov ebx, stdinstr
    mov ecx, 0
    mov edx, 0
    int 0x80
    jmp exit ; if we couldn't execve exit this child process so we do not get an infinite amount of child processes
    ret

read_stdin:
    mov eax, 3 ; sys_read(stdin, stdinstr, stdinstr_mlen)
    mov ebx, 0
    mov ecx, stdinstr
    mov edx, [stdinstr_mlen]
    int 0x80
    mov [stdinstr_len], eax
    mov ebx, 0
    mov [stdinstr+eax-1], ebx ; replace \n with null termination
    ret

exit:
    mov eax, 1 ; sys_exit(0)
    mov ebx, 0
    int 80h


section .data
    message db "welcome to the uselessShell:tm:" , 0xa
    length equ $-message
    stdinstr_mlen dw 40
    stdinstr times 40 dw 0
    stdinstr_len dw 0