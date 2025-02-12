use core::alloc::{GlobalAlloc, Layout};

pub struct VixAlloc;

use crate::bindings::{
    mm_kmalloc,
};

unsafe impl GlobalAlloc for VixAlloc {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        mm_kmalloc(layout.size()) as *mut u8
    }

    unsafe fn dealloc(&self, _ptr: *mut u8, _layout: Layout) {
        // FIXME: dealloc
    }
}

#[global_allocator]
pub static ALLOCATOR: VixAlloc = VixAlloc;
