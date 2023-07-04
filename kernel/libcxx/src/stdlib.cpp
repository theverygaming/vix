#include <stdlib.h>

extern "C" {

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *p = (uint8_t *)dst;
    const uint8_t *src_u = (const uint8_t *)src;
    while (n--)
        *p++ = *src_u++;
    return dst;
}

void *memset(void *ptr, int value, size_t n) {
    uint8_t *p = (uint8_t *)ptr;
    while (n--)
        *p++ = (uint8_t)value;
    return ptr;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
    uint8_t *c1 = (uint8_t *)ptr1;
    uint8_t *c2 = (uint8_t *)ptr2;
    while (num--) {
        if (*c1++ != *c2++) {
            return c1[-1] < c2[-1] ? -1 : 1;
        }
    }
    return 0;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *_src = (uint8_t *)src;
    uint8_t *_dest = (uint8_t *)dest;

    if (_src == _dest) {
        return dest;
    }

    if (_dest < _src) {
        while (n) {
            *_dest++ = *_src++;
            n--;
        }
    } else {
        while (n) {
            n--;
            _dest[n] = _src[n];
        }
    }

    return dest;
}

unsigned long int strtoul(const char *str, char **endptr, int base) {
    unsigned long int num = 0;
    while (*str) {
        if (*str < '0' || *str > '9') {
            break;
        }
        uint8_t current_num = *str - '0';
        num = (num * base) + current_num;
        str++;
    }
    if (endptr != 0) {
        *endptr = (char *)str;
    }
    return num;
}

int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *((const unsigned char *)str1) - *((const unsigned char *)str2);
}

int strncmp(const char *str1, const char *str2, size_t num) {
    size_t i;
    for (i = 0; (i < num) && *str1; i++) {
        if (*str1 != *str2) {
            return *((const unsigned char *)str1) - *((const unsigned char *)str2);
            break;
        }
        str1++;
        str2++;
    }
    if (i == num) {
        return 0;
    }
    return *((const unsigned char *)str1) - *((const unsigned char *)str2);
}

size_t strlen(const char *str) {
    const char *str2 = str;
    while (*str2) {
        str2++;
    }
    return (str2 - str);
}

char *strcpy(char *destination, const char *source) {
    char *saved_dest = destination;
    while (*source) {
        *destination++ = *source++;
    }
    *destination = *source;
    return saved_dest;
}

char *strstr(char *str1, char *str2) {
    size_t str2len = strlen(str2);
    while (*str1) {
        if (!memcmp(str1++, str2, str2len)) {
            return str1 - 1;
        }
    }
    return 0;
}

char *strchr(char *str, int character) {
    while (*str) {
        if (*str == character) {
            return str;
        }
        str++;
    }
    if (*str == character) {
        return str;
    }
    return 0;
}

size_t strcspn(const char *s1, const char *s2) {
    size_t n = 0;
    if (*s2 == 0) {
        return 0;
    }
    while (*s1) {
        if (strchr((char *)s2, *s1)) {
            return n;
        }
        s1++;
        n++;
    }
    return n;
}

char *itoa(size_t value, char *str, size_t base) {
    char *ptr = str;

    do {
        size_t mod = value % base;
        unsigned char start = '0';
        if ((base == 16) && (mod > 9)) {
            start = 'a';
            mod -= 10;
        }
        *ptr++ = start + mod;
    } while ((value /= base) > 0);
    *ptr = '\0';

    size_t len = strlen(str);

    for (int i = 0; i < len / 2; i++) {
        char c = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = c;
    }

    return str;
}

char *itoa_signed(ssize_t value, char *str, size_t base) {
    bool sign = false;
    if (value < 0) {
        sign = true;
        value = -value;
    }

    char *ptr = str;

    do {
        size_t mod = value % base;
        unsigned char start = '0';
        if ((base == 16) && (mod > 9)) {
            start = 'a';
            mod -= 10;
        }
        *ptr++ = start + mod;
    } while ((value /= base) > 0);
    if (sign) {
        *ptr++ = '-';
    }
    *ptr = '\0';

    size_t len = strlen(str);

    for (int i = 0; i < len / 2; i++) {
        char c = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = c;
    }

    return str;
}
}
