#[macro_export]
macro_rules! INITCALL_AFTER_MM_INIT {
    () => {
        "0"
    };
}

#[macro_export]
macro_rules! INITCALL_FIRST_THREAD {
    () => {
        "1"
    };
}

#[macro_export]
macro_rules! initcall {
    ($level:expr, $func:ident, $varname:ident) => {
        #[link_section = concat!(".initcall", $level)]
        #[used]
        static $varname: extern "C" fn() -> core::ffi::c_int = $func;
    };
}
