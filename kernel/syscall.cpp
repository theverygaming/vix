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
        current_context->eax = 0; // zero read characters
        return;
        // this is broken.

        isr::Registers regscp; // copy registers to stack, otherwise they will get lost --- another thing to be fixed later, why is this a pointer anyway?
        memcpy((char*)&regscp, (char*)regs, sizeof(isr::Registers));

        multitasking::process tempProcessStore;
        memcpy((char*)&tempProcessStore, (char*)currentProcess, sizeof(multitasking::process));
        multitasking::context tempContextStore;
        memcpy((char*)&tempContextStore, (char*)current_context, sizeof(multitasking::context));

        int bufStart = drivers::keyboard::bufferlocation;
        multitasking::setProcessSwitching(false);
        asm("sti");
        while(drivers::keyboard::bufferlocation - bufStart < (int)regscp.edx) {
            if(drivers::keyboard::buffer[drivers::keyboard::bufferlocation] == '\n') {
                break;
            }
        }
        asm("cli");
        multitasking::setProcessSwitching(true);
        uint32_t readCharacters = drivers::keyboard::bufferlocation - bufStart;

        drivers::keyboard::bufferlocation = -1;
        memcpy((char*)currentProcess, (char*)&tempProcessStore, sizeof(multitasking::process));
        memcpy((char*)current_context, (char*)&tempContextStore, sizeof(multitasking::context));
        register uint32_t esp asm("esp");
  	    printf("esp: %p\n", esp);
        for(;;);
        s_memcpy((char*)regscp.ecx, &drivers::keyboard::buffer[bufStart+1], readCharacters);
        for(;;);
        current_context->eax = readCharacters;

        for(;;);
    }
    else if(regs->eax == 2) { // sys_fork
        printf("syscall: sys_fork\n");
        multitasking::process* newprocess = multitasking::fork_process(currentProcess, regs->eip);
        newprocess->registerContext.eax = 0;
        current_context->eax = newprocess->pid;
        //current_context->eax = 0;
    }
    else if(regs->eax == 11) { // sys_fork
        printf("syscall: sys_execve\n");
        printf("%s\n", regs->ebx);
    }
    else {
        printf("Unknown syscall %u\n", regs->eax);
    }
}