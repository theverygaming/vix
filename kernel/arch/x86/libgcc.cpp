#include <vix/libgcc_math.h>
#include <vix/types.h>

extern "C" {

unsigned long __udivdi3(unsigned long a, unsigned long b) {
    return div(a, b);
}

unsigned long __umoddi3(unsigned long a, unsigned long b) {
    return mod(a, b);
}

}
