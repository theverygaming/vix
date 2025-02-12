#![no_std]

extern crate alloc;

mod panic;
pub mod bindings;
pub mod klog;
mod mem;
pub mod mutex;

pub fn test() -> i32 {
    return 123;
}
