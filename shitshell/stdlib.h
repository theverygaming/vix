#pragma once
#include "types.h"

void* memcpy(void* dest, const void* src, size_t n);
int strcmp(const char* str1, const char* str2);
size_t strlen(const char* str);

typedef char* va_list;
#define va_start(v,l)  __builtin_va_start((v),(l))
#define va_arg		   __builtin_va_arg
#define va_end		   __builtin_va_end

