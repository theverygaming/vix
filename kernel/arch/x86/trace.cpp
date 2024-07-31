#include <vix/types.h>

static void __attribute__((no_instrument_function)) e9putd(const void *data, size_t len) {
    const uint8_t *data_u8 = (const uint8_t *)data;
    while (len) {
        asm volatile("outb %%al, %%dx" ::"d"(0xE9), "a"(*data_u8++));
        len--;
    }
}

extern "C" void __attribute__((no_instrument_function)) __cyg_profile_func_enter(void *func, void *caller) {
    e9putd("e", 1);
    e9putd(&func, sizeof(void *));
    e9putd(&caller, sizeof(void *));
}

extern "C" void __attribute__((no_instrument_function)) __cyg_profile_func_exit(void *func, void *caller) {
    e9putd("x", 1);
    e9putd(&func, sizeof(void *));
    e9putd(&caller, sizeof(void *));
}
