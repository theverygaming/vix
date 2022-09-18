#pragma once
#include <types.h>

inline void *operator new(size_t, void *p) throw() {
    return p;
}
inline void *operator new[](size_t, void *p) throw() {
    return p;
}
inline void operator delete(void *, void *) throw(){};
inline void operator delete[](void *, void *) throw(){};

void cpp_init();
