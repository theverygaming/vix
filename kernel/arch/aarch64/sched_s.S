// extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);
// FIXME: signature CHANGED to extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::task *prev, struct sched::task *next);
.section .text
.global sched_switch
sched_switch:
    // old in x0
    // new in x1

    sub sp, sp, #(8 * 12)
    str lr, [sp, #(8 * 11)]
    str fp, [sp, #(8 * 10)]
    str x28, [sp, #(8 * 9)]
    str x27, [sp, #(8 * 8)]
    str x26, [sp, #(8 * 7)]
    str x25, [sp, #(8 * 6)]
    str x24, [sp, #(8 * 5)]
    str x23, [sp, #(8 * 4)]
    str x22, [sp, #(8 * 3)]
    str x21, [sp, #(8 * 2)]
    str x20, [sp, #(8 * 1)]
    str x19, [sp, #(8 * 0)]

    mov x2, sp
    str x2, [x0]
    mov sp, x1

    ldr x19, [sp, #(8 * 0)]
    ldr x20, [sp, #(8 * 1)]
    ldr x21, [sp, #(8 * 2)]
    ldr x22, [sp, #(8 * 3)]
    ldr x23, [sp, #(8 * 4)]
    ldr x24, [sp, #(8 * 5)]
    ldr x25, [sp, #(8 * 6)]
    ldr x26, [sp, #(8 * 7)]
    ldr x27, [sp, #(8 * 8)]
    ldr x28, [sp, #(8 * 9)]
    ldr fp, [sp, #(8 * 10)]
    ldr lr, [sp, #(8 * 11)]
    add sp, sp, #(8 * 12)

    ret
