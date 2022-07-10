#include "stdlib.h"

void* memcpy(void* dest, const void* src, size_t n) {
    char* dest2 = (char*)dest;
    char* src2 = (char*)src;
	while (n--)
		*dest2++ = *src2++;
	return dest;
}

int strcmp(const char* str1, const char* str2) {
    while(1) {
        if((*str1 == 0) && (*str2 == 0)) { return 0; }
        else if(*str1 != *str2) { return *str1 - *str2; }
        str1++;
        str2++;
    }
}

size_t strlen(const char* str) {
    const char* str2 = str;
    while(*str2) { str2++; }
    return (str2 - str);
}

