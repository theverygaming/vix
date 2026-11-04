use core::cell::UnsafeCell;
use core::sync::atomic::{AtomicBool, Ordering};
use core::sync::atomic::fence;
use core::hint::spin_loop;
use core::ops::{Deref, DerefMut};

// i did NOT write this, not good enougha t rust for that for now

pub struct Mutex<T> {
    locked: AtomicBool,
    data: UnsafeCell<T>,
}

unsafe impl<T> Sync for Mutex<T> {}
unsafe impl<T> Send for Mutex<T> {}

impl<T> Mutex<T> {
    pub const fn new(data: T) -> Self {
        Mutex {
            locked: AtomicBool::new(false),
            data: UnsafeCell::new(data),
        }
    }

    pub fn lock(&self) -> MutexGuard<T> {
        while self.locked.compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed).is_err() {
            // TODO: futex... yield..
            spin_loop();
        }

        MutexGuard {
            mutex: self,
        }
    }

    fn unlock(&self) {
        self.locked.store(false, Ordering::Release);
        fence(Ordering::Release);
    }
}

pub struct MutexGuard<'a, T> {
    mutex: &'a Mutex<T>,
}

impl<'a, T> Drop for MutexGuard<'a, T> {
    fn drop(&mut self) {
        self.mutex.unlock();
    }
}

impl<'a, T> Deref for MutexGuard<'a, T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        unsafe { &*self.mutex.data.get() }
    }
}

impl<'a, T> DerefMut for MutexGuard<'a, T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        unsafe { &mut *self.mutex.data.get() }
    }
}
