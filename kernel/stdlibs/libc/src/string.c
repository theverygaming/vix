#include <string.h>

// TODO: make memcpy, memmove etc. comply with standard C

void *memcpy(void *restrict dest, const void *restrict src, size_t count) {
    uint8_t *p = (uint8_t *)dest;
    const uint8_t *src_u = (const uint8_t *)src;
    while (count--) {
        *p++ = *src_u++;
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t count) {
    uint8_t *_src = (uint8_t *)src;
    uint8_t *_dest = (uint8_t *)dest;

    if (_src == _dest) {
        return dest;
    }

    if (_dest < _src) {
        while (count) {
            *_dest++ = *_src++;
            count--;
        }
    } else {
        while (count) {
            count--;
            _dest[count] = _src[count];
        }
    }

    return dest;
}

void *memset(void *dest, int ch, size_t count) {
    uint8_t *p = (uint8_t *)dest;
    while (count--)
        *p++ = (uint8_t)ch;
    return dest;
}

int bcmp(const void *lhs, const void *rhs, size_t count) {
    return memcmp(lhs, rhs, count);
}

int memcmp(const void *lhs, const void *rhs, size_t count) {
    uint8_t *c1 = (uint8_t *)lhs;
    uint8_t *c2 = (uint8_t *)rhs;
    while (count--) {
        if (*c1++ != *c2++) {
            return c1[-1] < c2[-1] ? -1 : 1;
        }
    }
    return 0;
}

size_t strlen(const char *str) {
    const char *str2 = str;
    while (*str2) {
        str2++;
    }
    return (str2 - str);
}

int strcmp(const char *lhs, const char *rhs) {
    while (*lhs && (*lhs == *rhs)) {
        lhs++;
        rhs++;
    }
    return *((const unsigned char *)lhs) - *((const unsigned char *)rhs);
}

int strncmp(const char *lhs, const char *rhs, size_t count) {
    size_t i;
    for (i = 0; (i < count) && *lhs; i++) {
        if (*lhs != *rhs) {
            return *((const unsigned char *)lhs) - *((const unsigned char *)rhs);
            break;
        }
        lhs++;
        rhs++;
    }
    if (i == count) {
        return 0;
    }
    return *((const unsigned char *)lhs) - *((const unsigned char *)rhs);
}

char *strstr(const char *str, const char *substr) {
    size_t substrlen = strlen(substr);
    while (*str) {
        if (!memcmp(str++, substr, substrlen)) {
            return str - 1;
        }
    }
    return 0;
}

size_t strcspn(const char *dest, const char *src) {
    size_t n = 0;
    if (*src == 0) {
        return 0;
    }
    while (*dest) {
        if (strchr((char *)src, *dest)) {
            return n;
        }
        dest++;
        n++;
    }
    return n;
}

char *strchr(const char *str, int ch) {
    while (*str) {
        if (*str == ch) {
            return str;
        }
        str++;
    }
    if (*str == ch) {
        return str;
    }
    return 0;
}

unsigned long strtoul(const char *restrict str, char **restrict str_end, int base) {
    unsigned long int num = 0;
    while (*str) {
        if (*str < '0' || *str > '9') {
            break;
        }
        uint8_t current_num = *str - '0';
        num = (num * base) + current_num;
        str++;
    }
    if (str_end != 0) {
        *str_end = (char *)str;
    }
    return num;
}
