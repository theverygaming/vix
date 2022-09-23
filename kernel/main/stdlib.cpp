#include <stdlib.h>

void *stdlib::memcpy(void *dst, const void *src, size_t n) {
    uint8_t *p = (uint8_t *)dst;
    const uint8_t *src_u = (const uint8_t *)src;
    while (n--)
        *p++ = *src_u++;
    return dst;
}

void *stdlib::memset(void *ptr, int value, size_t n) {
    uint8_t *p = (uint8_t *)ptr;
    while (n--)
        *p++ = (uint8_t)value;
    return ptr;
}

int stdlib::memcmp(const void *ptr1, const void *ptr2, size_t num) {
    uint8_t *c1 = (uint8_t *)ptr1;
    uint8_t *c2 = (uint8_t *)ptr2;
    while (num--) {
        if (*c1++ != *c2++) {
            return c1[-1] < c2[-1] ? -1 : 1;
        }
    }
    return 0;
}

int stdlib::strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *((const unsigned char *)str1) - *((const unsigned char *)str2);
}

size_t stdlib::strlen(const char *str) {
    const char *str2 = str;
    while (*str2) {
        str2++;
    }
    return (str2 - str);
}

char *stdlib::strcpy(char *destination, const char *source) {
    char *saved_dest = destination;
    while (*source) {
        *destination++ = *source++;
    }
    *destination = *source;
    return saved_dest;
}

char *stdlib::strstr(char *str1, char *str2) {
    size_t str2len = stdlib::strlen(str2);
    while (*str1) {
        if (!stdlib::memcmp(str1++, str2, str2len)) {
            return str1 - 1;
        }
    }
    return 0;
}
