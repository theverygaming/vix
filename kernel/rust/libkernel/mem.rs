use core::alloc::{GlobalAlloc, Layout};

pub struct VixAlloc;

use crate::bindings::{
    mm_kmalloc_aligned,
    mm_kfree,
    kernel_panic,
};

unsafe impl GlobalAlloc for VixAlloc {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        /*
        if (layout.align() != 1) {
            // we cannot use standard panic here because that could result in an infinite loop if panic allocates memory!
            // HACK: we manually add a silly null terminator because rust strings are not null-terminated
            kernel_panic(
                "Rust allocator: unable to provide correct alignment (asked for %u)\n\0".as_bytes().as_ptr() as *const core::ffi::c_char,
                layout.align() as core::ffi::c_uint
            );
        }
        mm_kmalloc(layout.size()) as *mut u8
        */
        mm_kmalloc_aligned(layout.size(), layout.align()) as *mut u8
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        // FIXME: aligned free
        //mm_kfree(ptr as *mut core::ffi::c_void);
    }
}

#[global_allocator]
pub static ALLOCATOR: VixAlloc = VixAlloc;
