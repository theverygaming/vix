#include "stdlib.h"
#include "stdio.h"
char* memcpy(char* dst, char* src, int n)
{
	char* p = dst;
	while (n--)
		*dst++ = *src++;
	return p;
}

uint8_t* memset(uint8_t* dst, uint8_t value, uint32_t n)
{
	uint8_t* p = dst;
	while (n--)
		*dst++ = value;
	return p;
}