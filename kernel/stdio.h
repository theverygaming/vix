#pragma once
#include "types.h"

void clrscr();
void putc(char c);
void puts(const char* str);
void putcolor(int x, int y, uint8_t color);
void printf(const char* fmt, ...);
void print_buffer(const char* msg, const void* buffer, uint32_t count);