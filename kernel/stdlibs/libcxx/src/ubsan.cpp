#include <vix/kprintf.h>
#include <vix/panic.h>
#include <vix/stdio.h>

// #define __ubsanfunc __attribute__((noreturn)) __attribute__((no_instrument_function))
#define __ubsanfunc __attribute__((no_instrument_function))

#define KERNEL_PANIC(x) \
    do {                \
    } while (1)

#define UBSAN_GENERIC()                                                                            \
    do {                                                                                           \
        kprintf(KP_EMERG, "UBSAN -- %s -- caller: 0x%p\n", __func__, __builtin_return_address(0)); \
        KERNEL_PANIC("ubsan");                                                                     \
    } while (0)

#define UBSAN_NOPANIC()                                                                            \
    do {                                                                                           \
        kprintf(KP_ALERT, "UBSAN -- %s -- caller: 0x%p\n", __func__, __builtin_return_address(0)); \
    } while (0)

extern "C" void __ubsanfunc __ubsan_handle_pointer_overflow() {
    UBSAN_GENERIC();
}

extern "C" void __ubsanfunc __ubsan_handle_type_mismatch_v1() {
    // UBSAN_NOPANIC();
}

extern "C" void __ubsanfunc __ubsan_handle_out_of_bounds() {
    UBSAN_GENERIC();
}

extern "C" void __ubsanfunc __ubsan_handle_sub_overflow() {
    UBSAN_GENERIC();
}
extern "C" void __ubsanfunc __ubsan_handle_add_overflow() {
    UBSAN_GENERIC();
}
extern "C" void __ubsanfunc __ubsan_handle_vla_bound_not_positive() {
    UBSAN_GENERIC();
}

extern "C" void __ubsanfunc __ubsan_handle_shift_out_of_bounds() {
    UBSAN_GENERIC();
}

extern "C" void __ubsanfunc __ubsan_handle_mul_overflow() {
    UBSAN_GENERIC();
}

extern "C" void __ubsanfunc __ubsan_handle_load_invalid_value() {
    UBSAN_GENERIC();
}

extern "C" void __ubsanfunc __ubsan_handle_divrem_overflow() {
    UBSAN_GENERIC();
}

extern "C" void __ubsanfunc __ubsan_handle_negate_overflow() {
    UBSAN_GENERIC();
}

extern "C" void __ubsanfunc __ubsan_handle_missing_return() {
    UBSAN_GENERIC();
}
