#include "stdlib.h"
char *memcpy(char *dst, const char *src, int n) {
    char *p = dst;
    while (n--)
        *dst++ = *src++;
    return p;
}

char *memset(char *dst, char value, int n) {
    char *p = dst;
    while (n--)
        *dst++ = value;
    return p;
}
