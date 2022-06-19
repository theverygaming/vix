#include "multitasking.h"
#include "stdio.h"
#include "stdlib.h"
#include "cpubasics.h"
#include "../config.h"

void task1() {
    int counter = 0;
    uint32_t counterr = 0;
    *((unsigned char *)((KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET) + 2 * 74 + 160 * 5)) += 1;
    while (1)
    {
        *((unsigned char *)((KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET) + 2 * 75 + 160 * 5)) = counter / 20;
        counter++;
        counterr++;
        if(counter == 1000) { counter = 0; }
        if(counterr == 10000000) {
            break;
        }
    }
}

multitasking::context *current_context = (multitasking::context*)(KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET);

multitasking::process current_process;

multitasking::process processes[10] = {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, false}; 

int counter = 0;

int currentProcess = 0;
int processCounter = 0;

void init_empty_stack(void* stackadr, void (*func)()) {
    uint32_t* stack = (uint32_t*)stackadr;
    stack[0] = (uint32_t)func; // EIP
    stack[1] = 8; // CS?
    stack[2] = 1 << 9; // EFLAGS, set interrupt bit
    stack[3] = (uint32_t)0xFFFFFFFF; // cause a page fault
    stack[4] = 8; // CS?
    stack[5] = 1 << 9; // EFLAGS, set interrupt bit
}

int initcounter = 0;

void multitasking::killCurrentProcess() {
    processes[currentProcess].run = false;
    //memcpy((char*)current_context, (char*)&processes[0].registerContext, sizeof(context));
    interruptTrigger();
}

void multitasking::interruptTrigger() {
    if(initcounter < 100) {
        initcounter++;
        return;
    }
    if(counter == 0) {
        processes[0] = {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, true};
        memcpy((char*)&processes[0].registerContext, (char*)current_context, sizeof(context));
        init_empty_stack((void*)0x17D7840, task1);
        processes[1] = {1, {0, 0, 0, 0, 0, 0, 0x17D7840, 0}, 0, true};
        counter = 19;
        currentProcess = 0;
    }
    memcpy((char*)&processes[currentProcess].registerContext, (char*)current_context, sizeof(context)); // Save current process context
    int runningProcesses = 0;
    for(int i = 0; i < 10; i++) {
        if(processes[i].run) {
            runningProcesses++;
        }
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