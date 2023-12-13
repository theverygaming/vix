#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

// non-standard
char *itoa(size_t value, char *str, size_t base);
char *itoa_signed(ssize_t value, char *str, size_t base);

#ifdef __cplusplus
}
#endif
