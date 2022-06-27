#include "syscall.h"
#include "multitasking.h"
#include "stdlib.h"
#include "stdio.h"
#include "drivers/keyboard.h"
#include "../config.h"

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

        multitasking::context *current_context = (multitasking::context*)(KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET);
        multitasking::process tempProcessStore;
        memcpy((char*)&tempProcessStore, (char*)currentProcess, sizeof(multitasking::process));
        multitasking::context tempContextStore;
        memcpy((char*)&tempContextStore, (char*)current_context, sizeof(multitasking::context));

        asm("sti");
        while(drivers::keyboard::bufferlocation < 10) {
            
        }
        asm("cli");

        drivers::keyboard::bufferlocation = -1;
        memcpy((char*)currentProcess, (char*)&tempProcessStore, sizeof(multitasking::process));
        memcpy((char*)current_context, (char*)&tempContextStore, sizeof(multitasking::context));

        char string[11] = "/bin/bash\n";
        memcpy((char*)regs->ecx, string, 10);
        current_context->eax = 10;
    }
    else if(regs->eax == 2) { // sys_fork
        printf("syscall: sys_fork\n");
        multitasking::fork_process(currentProcess);
    }
    else {
        printf("Unknown syscall %u\n", regs->eax);
    }
}