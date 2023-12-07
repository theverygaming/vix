// extern "C" void sched_switch(struct sched::xtensa_sched_ctx *old, struct sched::xtensa_sched_ctx *_new);
.section .text
.global sched_switch
sched_switch:
#ifndef __XTENSA_WINDOWED_ABI__
#error unknown xtensa ABI
#endif
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
