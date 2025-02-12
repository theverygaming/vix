#include <vix/panic.h>
#include <vix/macros.h>

#define DEF_STUB(fn) void fn () { KERNEL_PANIC("rust stub called: " TOSTRING(fn));}

extern "C" {
    // FIXME: this should not be required -- we need to compile our own core and alloc!
    DEF_STUB(rust_eh_personality)
    DEF_STUB(__rust_no_alloc_shim_is_unstable)
    DEF_STUB(_Unwind_Resume)
    DEF_STUB(__rust_alloc_error_handler_should_panic)
    DEF_STUB(__rust_alloc_error_handler)
}
