#include "syscall.h"
#include "../config.h"
#include "debug.h"
#include "drivers/keyboard.h"
#include "multitasking.h"
#include "paging.h"
#include "stdio.h"
#include "stdlib.h"

void syscall::syscallHandler(isr::Registers *regs) {
    multitasking::process *currentProcess = multitasking::getCurrentProcess();                                       // never use this to modify registers, it will not work
    multitasking::context *current_context = (multitasking::context *)(KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET); // return values using this
    
    if (regs->eax == 1) {                                                                                            // sys_exit
        DEBUG_PRINTF("syscall: sys_exit code: %d\n", regs->ebx);
        multitasking::killCurrentProcess();
    } else if (regs->eax == 4) { // sys_write
        DEBUG_PRINTF("syscall: sys_write\n");
        if (!paging::is_readable((void *)regs->ecx + regs->edx) || regs->edx > 100) {
            return;
        }
        char string[regs->edx + 1];
        string[regs->edx] = '\0';
        memcpy(string, (char *)regs->ecx, regs->edx);
        printf("%s", string);
        current_context->eax = regs->edx; // return number of written bytes
    } else if (regs->eax == 3) { // sys_read
        DEBUG_PRINTF("syscall: sys_read\n");

        int bufStart = drivers::keyboard::bufferlocation;
        while (drivers::keyboard::bufferlocation - bufStart < (int)regs->edx) {
            drivers::keyboard::manualRead();
            if (drivers::keyboard::buffer[drivers::keyboard::bufferlocation] == '\n') {
                break;
            }
        }
        uint32_t readCharacters = drivers::keyboard::bufferlocation - bufStart;

        drivers::keyboard::bufferlocation = -1;
        s_memcpy((char *)regs->ecx, &drivers::keyboard::buffer[bufStart + 1], readCharacters);
        current_context->eax = readCharacters;
    } else if (regs->eax == 2) { // sys_fork
        DEBUG_PRINTF("syscall: sys_fork\n");
        multitasking::process *newprocess = multitasking::fork_current_process();
        if (newprocess) {
            newprocess->registerContext.eax = 0;
            current_context->eax = newprocess->pid;
        } else {
            current_context->eax = -1; // this is probably the wrong return value
        }
    } else if (regs->eax == 11) { // sys_execve
        DEBUG_PRINTF("syscall: sys_execve\n");
        printf("%s\n", regs->ebx);
    } else if (regs->eax == 199) { // getuid32
        DEBUG_PRINTF("syscall: getuid32\n");
        current_context->eax = 0;
    } else if (regs->eax == 195) { // stat64
        DEBUG_PRINTF("syscall: sys_stat64\n");
        printf("%s\n", regs->ebx);
    } else if (regs->eax == 7) { // sys_waitpid
        DEBUG_PRINTF("syscall: sys_waitpid - PID: %d\n", regs->ebx);

        multitasking::context tempContextStore;
        memcpy((char *)&tempContextStore, (char *)current_context, sizeof(multitasking::context));

        asm("sti"); // TODO: figure out why this crashes when kernel memory runs out

        multitasking::waitForProcess(regs->ebx);

        asm("cli");
        memcpy((char *)current_context, (char *)&tempContextStore, sizeof(multitasking::context));

    } else {
        DEBUG_PRINTF("Unknown syscall %u\n", regs->eax);
        current_context->eax = -1; // return error - once again not the right return code
    }
}