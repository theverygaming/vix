#pragma once

#if defined(__UINT8_TYPE__) && defined(__UINT16_TYPE__) && defined(__UINT32_TYPE__) && defined(__UINT64_TYPE__)
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;
#else
#error "compiler does not provide __UINTxx_TYPE__"
#endif

#if defined(__INT8_TYPE__) && defined(__INT16_TYPE__) && defined(__INT32_TYPE__) && defined(__INT64_TYPE__)
typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;
#else
#error "compiler does not provide __INTxx_TYPE__"
#endif

#if defined(__UINTPTR_TYPE__) && defined(__INTPTR_TYPE__) && defined(__UINTPTR_MAX__) && defined(__INTPTR_MAX__)
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __INTPTR_TYPE__ intptr_t;
#undef UINTPTR_MAX
#undef INTPTR_MAX
#define UINTPTR_MAX __UINTPTR_MAX__
#define INTPTR_MAX  __INTPTR_MAX__
#else
#error "compiler does not provide __UINTPTR_TYPE__ ,__INTPTR_TYPE__, __UINTPTR_MAX__ or __INTPTR_MAX__"
#endif

#if defined(__SIZE_TYPE__) && defined(__SIZE_MAX__)
typedef __SIZE_TYPE__ size_t;
#undef SIZE_MAX
#define SIZE_MAX __SIZE_MAX__
#else
#error "compiler does not provide __SIZE_TYPE__ or __SIZE_MAX__"
#endif

typedef intptr_t ssize_t; // a little cursed

#include <vix/arch/common/types.h>
