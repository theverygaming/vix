extern "C" {

// https://en.wikipedia.org/wiki/Ancient_Egyptian_multiplication#Russian_peasant_multiplication
unsigned int __mulsi3(unsigned int a, unsigned int b) {
    unsigned int p = 0;

    while (a != 0) {
        if (a & 0x1 != 0) {
            p += b;
        }
        a >>= 1;
        b <<= 1;
    }
    return p;
}

unsigned int __udivsi3(unsigned int a, unsigned int b) {
    if (a <= 0xFFFF && b <= 0xFFFF) { // use 16 bit if possible
        return (unsigned short)a / (unsigned short)b;
    }

    unsigned int r = a;
    unsigned int q = 0;

    while (b <= r) {
        r -= b;
        q++;
    }

    return q;
}

unsigned int __umodsi3(unsigned int a, unsigned int b) {
    if (a <= 0xFFFF && b <= 0xFFFF) { // use 16 bit if possible
        return (unsigned short)a % (unsigned short)b;
    }

    unsigned int r = a;
    unsigned int q = 0;

    while (b <= r) {
        r -= b;
        q++;
    }

    return r;
}
}
