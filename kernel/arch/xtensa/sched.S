// extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);
.section .text
.global sched_switch
sched_switch:
    // FIXME
    /*
    move.l 4(%sp), %a0 // old
    move.l 8(%sp), %a1 // new

    move.l %d7, -(%sp)
    move.l %d6, -(%sp)
    move.l %d5, -(%sp)
    move.l %d4, -(%sp)
    move.l %d3, -(%sp)
    move.l %d2, -(%sp)
    move.l %a6, -(%sp)
    move.l %a5, -(%sp)
    move.l %a4, -(%sp)
    move.l %a3, -(%sp)
    move.l %a2, -(%sp)

    move.l %sp, (%a0)
    move.l %a1, %sp

    move.l (%sp)+, %a2
    move.l (%sp)+, %a3
    move.l (%sp)+, %a4
    move.l (%sp)+, %a5
    move.l (%sp)+, %a6
    move.l (%sp)+, %d2
    move.l (%sp)+, %d3
    move.l (%sp)+, %d4
    move.l (%sp)+, %d5
    move.l (%sp)+, %d6
    move.l (%sp)+, %d7
    
    rts
    */
