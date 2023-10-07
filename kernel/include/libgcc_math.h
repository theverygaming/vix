#pragma once

// https://en.wikipedia.org/wiki/Ancient_Egyptian_multiplication#Russian_peasant_multiplication
template <class T> inline T mul(T a, T b) {
    T p = 0;
    while (a != 0) {
        if ((a & 0x1) != 0) {
            p += b;
        }
        a >>= 1;
        b <<= 1;
    }
    return p;
}

template <class T> inline T div(T a, T b) {
    if (b == 0) { // bro really tryin to divide by zero :skull:
        return 0;
    }
    T r = a;
    T q = 0;
    while (r >= b) {
        r -= b;
        q++;
    }
    return q;
}

template <class T> inline T mod(T a, T b) {
    if (b == 0) { // bro really tryin to divide by zero :skull:
        return 0;
    }
    T r = a;
    T q = 0;
    while (r >= b) {
        r -= b;
        q++;
    }
    return r;
}
