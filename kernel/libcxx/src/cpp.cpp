#include <libcxx.h>
#include <mm/kmalloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <types.h>

extern "C" void __cxa_pure_virtual() {
    // undefined virtual function
    puts(__func__);
    putc('\n');
}

extern "C" void __cxa_atexit(void (*func)(void *), void *arg, void *dso_handle) {
    // registers destructors
}

extern "C" void __dso_handle() {
    puts(__func__);
    putc('\n');
}

typedef void (*constructor)();
extern "C" constructor START_CONSTRUCTORS;
extern "C" constructor END_CONSTRUCTORS;

void cpp_init() {
    for (constructor *i = &START_CONSTRUCTORS; i < &END_CONSTRUCTORS; i++) {
        (*i)();
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
