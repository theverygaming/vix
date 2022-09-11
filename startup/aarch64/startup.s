.section ".text.boot"

.global _start

_start:
    // get cpuid, stop other cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, cpu0
    b halt
cpu0:
    // stack 
    mov     x1, 0x6969
    mov     sp, x1

    // note -> clear BSS

4:  bl      main
    b halt

halt:
    wfe
    b halt
