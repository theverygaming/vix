use core::panic::PanicInfo;

#[panic_handler]
fn panic(panic_info: &PanicInfo<'_>) -> ! {
    crate::klog!(crate::klog::KP_EMERG, "Rust panic: {}", panic_info.message());
    loop {} // FIXME: log
}
