#include <panic.h>
#include <types.h>

uintptr_t __stack_chk_guard = (uintptr_t)0x244E62F0EE8AA49F;

extern "C" void __attribute__((noreturn)) __attribute__((no_instrument_function)) __stack_chk_fail() {
    KERNEL_PANIC("stack smashing detected");
}
