#include "multitasking.h"
#include "stdio.h"
#include "stdlib.h"
#include "cpubasics.h"

void task1() {
    __asm("sti");
    int counterr = 0;
    *((unsigned char *)(0xB8000 + 2 * 74 + 160 * 5)) += 1;
    while (1)
    {
        //printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        //cpubasics::sleep(10);
        *((unsigned char *)(0xB8000 + 2 * 75 + 160 * 5)) = counterr / 20;
        counterr++;
        if(counterr == 1000) { counterr = 0; }
    }
}

//multitasking::context kern_context = {0, 0, 0, 0, 0, 0, 0, 0};
//multitasking::context t1_context = {0, 0, 0, 0, 0, 0, 0, 0};

multitasking::context *kern_context = (multitasking::context*)0x100443C;
multitasking::context *t1_context = (multitasking::context*)0x100343C;

multitasking::context *current_context = (multitasking::context*)0x100043C;

int counter = 0;

void init_empty_stack(uint32_t stackadr, void (*func)()) {
    uint32_t* stack = (uint32_t*)stackadr;
    stack[0] = (uint32_t)func;
    stack[1] = 8; // no clue what this means but it seems important
}

int initcounter = 0;

void multitasking::interruptTrigger() {
    if(initcounter < 1000) {
        initcounter++;
        return;
    }
    if(counter == 0) {
        printf("Switching tasks!");
        memcpy((uint32_t*)kern_context, (uint32_t*)current_context, sizeof(context));
        init_empty_stack(0x17D7840, task1);
        t1_context->esp = 0x17D7840;
        //memcpy((uint32_t*)current_context, (uint32_t*)&old_context, sizeof(context));
        //current_context->esp = 0x17D7840;
        //memcpy((uint32_t*)current_context, (uint32_t*)t1_context, sizeof(context));
        //memcpy((uint32_t*)current_context, (uint32_t*)&kern_context, sizeof(context));
        counter = 19;
    }
    if(counter == 10) {
        memcpy((uint32_t*)t1_context, (uint32_t*)current_context, sizeof(context));
        memcpy((uint32_t*)current_context, (uint32_t*)kern_context, sizeof(context));
        *((unsigned char *)(0xB8000 + 2 * 75 + 160 * 10)) = 'K';
    }
    else if(counter == 20) {
        memcpy((uint32_t*)kern_context, (uint32_t*)current_context, sizeof(context));
        memcpy((uint32_t*)current_context, (uint32_t*)t1_context, sizeof(context));
        *((unsigned char *)(0xB8000 + 2 * 75 + 160 * 10)) = 'T';
        counter = 1;
    }
    counter++;
}