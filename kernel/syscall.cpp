#include "syscall.h"
#include "multitasking.h"
#include "stdlib.h"
#include "stdio.h"
#include "drivers/keyboard.h"
#include "../config.h"

void syscall::syscallHandler(isr::Registers* regs) {
    multitasking::process* currentProcess = multitasking::getCurrentProcess(); // never use this to modify registers, it will not work
    multitasking::context *current_context = (multitasking::context*)(KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET); // return values using this

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

        multitasking::process tempProcessStore;
        memcpy((char*)&tempProcessStore, (char*)currentProcess, sizeof(multitasking::process));
        multitasking::context tempContextStore;
        memcpy((char*)&tempContextStore, (char*)current_context, sizeof(multitasking::context));

        int bufStart = drivers::keyboard::bufferlocation;
        asm("sti");
        while(drivers::keyboard::bufferlocation - bufStart < regs->edx) {
            if(drivers::keyboard::buffer[drivers::keyboard::bufferlocation] == '\n') {
                break;
            }
        }
        asm("cli");
        uint32_t readCharacters = drivers::keyboard::bufferlocation - bufStart;

        drivers::keyboard::bufferlocation = -1;
        memcpy((char*)currentProcess, (char*)&tempProcessStore, sizeof(multitasking::process));
        memcpy((char*)current_context, (char*)&tempContextStore, sizeof(multitasking::context));

        memcpy((char*)regs->ecx, &drivers::keyboard::buffer[bufStart+1], readCharacters);
        current_context->eax = readCharacters;
    }
    else if(regs->eax == 2) { // sys_fork
        printf("syscall: sys_fork\n");
        multitasking::fork_process(currentProcess);
        current_context->eax = 0; // make the process think it's the child
    }
    else if(regs->eax == 11) { // sys_fork
        printf("syscall: sys_execve\n");
        printf("%s\n", regs->ebx);
    }
    else {
        printf("Unknown syscall %u\n", regs->eax);
    }
}