#include "multitasking.h"
#include "stdio.h"
#include "stdlib.h"
#include "cpubasics.h"
#include "../config.h"

multitasking::context *current_context = (multitasking::context*)(KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET);

multitasking::process processes[10] = {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, false}; 

int counter = 0;

int currentProcess = 0;
int processCounter = 0;

void init_empty_stack(void* stackadr, void* codeadr) {
    uint32_t* stack = (uint32_t*)stackadr;
    stack[0] = (uint32_t)codeadr; // EIP
    stack[1] = 8; // CS?
    stack[2] = 1 << 9; // EFLAGS, set interrupt bit
    /*stack[3] = (uint32_t)0xFFFFFFFF; // cause a page fault
    stack[4] = 8; // CS?
    stack[5] = 1 << 9; // EFLAGS, set interrupt bit*/
    stack[3] = 0; // argc
    stack[4] = 0; // NULL
    stack[5] = 0; // envp
    stack[6] = 0; // NULL
}

int initcounter = 0;

multitasking::process* multitasking::getCurrentProcess() {
    return &processes[currentProcess];
}

multitasking::process* multitasking::fork_process(multitasking::process* process) {
    
}

void multitasking::killCurrentProcess() {
    processes[currentProcess].run = false;
    printf("Killed PID %u\n", processes[currentProcess].pid);
    interruptTrigger();
}

void multitasking::create_task(void* stackadr, void* codeadr) {
    stackadr -= (4 * 7); // init_empty_stack has to build the stack up
    init_empty_stack(stackadr, codeadr);
    for(int i = 0; i < 10; i++) {
        if(!processes[i].run) {
            processes[i] = {i, {0, 0, 0, 0, 0, 0, (uint32_t)stackadr, 0}, 0, true};
            break;
        }
    }
}

void multitasking::interruptTrigger() {
    if(initcounter < 200) {
        initcounter++;
        return;
    }
    
    if(counter == 0) {
        processes[0] = {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, true};
        memcpy((char*)&processes[0].registerContext, (char*)current_context, sizeof(context));
        counter = 19;
        currentProcess = 0;
        printf("---Multitasking enabled---\n");
    }
    memcpy((char*)&processes[currentProcess].registerContext, (char*)current_context, sizeof(context)); // Save current process context
    int runningProcesses = 0;
    for(int i = 0; i < 10; i++) {
        if(processes[i].run) {
            runningProcesses++;
        }
    }
    if(runningProcesses == 0) {
        printf("PANIK: All processes died. Halting system\n");
        asm("hlt");
    }
    if(!processes[currentProcess].run) {
        for(int i = 0; i < 10; i++) {
                if(processes[i].run && currentProcess != i) {
                    memcpy((char*)current_context, (char*)&processes[i].registerContext, sizeof(context));
                    currentProcess = i;
                    break;
                }
            }
    }
    if(processCounter > processes[currentProcess].priority) {
        // Priority exceeded, now we have to switch process
        if(runningProcesses > 1) { // is it even possible to switch?
            int start = currentProcess;
            if(currentProcess == runningProcesses - 1) { start = 0; }
            for(int i = start; i < 10; i++) {
                if(processes[i].run && currentProcess != i) {
                    memcpy((char*)current_context, (char*)&processes[i].registerContext, sizeof(context));
                    currentProcess = i;
                    break;
                }
            }
        }
        processCounter = 0;
    }
    processCounter++;
    counter++;
}