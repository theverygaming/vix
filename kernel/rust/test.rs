#![no_std]
#![no_main]

#[no_mangle]
pub extern "C" fn rust_test(n: i32) -> i32 {
    unsafe {
        kernel::bindings::kprintf::kprintf(kernel::bindings::kprintf::KP_INFO, "Hi from Rust :3 :3 number: %u\n".as_bytes().as_ptr() as *const i8, n);
    }
    return n;
}
