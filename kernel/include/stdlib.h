#pragma once
#include <types.h>

namespace stdlib {
    void *memcpy(void *dst, const void *src, size_t n);
    void *memset(void *ptr, int value, size_t n);
    int memcmp(const void *ptr1, const void *ptr2, size_t num);

    int strcmp(const char *str1, const char *str2);
    size_t strlen(const char *str);
    char *strcpy(char *destination, const char *source);
    char *strstr(char *str1, char *str2);
}
