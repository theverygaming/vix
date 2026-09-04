.section .iram

.align 128 // ESP8266 hardwires the bottom 7 bits of VECBASE to 0
xtensa_vectors:
    // vector offsets: https://github.com/espressif/xtensa-overlays/blob/dd1cf19f6eb327a9db51043439974a6de13f5c7f/xtensa_esp8266/newlib/newlib/libc/sys/xtensa/include/xtensa/config/core-isa.h#L400-L420
    // Interrupt level 2, Debug
    .org xtensa_vectors + 0x10
    j .vector_debug
    // Interrupt level 3, NMI
    .org xtensa_vectors + 0x20
    j .vector_nmi
    // KernelExceptionVector
    .org xtensa_vectors + 0x30
    j .vector_kernel_exception
    // UserExceptionVector
    .org xtensa_vectors + 0x50
    j .vector_user_exception
    // DoubleExceptionVector
    .org xtensa_vectors + 0x70
    j .vector_double_exception

.align 4
.vector_debug:
    rsr.epc1 a2
    rsr.epc2 a3
    rsr.epc3 a4
    rsr.exccause a5
    rsr.debugcause a6
    rsr.interrupt a7
    call0 xtensa_vector_debug
    j .loop

.align 4
.vector_nmi:
    rsr.epc1 a2
    rsr.epc2 a3
    rsr.epc3 a4
    rsr.exccause a5
    rsr.excvaddr a6
    rsr.interrupt a7
    call0 xtensa_vector_nmi
    j .loop

.align 4
.vector_kernel_exception:
    wsr.excsave1 a1 // a1 (sp) -> excsave1 for .common_exception
    j .common_exception

.align 4
.common_exception:
    // reserve stack space
    addi a1, a1, -96

    // save all context to stack
    // aregs
    s32i.n a0, a1, 0
    rsr.excsave1 a0 // read original a1 (sp) into a0
    s32i.n a0, a1, 4
    s32i.n a2, a1, 8
    s32i.n a3, a1, 12
    s32i.n a4, a1, 16
    s32i.n a5, a1, 20
    s32i.n a6, a1, 24
    s32i.n a7, a1, 28
    s32i.n a8, a1, 32
    s32i.n a9, a1, 36
    s32i.n a10, a1, 40
    s32i.n a11, a1, 44
    s32i.n a12, a1, 48
    s32i.n a13, a1, 52
    s32i.n a14, a1, 56
    s32i.n a15, a1, 60

    // special regs
    rsr.epc1 a0
    s32i a0, a1, 64
    rsr.sar a0
    s32i a0, a1, 68
    rsr.ps a0
    s32i a0, a1, 72
    rsr.exccause a0
    s32i a0, a1, 76
    rsr.excvaddr a0
    s32i a0, a1, 80
    rsr.debugcause a0
    s32i a0, a1, 84

    // ensure the CPU is in the correct mode
    rsr.ps a0
    // unset user mode bit
    movi.n a2, (1 << 5)
    movi.n a3, -1
    xor a2, a2, a3
    and a0, a0, a2
    wsr.ps a0

    mov.n a2, a1
    call0 xtensa_common_exception
    j xtensa_restore_full_ctx

.align 4
.global xtensa_restore_full_ctx
xtensa_restore_full_ctx:
    // restore all context and return from interrupt
    // special regs
    l32i.n a0, a1, 72
    wsr.ps a0
    l32i.n a0, a1, 68
    wsr.sar a0
    l32i.n a0, a1, 64
    wsr.epc1 a0

    // aregs
    l32i.n a15, a1, 60
    l32i.n a14, a1, 56
    l32i.n a13, a1, 52
    l32i.n a12, a1, 48
    l32i.n a11, a1, 44
    l32i.n a10, a1, 40
    l32i.n a9, a1, 36
    l32i.n a8, a1, 32
    l32i.n a7, a1, 28
    l32i.n a6, a1, 24
    l32i.n a5, a1, 20
    l32i.n a4, a1, 16
    l32i.n a3, a1, 12
    l32i.n a2, a1, 8
    l32i.n a0, a1, 0
    // restore original a1 (sp)
    l32i.n a1, a1, 4

    rfe

.align 4
.vector_user_exception:
    rsr.epc1 a2
    rsr.epc2 a3
    rsr.epc3 a4
    rsr.exccause a5
    rsr.excvaddr a6
    rsr.interrupt a7
    call0 xtensa_user_exception
    j .loop

.align 4
.vector_double_exception:
    rsr.epc1 a2
    rsr.epc2 a3
    rsr.epc3 a4
    rsr.exccause a5
    rsr.excvaddr a6
    rsr.depc a7
    call0 xtensa_double_exception
    j .loop

.align 4
.loop:
    j .loop


.section .text

.literal_position
.literal .Lxtensa_vectors, xtensa_vectors
// extern "C" void xtensa_setup_vectors();
.global xtensa_setup_vectors
.align 4
xtensa_setup_vectors:
    l32r a2, .Lxtensa_vectors
    wsr.vecbase a2
    ret.n
