#include "syscall.h"
#include "multitasking.h"
#include "stdlib.h"
#include "stdio.h"

void syscall::syscallHandler(isr::Registers* regs) {
    if(regs->eax == 1) { // sys_exit
        printf("syscall: sys_exit\n");
        multitasking::killCurrentProcess();
    }
    else if(regs->eax == 4) { // sys_write
        printf("syscall: sys_write\n");
        char string[regs->edx + 1];
        string[regs->edx] = '\0';
        memcpy((char*)&string, (char*)regs->ecx, regs->edx);
        printf("%s", string);
    }
    else {
        printf("Unknown syscall %u\n", regs->eax);
        asm("hlt");
    }
}