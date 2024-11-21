use core::panic::PanicInfo;

#[panic_handler]
fn panic(_panic: &PanicInfo<'_>) -> ! {
    loop {}
}


// TODO: this should not be required
#[no_mangle]
pub extern "C" fn rust_eh_personality() {}
