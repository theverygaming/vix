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

#[alloc_error_handler]
fn alloc_error_handler(layout: Layout) -> ! {
    panic!("alloc_error_handler AAAAAAAAA");
}

// make sure our error handler will actually get called LOL
// #[alloc_error_handler] doesn't seem to work idk
#[rustc_std_internal_symbol]
fn __rust_alloc_error_handler(size: usize, align: usize) -> ! {
    let layout = unsafe {
        Layout::from_size_align_unchecked(size, align)
    };
    alloc_error_handler(layout)
}

#[rustc_std_internal_symbol]
fn __rust_alloc_error_handler_should_panic_v2() -> u8 {
    // FIXME: do we really need this? Is this ever called?
    1 // always panic!
}

// This symbol is emitted when you do the final linking stage outside of rustc (rustc would get rid of it)
// https://users.rust-lang.org/t/problem-with-external-global-static-linker-region/100081/3
#[rustc_std_internal_symbol]
fn __rust_no_alloc_shim_is_unstable_v2() {}
