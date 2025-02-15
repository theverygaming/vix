use core::panic::PanicInfo;
use alloc::ffi::CString;
use alloc::format;

use crate::bindings::{
    kernel_panic,
};

#[panic_handler]
fn panic(panic_info: &PanicInfo<'_>) -> ! {
    match (
        CString::new("Rust panic: %s\n"),
        CString::new(format!("{}", panic_info.message())),
    ) {
        (Ok(fmt), Ok(info)) => unsafe {
            kernel_panic(fmt.as_ptr(), info.as_ptr());
        },
        _ => unsafe {
            // HACK: we manually add a silly null terminator because rust strings are not null-terminated
            kernel_panic("Massive issue: rust panic failed!\n\0".as_bytes().as_ptr() as *const core::ffi::c_char);
        },
    }
}
