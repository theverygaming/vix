#pragma once
#include <vix/types.h>

#define STRINGIFY_(x) #x
#define STRINGIFY(x)  STRINGIFY_(x)

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

#define likely(cond)   __builtin_expect((cond), 1)
#define unlikely(cond) __builtin_expect((cond), 0)

/* alignment must be a power of 2 */
#define ALIGN_UP(value, alignment)            (((value) + (alignment) - 1) & ~((alignment) - 1))
#define ALIGN_DOWN(value, alignment)          ((value) & ~((alignment) - 1))
#define PTR_ALIGN_UP(value, alignment)        ((decltype(value))ALIGN_UP((uintptr_t)(value), (alignment)))
#define PTR_ALIGN_DOWN(value, alignment)      ((decltype(value))ALIGN_DOWN((uintptr_t)(value), (alignment)))
#define IS_ALIGNED(value, alignment)          (((value) & ((decltype(value))(alignment) - 1)) == 0)
#define PTR_IS_ALIGNED(value, alignment)      (IS_ALIGNED((uintptr_t)(value), (alignment)))
#define ALIGN_UP_DIFF(value, alignment)       (ALIGN_UP((value), (alignment)) - (value))
#define ALIGN_DOWN_DIFF(value, alignment)     ((value) - ALIGN_DOWN((value), (alignment)))
#define PTR_ALIGN_UP_DIFF(value, alignment)   (ALIGN_UP_DIFF((uintptr_t)(value), (alignment)))
#define PTR_ALIGN_DOWN_DIFF(value, alignment) (ALIGN_DOWN_DIFF((uintptr_t)(value), (alignment)))

/* alignment can be any value (except zero ofc sob) */
#define ALIGN_ANY_UP(value, alignment)   ((((value) + (alignment) - 1) / (alignment)) * (alignment))
#define ALIGN_ANY_DOWN(value, alignment) ((value) / (alignment) * (alignment))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/preprocessor/macros/__VA_ARGS__/count-arguments
// relies on GNU C/C++ extensions
#define MACRO_ARG_COUNT_(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, n, ...) n
#define MACRO_ARG_COUNT(...) MACRO_ARG_COUNT_(dummy, ##__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define MACRO_FOREACH_0(thing, x)
#define MACRO_FOREACH_1(thing, x) thing(x)
#define MACRO_FOREACH_2(thing, x, ...) thing(x) MACRO_FOREACH_1(thing, __VA_ARGS__)
#define MACRO_FOREACH_3(thing, x, ...) thing(x) MACRO_FOREACH_2(thing, __VA_ARGS__)
#define MACRO_FOREACH_4(thing, x, ...) thing(x) MACRO_FOREACH_3(thing, __VA_ARGS__)
#define MACRO_FOREACH_5(thing, x, ...) thing(x) MACRO_FOREACH_4(thing, __VA_ARGS__)
#define MACRO_FOREACH_6(thing, x, ...) thing(x) MACRO_FOREACH_5(thing, __VA_ARGS__)
#define MACRO_FOREACH_7(thing, x, ...) thing(x) MACRO_FOREACH_6(thing, __VA_ARGS__)
#define MACRO_FOREACH_8(thing, x, ...) thing(x) MACRO_FOREACH_7(thing, __VA_ARGS__)
#define MACRO_FOREACH_9(thing, x, ...) thing(x) MACRO_FOREACH_8(thing, __VA_ARGS__)
#define MACRO_FOREACH_10(thing, x, ...) thing(x) MACRO_FOREACH_9(thing, __VA_ARGS__)
#define MACRO_FOREACH_ARG(thing, ...) CONCAT(MACRO_FOREACH_, MACRO_ARG_COUNT(__VA_ARGS__))(thing, __VA_ARGS__)
