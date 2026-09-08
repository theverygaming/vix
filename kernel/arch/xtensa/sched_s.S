#include <vix/config.h>

// extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::thread *prev, struct sched::thread *next);
.section .text
.global sched_switch
sched_switch:

#ifdef CONFIG_XTENSA_PLATFORM_ESP32
#ifndef __XTENSA_WINDOWED_ABI__
#error unknown xtensa ABI
#endif
    // FIXME: this is probably completely broken lol
    entry	sp, 32
    mov.n	a7, sp
    // a2: *old
    // a3: *new

    // save sp and ra in *old
    s32i a0, a2, 0
    s32i a1, a2, 4

    // restore ra and sp from *new
    l32i a0, a3, 0
    l32i a1, a3, 4

    retw.n
#endif

#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
    addi a1, a1, -32 // xtensa docs say the stack pointer must be aligned to 16-byte boundaries

    // save all callee-saved registers
    s32i.n a0, a1, 0
    s32i.n a12, a1, 4
    s32i.n a13, a1, 8
    s32i.n a14, a1, 12
    s32i.n a15, a1, 16

    // switch stacks
    s32i.n a1, a2, 0
    mov.n a1, a3

    // restore all callee-saved registers
    l32i.n a15, a1, 16
    l32i.n a14, a1, 12
    l32i.n a13, a1, 8
    l32i.n a12, a1, 4
    l32i.n a0, a1, 0

    addi a1, a1, 32
    ret.n
#endif
