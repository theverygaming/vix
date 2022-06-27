#include "syscall.h"
#include "multitasking.h"
#include "stdlib.h"
#include "stdio.h"

void syscall::syscallHandler(isr::Registers* regs) {
    multitasking::process* currentProcess = multitasking::getCurrentProcess();

    if(regs->eax == 1) { // sys_exit
        printf("syscall: sys_exit code: %d\n", regs->ebx);
        multitasking::killCurrentProcess();
    }
    else if(regs->eax == 4) { // sys_write
        printf("syscall: sys_write\n");
        char string[regs->edx + 1];
        string[regs->edx] = '\0';
        memcpy(string, (char*)regs->ecx, regs->edx);
        printf("%s", string);
    }
    else if(regs->eax == 3) { // sys_read
        printf("syscall: sys_read\n");
        char string[11] = "/bin/bash\n";
        memcpy((char*)regs->ecx, string, 10);
        currentProcess->registerContext.eax = 10;
    }
    else if(regs->eax == 2) { // sys_fork
        printf("syscall: sys_fork\n");
        multitasking::fork_process(currentProcess);
    }
    else {
        printf("Unknown syscall %u\n", regs->eax);
    }
}