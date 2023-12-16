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
