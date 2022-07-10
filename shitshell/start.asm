section .text
global  _start
extern main
_start:
    xor ebp, ebp ; mark end of stack frames
    

    call main ; call C main(int argc, char* argv[])
    
    ; exit with return value of main
    mov ebx, eax ; main returns in eax
    mov eax, 1 ; sys_exit
    int 0x80
    hlt ; if we are here something went wrong badly