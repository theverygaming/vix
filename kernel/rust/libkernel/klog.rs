use core::ffi::c_int;
use core::ffi::c_char;
use core::fmt::Write;
use alloc::string::String;
use alloc::format;
use alloc::ffi::CString;

use crate::bindings::{
    kprintf,
};

pub use crate::bindings::{
    KP_EMERG,
    KP_ALERT,
    KP_CRIT,
    KP_ERR,
    KP_WARNING,
    KP_NOTICE,
    KP_INFO,
};

pub fn do_klog(level: u32, args: core::fmt::Arguments) {
    use core::ptr;

    let formatted = format!("{}\n", args);

    if let Ok(c_string) = CString::new(formatted) {
        unsafe {
            kprintf(level as i32, c_string.as_ptr());
        }
    }
}

#[macro_export]
macro_rules! klog {
    ($level:expr, $($arg:tt)*) => {
        $crate::klog::do_klog($level, format_args!($($arg)*));
    };
}
