#include <panic.h>
#include <stdio.h>

// #define __ubsanfunc __attribute__((noreturn)) __attribute__((no_instrument_function))
#define __ubsanfunc __attribute__((no_instrument_function))

#define KERNEL_PANIC(x) \
    do {                \
    } while (0)

extern "C" void __ubsanfunc __ubsan_handle_pointer_overflow() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}

extern "C" void __ubsanfunc __ubsan_handle_type_mismatch_v1() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}

extern "C" void __ubsanfunc __ubsan_handle_out_of_bounds() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}

extern "C" void __ubsanfunc __ubsan_handle_sub_overflow() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}
extern "C" void __ubsanfunc __ubsan_handle_add_overflow() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}
extern "C" void __ubsanfunc __ubsan_handle_vla_bound_not_positive() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}

extern "C" void __ubsanfunc __ubsan_handle_shift_out_of_bounds() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}

extern "C" void __ubsanfunc __ubsan_handle_mul_overflow() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}

extern "C" void __ubsanfunc __ubsan_handle_load_invalid_value() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}

extern "C" void __ubsanfunc __ubsan_handle_divrem_overflow() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}

extern "C" void __ubsanfunc __ubsan_handle_negate_overflow() {
    puts(__func__);
    puts("\n");
    KERNEL_PANIC("ubsan");
}
