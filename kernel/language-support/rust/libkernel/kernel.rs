#![no_std]
#![feature(alloc_error_handler)]
#![feature(rustc_attrs)]
#![feature(pointer_is_aligned_to)]
#![feature(macro_metavar_expr_concat)]

extern crate alloc;

mod panic;
pub mod bindings;
pub mod klog;
mod mem;
pub mod mutex;
pub mod initfn;

pub fn test() -> i32 {
    return 123;
}
