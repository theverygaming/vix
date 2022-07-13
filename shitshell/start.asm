section .text
global  _start
extern _startc
_start:
    xor ebp, ebp ; mark end of stack frames
    
    pop eax ; argc
    pop ebx ; argv
    lea ecx, [4+ebx+eax*4] ; envp_str = 4+argv+(4*argc)
    push ecx ; envp_str
    push ebx ; argv
    push eax ; argc

    call _startc ; call C start function(int argc, char* argv[], char* envp_str)
    
    ; exit with return value of main
    mov ebx, eax ; main returns in eax
    mov eax, 1 ; sys_exit
    int 0x80
    hlt ; if we are here something went wrong badly