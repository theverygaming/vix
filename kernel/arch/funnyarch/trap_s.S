.section .text

.global funnyarch_trap_entry
funnyarch_trap_entry:
    mtsr scr0, rsp

    // TODO: switch stacks

    // allocate space on the stack for struct arch::full_ctx
    sub rsp, rsp, #132

    // save GPRs
    str rsp, r0, #128
    str rsp, r1, #124
    str rsp, r2, #120
    str rsp, r3, #116
    str rsp, r4, #112
    str rsp, r5, #108
    str rsp, r6, #104
    str rsp, r7, #100
    str rsp, r8, #96
    str rsp, r9, #92
    str rsp, r10, #88
    str rsp, r11, #84
    str rsp, r12, #80
    str rsp, r13, #76
    str rsp, r14, #72
    str rsp, r15, #68
    str rsp, r16, #64
    str rsp, r17, #60
    str rsp, r18, #56
    str rsp, r19, #52
    str rsp, r20, #48
    str rsp, r21, #44
    str rsp, r22, #40
    str rsp, r23, #36
    str rsp, r24, #32
    str rsp, r25, #28
    str rsp, r26, #24

    // save rf, lr and rfp
    str rsp, rf, #4
    str rsp, lr, #16
    str rsp, rfp, #20

    // save rip
    mfsr r0, irip
    str rsp, r0, #8

    // save original rsp
    mfsr r0, scr0
    str rsp, r0, #12

    // save pcst
    mfsr r0, pcst
    shr r0, r0, #4
    and r0, r0, #0b1111
    str rsp, r0, #0

    mov r0, rsp
    rjal funnyarch_trap // extern "C" void funnyarch_trap(struct arch::full_ctx *ctx)
    rjmp funnyarch_trap_return_stack

// trashes r0 and r1
.Ldisable_hw_interrupts:
    mfsr r0, pcst
    mov r1, #0b1000 // hardware interrupt bit
    not r1, r1
    and r0, r0, r1
    mtsr pcst, r0
    mov rip, lr

// extern "C" void funnyarch_trap_return(struct arch::full_ctx *);
.global funnyarch_trap_return
funnyarch_trap_return:
    mov rsp, r0
funnyarch_trap_return_stack:
    // make sure interrupts are disabled first and foremost. If we get interrupted here there's no recovering (it would erase the state we store in system registers)
    rjal .Ldisable_hw_interrupts

    // prepare pcst to include the new pcst on the state bit stack
    // r1 will be the new state bits on position 1 on the stack
    ldr r1, rsp, #0
    and r1, #0b1111
    shl r1, r1, #4
    // r2 is a mask to unset the pcst bits on the stack in position 1
    mov r2, #0b11110000
    not r2, r2
    mfsr r0, pcst
    and r0, r0, r2 // unset the bits
    or r0, r0, r1 // set our new bits
    mtsr pcst, r0

    // move new rip into irip for iret
    ldr r0, rsp, #8
    mtsr irip, r0

    // restore rf, lr and rfp
    ldr rf, rsp, #4
    ldr lr, rsp, #16
    ldr rfp, rsp, #20

    // restore GPRs
    ldr r26, rsp, #24
    ldr r25, rsp, #28
    ldr r24, rsp, #32
    ldr r23, rsp, #36
    ldr r22, rsp, #40
    ldr r21, rsp, #44
    ldr r20, rsp, #48
    ldr r19, rsp, #52
    ldr r18, rsp, #56
    ldr r17, rsp, #60
    ldr r16, rsp, #64
    ldr r15, rsp, #68
    ldr r14, rsp, #72
    ldr r13, rsp, #76
    ldr r12, rsp, #80
    ldr r11, rsp, #84
    ldr r10, rsp, #88
    ldr r9, rsp, #92
    ldr r8, rsp, #96
    ldr r7, rsp, #100
    ldr r6, rsp, #104
    ldr r5, rsp, #108
    ldr r4, rsp, #112
    ldr r3, rsp, #116
    ldr r2, rsp, #120
    ldr r1, rsp, #124
    ldr r0, rsp, #128

    // restore rsp itself
    ldr rsp, rsp, #12

    // and go!!
    iret
