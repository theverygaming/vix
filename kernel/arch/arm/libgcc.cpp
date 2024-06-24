#include <libgcc_math.h>
#include <types.h>
#include <panic.h>

extern "C" {

long __aeabi_lmul(long a, long b) {
    return mul(a, b);
}

unsigned int __aeabi_uidivmod(unsigned int a, unsigned int b) {
    return mod(a, b);
}

int __aeabi_idivmod(int a, int b) {
    return mod(a, b);
}

unsigned int __aeabi_uidiv(unsigned int a, unsigned int b) {
    return div(a, b);
}

void __aeabi_atexit() {
    KERNEL_PANIC("__aeabi_atexit");
}
}
