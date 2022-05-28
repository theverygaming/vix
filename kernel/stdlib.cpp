#include "stdlib.h"
uint32_t *memcpy(uint32_t *dst, uint32_t *src, int n)
{
	uint32_t *p = dst;
	while (n--)
		*dst++ = *src++;
	return p;
}