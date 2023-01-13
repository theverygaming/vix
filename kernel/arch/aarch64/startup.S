/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

// source: https://github.com/bztsrc/raspi3-tutorial/blob/master/11_exceptions/start.S

.section ".data" // these cannot be in the .bss section because that only gets zeroed after the first cpu starts
.align 8
.global _initcpuid
_initcpuid:
.dword 0
.global _initcpuadr
_initcpuadr:
.dword 0
.global _initcpustack
_initcpustack:
.dword 0

.section ".text.boot"

.global _start

_start:    
    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start

    // set up EL1
    mrs     x0, CurrentEL
    and     x0, x0, #12 // clear reserved bits

    // running at EL3?
    cmp     x0, #12
    bne     5f
    // should never be executed, just for completeness
    mov     x2, #0x5b1
    msr     scr_el3, x2
    mov     x2, #0x3c9
    msr     spsr_el3, x2
    adr     x2, 5f
    msr     elr_el3, x2
    eret

    // running at EL2?
5:  cmp     x0, #4
    beq     5f
    msr     sp_el1, x1
    // enable CNTP for EL1
    mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr
    // enable AArch64 in EL1
    mov     x0, #(1 << 31)      // AArch64
    orr     x0, x0, #(1 << 1)   // SWIO hardwired on Pi3
    msr     hcr_el2, x0
    mrs     x0, hcr_el2
    // Setup SCTLR access
    mov     x2, #0x0800
    movk    x2, #0x30d0, lsl #16
    msr     sctlr_el1, x2
    // set up exception handlers
    ldr     x2, =_evt
    msr     vbar_el1, x2
    // change execution level to EL1
    mov     x2, #0x3c4
    msr     spsr_el2, x2
    adr     x2, 5f
    msr     elr_el2, x2
    eret

5:  mov     sp, x1
    // trigger exception on unaligned access
    mrs     x2, sctlr_el1
    orr     x2, x2, #(1 << 1)
    msr     sctlr_el1, x2
    // read cpu id, cpu_init for all cores other than 0
    mrs     x0, mpidr_el1
    and     x0, x0, #0x7
    cbz     x0, 2f
    b       cpu_init
2:

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to C code, should not return
4:  mrs     x0, mpidr_el1
    bl      _kentry
    // for failsafe, halt this core too
1:  wfe
    b       1b

cpu_init:
    wfe
    ldr     x2, =_initcpuid
    ldxr    x1, [x2]
    cmp     x0, x1
    b.ne    cpu_init
    
    ldr     x2, =_initcpustack
    ldxr    x1, [x2]
    mov     sp, x1

    ldr     x2, =_initcpuadr
    ldxr    x1, [x2]

    blr     x1
    b       1b

.macro evt_entry el, n
    .align  7
    mov     x0, #\n
    mrs     x1, esr_el\el
    mrs     x2, elr_el\el
    mrs     x3, spsr_el\el
    mrs     x4, far_el\el
    b       exception_handler
.endm

    .align 11
_evt:
    // Synchronous
    evt_entry 1, 0
    // IRQ
    evt_entry 1, 1
    // FIQ
    evt_entry 1, 2
    // SError
    evt_entry 1, 3
