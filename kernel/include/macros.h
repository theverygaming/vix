#pragma once
#include <types.h>

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)

#define likely(cond)   __builtin_expect((cond), 1)
#define unlikely(cond) __builtin_expect((cond), 0)

/* alignment must be a power of 2 */
#define ALIGN_UP(value, alignment)            (((value) + (alignment)-1) & ~((alignment)-1))
#define ALIGN_DOWN(value, alignment)          ((value) & ~((alignment)-1))
#define PTR_ALIGN_UP(value, alignment)        ((typeof(value))ALIGN_UP((uintptr_t)(value), (alignment)))
#define PTR_ALIGN_DOWN(value, alignment)      ((typeof(value))ALIGN_DOWN((uintptr_t)(value), (alignment)))
#define IS_ALIGNED(value, alignment)          (((value) & ((typeof(value))(alignment)-1)) == 0)
#define PTR_IS_ALIGNED(value, alignment)      (IS_ALIGNED((uintptr_t)(value), (alignment)))
#define ALIGN_UP_DIFF(value, alignment)       (ALIGN_UP((value), (alignment)) - (value))
#define ALIGN_DOWN_DIFF(value, alignment)     ((value)-ALIGN_DOWN((value), (alignment)))
#define PTR_ALIGN_UP_DIFF(value, alignment)   (ALIGN_UP_DIFF((uintptr_t)(value), (alignment)))
#define PTR_ALIGN_DOWN_DIFF(value, alignment) (ALIGN_DOWN_DIFF((uintptr_t)(value), (alignment)))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
