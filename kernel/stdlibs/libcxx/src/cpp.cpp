#include <libcxx.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <stdio.h>
#include <stdlib.h>
#include <types.h>

extern "C" void __cxa_pure_virtual() {
    // undefined virtual function
    KERNEL_PANIC("__cxa_pure_virtual");
}

extern "C" void __cxa_atexit(void (*func)(void *), void *arg, void *dso_handle) {
    // registers destructors
}

extern "C" void __dso_handle() {
    KERNEL_PANIC("__dso_handle(");
}

typedef void (*constructor)();
extern "C" constructor START_CONSTRUCTORS_INITARR;
extern "C" constructor END_CONSTRUCTORS_INITARR;
extern "C" constructor START_CONSTRUCTORS_CTORS;
extern "C" constructor END_CONSTRUCTORS_CTORS;

void cpp_init() {
    if (&START_CONSTRUCTORS_INITARR == &END_CONSTRUCTORS_INITARR) {
        for (constructor *i = &START_CONSTRUCTORS_CTORS; i < &END_CONSTRUCTORS_CTORS; i++) {
            (*i)();
        }
    } else {
        for (constructor *i = &START_CONSTRUCTORS_INITARR; i < &END_CONSTRUCTORS_INITARR; i++) {
            (*i)();
        }
    }
}

void *operator new(size_t size) {
    return mm::kmalloc(size);
    // return mm::kmalloc_aligned(size, size);
}

void *operator new[](size_t size) {
    return mm::kmalloc(size);
    // return mm::kmalloc_aligned(size, size);
}

void operator delete(void *p) {
    mm::kfree(p);
}

void operator delete(void *p, size_t) {
    mm::kfree(p);
}

void operator delete[](void *p) {
    mm::kfree(p);
}

void operator delete[](void *p, size_t) {
    mm::kfree(p);
}
