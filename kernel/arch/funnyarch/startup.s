.section .entry
__kentry:
// set up interrupt/exception handler
mov r0, __except_early
or r0, r0, #0x1 // no jump table
mtsr ibptr, r0
// init stack
mov rsp, #0x0000
movh rsp, #0x0010
mov rfp, #0
// print that we got so far + test stack
mov r0, #0xB000
movh r0, #0xF004
mov r1, #97 // 97 = a
strpi rsp, r1, #-4
mov r1, #0x45 // 0x45 = E
ldri r1, rsp, #4
str r0, r1, #0
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
