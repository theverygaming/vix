#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memcpy(void *dest, const void *src, size_t count);
int memcmp(const void *lhs, const void *rhs, size_t count);
void *memmove(void *dest, const void *src, size_t count);
void *memset(void *dest, int ch, size_t count);

size_t strlen(const char *str);
int strcmp(const char *lhs, const char *rhs);
int strncmp(const char *lhs, const char *rhs, size_t count);
char *strstr(const char *str, const char *substr);
size_t strcspn(const char *dest, const char *src);
char *strchr(const char *str, int ch);

unsigned long strtoul(const char *str, char **str_end, int base);

#ifdef __cplusplus
}
#endif
