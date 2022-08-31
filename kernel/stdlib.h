#pragma once
#include "types.h"
char *memcpy(char *dst, const char *src, int n);
char *s_memcpy(char *dst, const char *src, int n);
uint8_t *memset(uint8_t *dst, uint8_t value, uint32_t n);
int memcmp(const void *ptr1, const void *ptr2, size_t num);

int strcmp(const char *str1, const char *str2);
size_t strlen(const char *str);
char *strcpy(char *destination, const char *source);
char *strstr(char *str1, char *str2);