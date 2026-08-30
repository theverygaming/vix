.section .text

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

.literal_position
.literal .Lxtensa_vectors, xtensa_vectors
// extern "C" void xtensa_setup_vectors();
.global xtensa_setup_vectors
.align 4
xtensa_setup_vectors:
    l32r a2, .Lxtensa_vectors
    wsr.vecbase a2
    ret.n

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
    rsr.epc1 a2
    rsr.epc2 a3
    rsr.epc3 a4
    rsr.exccause a5
    rsr.excvaddr a6
    rsr.interrupt a7
    call0 xtensa_kernel_exception
    j .loop

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
