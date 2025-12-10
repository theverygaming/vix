#pragma once
#include <vix/macros.h>

void initfn_call_level(unsigned int level);
void initfn_call(struct init_function *);

// called almost immediately after control is handed to the kernel, shortly prior to initializing the allocators
#define INITFN_PRE_MM_INIT 0
// called shortly after memory allocators have been initialized and C++ constructors called but before the scheduler has been initialized
#define INITFN_EARLY_DRIVER_INIT 1
// called from inside the very first scheduler thread (thread 0)
#define INITFN_DRIVER_INIT 2

#define INIT_FUNCTION_FLAG_EXECUTED 1

#define INIT_GET_FN_SYMBOL(name)        init_fn_##name
#define INIT_GET_FN_STRUCT_SYMBOL(name) init_fn_struct_##name
#define INIT_GET_DEPS_SYMBOL(name)      init_deps_##name

struct init_function {
    const char *name;
    unsigned int level;
    unsigned int flags;
    void (*function)();
    struct init_function ***deps;
    size_t n_deps;
};

#define INIT_DEFINE_SYM_EXTERN(name) extern "C" struct init_function *INIT_GET_FN_SYMBOL(name);
#define INIT_GET_DEP_ARR_EL(name) &INIT_GET_FN_SYMBOL(name),

#define INITFN_DEFINE(name_, level_, flags_, function_, ...)                                  \
    MACRO_FOREACH_ARG(INIT_DEFINE_SYM_EXTERN, __VA_ARGS__)                                    \
    static struct init_function **INIT_GET_DEPS_SYMBOL(name_)[] = {                           \
        MACRO_FOREACH_ARG(                                                                    \
            INIT_GET_DEP_ARR_EL, __VA_ARGS__                                                  \
        ) nullptr, /* nullptr to prevent a zero-sized array when there are no dependencies */ \
    };                                                                                        \
    static struct init_function INIT_GET_FN_STRUCT_SYMBOL(name_) = {                          \
        .name = STRINGIFY(name_),                                                             \
        .level = level_,                                                                      \
        .flags = flags_,                                                                      \
        .function = function_,                                                                \
        .deps = INIT_GET_DEPS_SYMBOL(name_),                                                  \
        .n_deps = MACRO_ARG_COUNT(__VA_ARGS__),                                               \
    };                                                                                        \
    extern "C" {                                                                              \
    struct init_function __attribute__((section(".initfn_functions"), used)) *                \
        INIT_GET_FN_SYMBOL(name_) = &INIT_GET_FN_STRUCT_SYMBOL(name_);                        \
    }
