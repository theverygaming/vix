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
        register uint32_t esp asm("esp");
        printf("esp: %p\n", esp);
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

        int bufStart = drivers::keyboard::bufferlocation;
        while(drivers::keyboard::bufferlocation - bufStart < (int)regs->edx) {
            drivers::keyboard::manualRead();
            if(drivers::keyboard::buffer[drivers::keyboard::bufferlocation] == '\n') {
                break;
            }
        }
        uint32_t readCharacters = drivers::keyboard::bufferlocation - bufStart;

        drivers::keyboard::bufferlocation = -1;
        s_memcpy((char*)regs->ecx, &drivers::keyboard::buffer[bufStart+1], readCharacters);
        current_context->eax = readCharacters;
    }
    else if(regs->eax == 2) { // sys_fork
        printf("syscall: sys_fork\n");
        multitasking::process* newprocess = multitasking::fork_process(currentProcess, regs->eip);
        newprocess->registerContext.eax = 0;
        current_context->eax = newprocess->pid;
    }
    else if(regs->eax == 11) { // sys_execve
        printf("syscall: sys_execve\n");
        printf("%s\n", regs->ebx);
    }
    else {
        printf("Unknown syscall %u\n", regs->eax);
    }
}