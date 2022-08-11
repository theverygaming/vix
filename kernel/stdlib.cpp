#include "stdlib.h"
#include <arch/x86/paging.h>
#include "stdio.h"

char *memcpy(char *dst, const char *src, int n) {
    char *p = dst;
    while (n--)
        *dst++ = *src++;
    return p;
}

char *s_memcpy(char *dst, const char *src, int n) {
    char *p = dst;
    while (n--) { // this could be made a lot faster
        if (!(paging::is_readable(dst) && paging::is_readable(src))) {
            printf("s_memcpy page issue, src: 0x%p dest: 0x%p\n", src, dst);
            break;
        }
        *dst++ = *src++;
    }
    return p;
}

uint8_t *memset(uint8_t *dst, uint8_t value, uint32_t n) {
    uint8_t *p = dst;
    while (n--)
        *dst++ = value;
    return p;
}