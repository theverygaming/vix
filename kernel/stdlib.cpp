#include "stdlib.h"
#include "stdio.h"
char* memcpy(char* dst, char* src, int n)
{
	char* p = dst;
	while (n--)
		*dst++ = *src++;
	return p;
}

char* memset(char* dst, char value, int n)
{
	char* p = dst;
	while (n--)
		*dst++ = value;
	return p;
}