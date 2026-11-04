#[macro_export]
macro_rules! define_initfn_level {
    ($name:ident, $level:expr) => {
        #[macro_export]
        macro_rules! $name {
            () => {
                $level
            };
        }
    };
}

#[repr(C)]
pub struct init_function {
    pub name: *const::core::ffi::c_char,
    pub level: ::core::ffi::c_uint,
    pub flags: ::core::ffi::c_uint,
    pub function: ::core::option::Option<unsafe extern "C" fn()>,
    pub deps: *mut *mut *mut init_function,
    pub n_deps: usize,
}
unsafe impl Sync for init_function {}

#[repr(C)]
pub struct init_function_ptr {
    pub ptr: *const init_function,
}
unsafe impl Sync for init_function_ptr {}

// called almost immediately after control is handed to the kernel, shortly prior to initializing the allocators
define_initfn_level!(INITFN_PRE_MM_INIT, 10);
// called shortly after memory allocators have been initialized and C++ constructors called but before some architecture things have been initialized
define_initfn_level!(INITFN_SUPER_EARLY_DRIVER_INIT, 19);
// called shortly after memory allocators have been initialized and C++ constructors called but before the scheduler has been initialized
define_initfn_level!(INITFN_EARLY_DRIVER_INIT, 20);
// called from inside the very first scheduler thread (thread 0)
define_initfn_level!(INITFN_DRIVER_INIT, 30);


#[macro_export]
macro_rules! initfn {
    ($name:ident, $level:expr, $flags:expr, $func:path $(, $dep:ident)*) => {
        $(
            unsafe extern "C" {
                #[link_name = stringify!(${ concat(init_fn_, $dep) })] 
                static ${ concat(_INIT_DEP_, $dep) }: $crate::initfn::init_function_ptr;
            }
        )*

        const ${ concat(init_deps_, $name) }: &[*const *const $crate::initfn::init_function] = &[
            $(
                unsafe { & ${ concat(_INIT_DEP_, $dep) } as *const $crate::initfn::init_function_ptr as *const *const _ },
            )*
            // nullptr to prevent a zero-sized array when there are no dependencies
            core::ptr::null(),
        ];

        static ${ concat(init_fn_struct_, $name) }: $crate::initfn::init_function = $crate::initfn::init_function {
            name: concat!(stringify!($name), "\0").as_ptr() as *const ::core::ffi::c_char,
            level: $level,
            flags: $flags,
            function: Some($func),
            deps: ${ concat(init_deps_, $name) }.as_ptr() as *mut *mut *mut _,
            // excluding nullptr at the end
            n_deps: ${ concat(init_deps_, $name) }.len() - 1, 
        };

        #[used]
        #[unsafe(link_section = ".initfn_functions")]
        #[unsafe(no_mangle)]
        pub static ${ concat(init_fn_, $name) }: $crate::initfn::init_function_ptr = $crate::initfn::init_function_ptr {
            ptr: &${ concat(init_fn_struct_, $name) },
        };
    };
}
