// extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);
.section .text
.global sched_switch
sched_switch:
#ifndef __XTENSA_WINDOWED_ABI__
#error unknown xtensa ABI
#endif
    entry	sp, 32
    mov.n	a7, sp
    // a2: **old
    // a3: *new

    // save ra on stack
    addi.n a1, a1, -4
    s32i a0, a1, 0

	s32i a1, a2, 0 // save sp in **old

	mov a1, a3 // restore sp from *new

    // restore ra from stack
    l32i a0, a1, 0
    addi.n a1, a1, 4

    retw.n
