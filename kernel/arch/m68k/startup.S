.section .entry

.global _kentry

// vector table
.long 0xfffc           // Initial SP
.long init             // Initial PC
.fill 254, 4, (256*4) // ... all the other vectors point to loop_infinite

loop_infinite:
move.l #0xef00, %a0
move.b #'L', (%a0)
jmp loop_infinite

init:
move.w #0x2600, %sr // supervisor set, IPL 6
move.l #0xef00, %a0
move.b #'S', (%a0)
bsr _kentry
jmp loop_infinite
