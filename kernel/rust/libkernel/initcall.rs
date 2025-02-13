#[macro_export]
macro_rules! initcall {
    ($level:expr, $func:ident, $varname:ident) => {
        #[link_section = concat!(".initcall", stringify!($level))]
        #[used]
        static $varname: extern "C" fn() -> core::ffi::c_int = $func;
    };
}
