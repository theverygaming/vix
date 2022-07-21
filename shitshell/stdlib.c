#include "stdlib.h"

void* memcpy(void* dest, const void* src, size_t n) {
    char* dest2 = (char*)dest;
    char* src2 = (char*)src;
	while (n--)
		*dest2++ = *src2++;
	return dest;
}

void* memset(void* ptr, int value, size_t num) {
    char* dest = (char*)ptr;
	while (num--)
		*dest++ = value;
	return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num) {
    uint8_t* c1 = (uint8_t*)ptr1;
    uint8_t* c2 = (uint8_t*)ptr2;
    while(num--) {
        if(*c1++ != *c2++) { return c1[-1] < c2[-1] ? -1 : 1; }
    }
    return 0;
}

int strcmp(const char* str1, const char* str2) {
    while(true) {
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

char* strchr(char* str, int character) {
    while(*str) {
        if(*str == character) {
            return str;
        }
        str++;
    }
    if(*str == character) { return str; }
    return 0;
}

size_t strcspn(const char* s1, const char* s2) {
    size_t n = 0;
    if(*s2 == 0) { return 0; }
    while(*s1) {
        if(strchr(s2, *s1)) {
            return n;
        }
        s1++;
        n++;
    }
    return n;
}

char* strstr(const char* str1, const char* str2) {
    size_t str2s = strlen(str2);
    while(*str1) {
        if(!memcmp(str1++, str2, str2s)) { return str1 -1; }
    }
    return 0;
}

size_t sscanf(const char* str, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    size_t instrcnt = 0;
    size_t items_filled = 0;
    while (*fmt) {
        if(*fmt == '%') {
            fmt++;
            switch(*fmt) {
                case 's': {
                    char fmtcut[100];
                    size_t cspn = strcspn(fmt +1, "%");
                    if(cspn >= 100) { return items_filled; } 
                    memcpy(fmtcut, fmt + 1, cspn);
                    fmtcut[cspn] = '\0';
                    
                    size_t cpcnt = (size_t)strstr(&str[instrcnt], fmtcut);
                    if(!cpcnt) { return items_filled; }
                    cpcnt -= (size_t)&str[instrcnt];
                    char* arg = va_arg(args, char*);
                    memcpy((void*)arg, (void*)&str[instrcnt], cpcnt);
                    instrcnt += cpcnt - 1;

                    items_filled++;
                    break;
                }
            }
        }
        instrcnt++;
        fmt++;
    }

    va_end(args);
    return items_filled;
}


double sin(double x) {
    float xf = (float)x;
    float* xfp = &xf;
    asm volatile (
    "flds (%0)\n\t" // load into ST(0)
    "fsin\n\t"
    "fstps (%1)\n\t" // pop off ST(0)
    : "=r" (xfp)
    : "r" (xfp));
    return (double)xf;
}

double cos(double x) {
    float xf = (float)x;
    float* xfp = &xf;
    asm volatile (
    "flds (%0)\n\t" // load into ST(0)
    "fcos\n\t"
    "fstps (%1)\n\t" // pop off ST(0)
    : "=r" (xfp)
    : "r" (xfp));
    return (double)xf;
}