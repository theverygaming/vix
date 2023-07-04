#include <libgcc_math.h>
#include <types.h>

extern "C" {

unsigned int __mulsi3(unsigned int a, unsigned int b) {
    return mul(a, b);
}

unsigned long long __muldi3(unsigned long long a, unsigned long long b) {
    return mul(a, b);
}

unsigned int __udivsi3(unsigned int a, unsigned int b) {
    return div(a, b);
}

int __modsi3(int a, int b) {
    return mod(a, b);
}

unsigned int __umodsi3(unsigned int a, unsigned int b) {
    return mod(a, b);
}
}
