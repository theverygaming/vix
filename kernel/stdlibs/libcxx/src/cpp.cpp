#include <vix/kprintf.h>
#include <libcxx.h>
#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/stdio.h>
#include <stdlib.h>
#include <vix/types.h>

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
            kprintf(KP_DEBUG, "calling ctor constructor - ptr: 0x%p *ptr: 0x%p\n", i, *i);
            (*i)();
        }
    } else {
        for (constructor *i = &START_CONSTRUCTORS_INITARR; i < &END_CONSTRUCTORS_INITARR; i++) {
            kprintf(KP_DEBUG, "calling initarr constructor ptr: 0x%p *ptr: 0x%p\n", i, *i);
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
