#include "cpp.h"
#include <stdio.h>
#include <types.h>

extern "C" void __cxa_pure_virtual() {
    // undefined virtual function
}

extern "C" void __cxa_atexit() {}

extern "C" void __dso_handle() {}

typedef void (*constructor)();
extern "C" constructor START_CONSTRUCTORS;
extern "C" constructor END_CONSTRUCTORS;

void cpp_init() {
    for (constructor *i = &START_CONSTRUCTORS; i < &END_CONSTRUCTORS; i++) {
        (*i)();
    }
}