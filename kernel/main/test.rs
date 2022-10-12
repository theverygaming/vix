#![no_std]
#![no_main]

#[no_mangle]
pub extern "C" fn rust_test() -> i32 {
    unsafe {
        kernel::bindings::printf("hello world from rust C++ bindings!\nnumber: %u\n\0".as_bytes().as_ptr() as *const i8, 69420 as u32);
    }
    20
}
