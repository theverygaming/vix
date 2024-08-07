.section .entry

.global _kentry

// vector table
.long 0xfffc           // Initial SP
.long init             // Initial PC
.fill 254, 4, (256*4) // ... all the other vectors point to loop_infinite

loop_infinite:
move.l #0x100000, %a0
move.b #'L', (%a0)
.L1:
jmp .L1

init:
move.w #0x2700, %sr // supervisor set, IPL 7
move.l #0x100000, %a0
move.b #'S', (%a0)
bsr _kentry
jmp loop_infinite
