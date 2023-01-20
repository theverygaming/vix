#pragma once
#include <config.h>

#ifdef CONFIG_ENABLE_KERNEL_64

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef long unsigned int uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int int64_t;

typedef uint64_t size_t;

typedef int64_t ssize_t;

typedef int pid_t;

typedef uint64_t uintptr_t;

#define SIZE_MAX    0xFFFFFFFFFFFFFFFF
#define UINTPTR_MAX 0xFFFFFFFFFFFFFFFF

#else

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef uint32_t size_t;

typedef int32_t ssize_t;

typedef int pid_t;

typedef uint32_t uintptr_t;

#define SIZE_MAX    0xFFFFFFFF
#define UINTPTR_MAX 0xFFFFFFFF

#endif
