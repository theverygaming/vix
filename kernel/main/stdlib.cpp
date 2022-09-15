#include <stdlib.h>
#include <stdio.h>

char *memcpy(char *dst, const char *src, int n) {
    char *p = dst;
    while (n--)
        *dst++ = *src++;
    return p;
}

uint8_t *memset(uint8_t *dst, uint8_t value, uint32_t n) {
    uint8_t *p = dst;
    while (n--)
        *dst++ = value;
    return p;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
    uint8_t *c1 = (uint8_t *)ptr1;
    uint8_t *c2 = (uint8_t *)ptr2;
    while (num--) {
        if (*c1++ != *c2++) {
            return c1[-1] < c2[-1] ? -1 : 1;
        }
    }
    return 0;
}

int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *((const unsigned char *)str1) - *((const unsigned char *)str2);
}

size_t strlen(const char *str) {
    const char *str2 = str;
    while (*str2) {
        str2++;
    }
    return (str2 - str);
}

char *strcpy(char *destination, const char *source) {
    char *saved_dest = destination;
    while (*source) {
        *destination++ = *source++;
    }
    *destination = *source;
    return saved_dest;
}

char *strstr(char *str1, char *str2) {
    size_t str2len = strlen(str2);
    while (*str1) {
        if (!memcmp(str1++, str2, str2len)) {
            return str1 - 1;
        }
    }
    return 0;
}
