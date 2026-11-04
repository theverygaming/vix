#![no_std]
#![feature(macro_metavar_expr_concat)]
use core::convert::TryFrom;

extern crate kernel;


#[unsafe(no_mangle)]
pub extern "C" fn rust_test(n: i32) -> i32 {
    kernel::klog!(kernel::klog::KP_INFO, "hello world from rust!! :3 {}", "test string");

    let bytes: [u8; 3] = [1, 0, 2];
    u16::from_le_bytes(<[u8; 2]>::try_from(&bytes[0..2]).unwrap());

    return n;
}

extern "C" fn rust_initfn() {
    kernel::klog!(kernel::klog::KP_INFO, "Rust initfn demo");
}

kernel::initfn!(rust_demo_1, kernel::INITFN_DRIVER_INIT!(), 0, rust_initfn, rust_demo_2);

extern "C" fn rust_initfn2() {
    kernel::klog!(kernel::klog::KP_INFO, "Rust initfn demo 2");
}

kernel::initfn!(rust_demo_2, kernel::INITFN_DRIVER_INIT!(), 0, rust_initfn2);
