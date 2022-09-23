#include <cpp.h>
#include <memory_alloc/memalloc.h>
#include <stdlib.h>
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

void *operator new(size_t size) {
    return memalloc::single::kmalloc(size);
}

void *operator new[](size_t size) {
    return memalloc::single::kmalloc(size);
}

void operator delete(void *p) {
    memalloc::single::kfree(p);
}

void operator delete(void *p, size_t) {
    memalloc::single::kfree(p);
}

void operator delete[](void *p) {
    memalloc::single::kfree(p);
}

void operator delete[](void *p, size_t) {
    memalloc::single::kfree(p);
}

/* some compilers internally use memset and memcpy */

extern "C" void *memcpy(void *dst, const void *src, size_t n) {
    stdlib::memcpy(dst, src, n);
}
extern "C" void *memset(void *ptr, int value, size_t n) {
    stdlib::memset(ptr, value, n);
}
