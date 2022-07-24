#pragma once
#include "../../kernel/types.h"

#define outb(port, value) asm volatile("outb %%al, %%dx" ::"d"(port), "a"(value))

void clrscr();
void putc(char c);
void puts(const char *str);
void printf(const char *fmt, ...);
void print_buffer(const char *msg, const void *buffer, uint32_t count);