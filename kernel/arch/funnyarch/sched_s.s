.section .text
// extern "C" void funnyarch_sched_switch(struct arch::ctx **old, struct arch::ctx *_new);
.global funnyarch_sched_switch
funnyarch_sched_switch:
    strpi rsp, r8, #-4
	strpi rsp, r9, #-4
	strpi rsp, r10, #-4
	strpi rsp, r11, #-4
	strpi rsp, r12, #-4
    strpi rsp, r13, #-4
    strpi rsp, r14, #-4
    strpi rsp, r15, #-4
    strpi rsp, r16, #-4
    strpi rsp, r17, #-4
    strpi rsp, r18, #-4
    strpi rsp, r19, #-4
    strpi rsp, r20, #-4
    strpi rsp, r21, #-4
    strpi rsp, r22, #-4
    strpi rsp, r23, #-4
    strpi rsp, r24, #-4
    strpi rsp, r25, #-4
    strpi rsp, r26, #-4
    strpi rsp, rfp, #-4
    strpi rsp, lr, #-4

    // swap stacks
    str r0, rsp, #0
    mov rsp, r1

    ldri lr, rsp, #4
    ldri rfp, rsp, #4
    ldri r26, rsp, #4
    ldri r25, rsp, #4
    ldri r24, rsp, #4
    ldri r23, rsp, #4
    ldri r22, rsp, #4
    ldri r21, rsp, #4
    ldri r20, rsp, #4
    ldri r19, rsp, #4
    ldri r18, rsp, #4
    ldri r17, rsp, #4
    ldri r16, rsp, #4
    ldri r15, rsp, #4
    ldri r14, rsp, #4
    ldri r13, rsp, #4
    ldri r12, rsp, #4
    ldri r11, rsp, #4
    ldri r10, rsp, #4
    ldri r9, rsp, #4
    ldri r8, rsp, #4

    mov rip, lr
