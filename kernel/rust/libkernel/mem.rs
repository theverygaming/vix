use core::alloc::{GlobalAlloc, Layout};

pub struct VixAlloc;

use crate::bindings::{
    mm_kmalloc,
    mm_kfree,
};

unsafe impl GlobalAlloc for VixAlloc {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        mm_kmalloc(layout.size()) as *mut u8
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        mm_kfree(ptr as *mut core::ffi::c_void);
    }
}

#[global_allocator]
pub static ALLOCATOR: VixAlloc = VixAlloc;
