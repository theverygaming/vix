.section .entry
.global __kentry
__kentry:
// set up interrupt/exception handler
mov r0, __except_early.low
movh r0, __except_early.high
or r0, r0, #0x1 // no jump table
mtsr ibptr, r0
// go!!
jmp _kentry
.Lloop1:
rjmp .Lloop1

__except_early:
mov r0, #0xB000
movh r0, #0xF004
mov r1, #0x45 // 0x45 = E
str r0, r1, #0
.Lloop2:
rjmp .Lloop2
