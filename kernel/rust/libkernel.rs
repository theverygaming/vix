#![no_std]
#![no_main]

#![allow(non_camel_case_types)]

/* these should later be defined differently depending on the chosen architecture */
pub type c_char = core::ffi::c_schar;
pub type c_int = i32;
pub type c_uint = u32;
pub type c_void = core::ffi::c_void;

pub mod bindings;

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_panic: &PanicInfo<'_>) -> ! {
    loop {}
}
