#pragma once
#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef __builtin_va_list va_list;
#define va_start(v, l)    __builtin_va_start(v, l)
#define va_end(v)         __builtin_va_end(v)
#define va_arg(v, l)      __builtin_va_arg(v, l)
#define va_copy(dst, src) __builtin_va_copy(dst, src)

void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *ptr, int value, size_t n);

int memcmp(const void *ptr1, const void *ptr2, size_t num);
void *memmove(void *dest, const void *src, size_t n);

unsigned long int strtoul(const char *str, char **endptr, int base);

int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t num);
size_t strlen(const char *str);
char *strcpy(char *destination, const char *source);
char *strstr(char *str1, char *str2);
size_t strcspn(const char *s1, const char *s2);
char *strchr(char *str, int character);

char *itoa(size_t value, char *str, size_t base);
char *itoa_signed(ssize_t value, char *str, size_t base);

#ifdef __cplusplus
}
#endif
