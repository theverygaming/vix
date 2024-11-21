#![no_std]

use core::convert::TryFrom;

pub const KP_EMERG: core::ffi::c_int = 0;
pub const KP_ALERT: core::ffi::c_int = 1;
pub const KP_CRIT: core::ffi::c_int = 2;
pub const KP_ERR: core::ffi::c_int = 3;
pub const KP_WARNING: core::ffi::c_int = 4;
pub const KP_NOTICE: core::ffi::c_int = 5;
pub const KP_INFO: core::ffi::c_int = 6;
pub const KP_DEBUG: core::ffi::c_int = 7;
extern "C" {
    pub fn kprintf(loglevel: core::ffi::c_int, fmt: *const core::ffi::c_char, ...);
}

#[no_mangle]
pub extern "C" fn rust_test(n: i32) -> i32 {
    unsafe {
        kprintf(KP_INFO, "Hi from Rust :3 :3 number: %u\n".as_bytes().as_ptr() as *const i8, n);
    }
    let bytes: [u8; 3] = [1, 0, 2];
    u16::from_le_bytes(<[u8; 2]>::try_from(&bytes[0..2]).unwrap());
    return n;
}
