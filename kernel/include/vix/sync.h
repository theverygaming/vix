#pragma once
#include <vix/interrupts.h>
#include <vix/config.h>
#include <vix/mm/kheap.h>

// FIXME: spinlock doesn't have to spin if we don't have SMP
typedef unsigned int spinlock_t;

inline void spinlock_init(spinlock_t *l) {
    *l = 0;
}

inline bool spinlock_try_lock(spinlock_t *l) {
    return __sync_bool_compare_and_swap(l, 0, 1);
}

inline void spinlock_lock(spinlock_t *l) {
    while (!spinlock_try_lock(l)) {
        // FIXME: need a pause instr or something. catching fire is no good.
        asm volatile ("");
    }
}

inline void spinlock_unlock(spinlock_t *l) {
    __atomic_store_n(l, 0, __ATOMIC_RELEASE);
}

inline void spinlock_lock_intdisable(spinlock_t *l) {
    push_interrupt_disable();
    spinlock_lock(l);
}

inline void spinlock_unlock_intdisable(spinlock_t *l) {
    spinlock_unlock(l);
    pop_interrupt_disable();
}

inline spinlock_t *spinlock_alloc() {
    spinlock_t *l = (spinlock_t *)mm::kmalloc(sizeof(spinlock_t));
    spinlock_init(l);
    return l;
}

inline void spinlock_free(spinlock_t *l) {
    mm::kfree(l);
}
