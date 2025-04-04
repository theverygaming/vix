#[macro_export]
macro_rules! define_initcall_level {
    ($name:ident, $level:expr) => {
        #[macro_export]
        macro_rules! $name {
            () => {
                $level
            };
        }
    };
}

// called almost immediately after control is handed to the kernel, shortly prior to initializing the allocators
define_initcall_level!(INITCALL_PRE_MM_INIT, 0);
// called shortly after memory allocators have been initialized and C++ constructors called but before the scheduler has been initialized
define_initcall_level!(INITCALL_EARLY_DRIVER_INIT, 1);
// called from inside the very first scheduler thread (thread 0)
define_initcall_level!(INITCALL_DRIVER_INIT, 2);

define_initcall_level!(INITCALL_PRIO_DEFAULT, 1024);

#[macro_export]
macro_rules! initcall {
    ($level:expr, $prio:expr, $func:ident, $varname:ident) => {
        #[link_section = concat!(".initcall", $level, ".", $prio)]
        #[used]
        static $varname: extern "C" fn() -> core::ffi::c_int = $func;
    };
}
