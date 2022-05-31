#include "multitasking.h"
#include "stdio.h"
#include "stdlib.h"
#include "cpubasics.h"

void task1() {
    int counter = 0;
    uint32_t counterr = 0;
    *((unsigned char *)(0xB8000 + 2 * 74 + 160 * 5)) += 1;
    while (1)
    {
        *((unsigned char *)(0xB8000 + 2 * 75 + 160 * 5)) = counter / 20;
        counter++;
        counterr++;
        if(counter == 1000) { counter = 0; }
        if(counterr == 10000000) {
            break;
        }
    }
}

int died = 0;
void end() {
    printf("process died!\n");
    died = 1;
    asm("int $32");
}

multitasking::context *kern_context = (multitasking::context*)0x100443C;
multitasking::context *t1_context = (multitasking::context*)0x100343C;

multitasking::context *current_context = (multitasking::context*)0x100043C;

int counter = 0;

void init_empty_stack(uint32_t stackadr, void (*func)()) {
    uint32_t* stack = (uint32_t*)stackadr;
    stack[0] = (uint32_t)func; // EIP
    stack[1] = 8; // CS?
    stack[2] = 1 << 9; // EFLAGS, set interrupt bit
    stack[3] = (uint32_t)end;
    stack[4] = 8; // CS?
    stack[5] = 1 << 9; // EFLAGS, set interrupt bit
}

int initcounter = 0;

extern "C" void task2();

void multitasking::interruptTrigger() {
    if(initcounter < -1) {
        initcounter++;
        return;
    }
    if(counter == 0) {
        //printf("Switching tasks!");
        memcpy((uint32_t*)kern_context, (uint32_t*)current_context, sizeof(context));
        init_empty_stack(0x17D7840, task1);
        t1_context->esp = 0x17D7840;
        counter = 19;
    }
    if(died == 1) {
        memcpy((uint32_t*)current_context, (uint32_t*)kern_context, sizeof(context));
    }
    if(counter == 10) {
        if(died == 0) {
            memcpy((uint32_t*)t1_context, (uint32_t*)current_context, sizeof(context));
        }
        memcpy((uint32_t*)current_context, (uint32_t*)kern_context, sizeof(context));
        *((unsigned char *)(0xB8000 + 2 * 75 + 160 * 10)) = 'K';
    }
    else if(counter == 20) {
        memcpy((uint32_t*)kern_context, (uint32_t*)current_context, sizeof(context));
        if(died == 0) {
            memcpy((uint32_t*)current_context, (uint32_t*)t1_context, sizeof(context));
        }
        *((unsigned char *)(0xB8000 + 2 * 75 + 160 * 10)) = 'T';
        counter = 1;
    }
    counter++;
}