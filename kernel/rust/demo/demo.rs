#![no_std]
use core::convert::TryFrom;

extern crate kernel;


#[no_mangle]
pub extern "C" fn rust_test(n: i32) -> i32 {
    kernel::klog!(kernel::klog::KP_INFO, "hello world from rust!! :3 {}", "test string");

    let bytes: [u8; 3] = [1, 0, 2];
    u16::from_le_bytes(<[u8; 2]>::try_from(&bytes[0..2]).unwrap());

    return n;
}

extern "C" fn rust_initcall() -> core::ffi::c_int {
    kernel::klog!(kernel::klog::KP_INFO, "Rust initcall demo");
    return 0;
}

kernel::initcall!(kernel::INITCALL_DRIVER_INIT!(), kernel::INITCALL_PRIO_NORMAL!(), rust_initcall, __INITCALL_RUST_INITCALL);

extern "C" fn rust_initcall2() -> core::ffi::c_int {
    kernel::klog!(kernel::klog::KP_INFO, "Rust initcall demo 2");
    return 0;
}

kernel::initcall!(kernel::INITCALL_DRIVER_INIT!(), kernel::INITCALL_PRIO_NORMAL!(), rust_initcall2, __INITCALL_RUST_INITCALL2);
