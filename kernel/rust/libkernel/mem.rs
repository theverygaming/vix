use core::alloc::{GlobalAlloc, Layout};

pub struct VixAlloc;

use crate::bindings::{
    mm_kmalloc_aligned,
    mm_kfree,
    kernel_panic,
};

unsafe impl GlobalAlloc for VixAlloc {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        if (layout.size() == 0) {
            panic!("zero-size sob");
        }
        let ptr = mm_kmalloc_aligned(layout.size(), layout.align()) as *mut u8;
        if ptr.is_null() {
            panic!("nullptr");
        }
        if (!ptr.is_aligned_to(layout.align())) {
            panic!("bad alignment");
        }
        ptr
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

#[rustc_std_internal_symbol]
fn __rust_alloc_error_handler_should_panic_v2() -> u8 {
    panic!("unreachable");
    // FIXME: do we really need this? Is this ever called?
    1 // always panic!
}

// This symbol is emitted when you do the final linking stage outside of rustc (rustc would get rid of it)
// https://users.rust-lang.org/t/problem-with-external-global-static-linker-region/100081/3
#[rustc_std_internal_symbol]
fn __rust_no_alloc_shim_is_unstable_v2() {}
