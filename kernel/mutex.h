#include <panic.h>

class mutex {
public:
    void lock() {
        while (__atomic_load_n(&mtx_locked, __ATOMIC_SEQ_CST)) {}
        lockMtx();
    }

    bool try_lock() {
        if (__atomic_load_n(&mtx_locked, __ATOMIC_SEQ_CST)) {
            return false;
        }
        lockMtx();
        return true;
    }

    void unlock() {
        __atomic_store_n(&mtx_locked, false, __ATOMIC_SEQ_CST);
    }

private:
    volatile bool mtx_locked = false;
    void lockMtx() {
        if (__atomic_exchange_n(&mtx_locked, true, __ATOMIC_SEQ_CST)) {
            KERNEL_PANIC("tried locking locked mutex, this is a mutex bug");
        }
    }
};