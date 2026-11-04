use core::panic::PanicInfo;
use core::fmt::{Result, Write};

use crate::bindings::kernel_panic;

struct SillyBuffer<'a> {
    buf: &'a mut [u8],
    pos: usize,
}

impl Write for SillyBuffer<'_> {
    fn write_str(&mut self, s: &str) -> Result {
        let bytes = s.as_bytes();
        let remaining = self.buf.len().saturating_sub(self.pos + 1); // leave space for null
        let write_len = core::cmp::min(remaining, bytes.len());
        self.buf[self.pos..self.pos + write_len].copy_from_slice(&bytes[..write_len]);
        self.pos += write_len;
        Ok(())
    }
}

impl<'a> SillyBuffer<'a> {
    fn as_cstr(&mut self) -> &[u8] {
        let idx = if self.pos < self.buf.len() { self.pos } else { self.buf.len() - 1 };
        self.buf[idx] = 0;
        &self.buf[..=idx]
    }
}

#[panic_handler]
fn panic(panic_info: &PanicInfo<'_>) -> ! {
    // we need to handle the whole panic on the stack, otherwise we might do an infinite panic loop!
    let mut buffer = [0u8; 256];
    let mut writer = SillyBuffer { buf: &mut buffer, pos: 0 };

    let _ = write!(writer, "rust panic: {}\n", panic_info.message());

    let c_str = writer.as_cstr();

    unsafe {
        kernel_panic(c_str.as_ptr() as *const core::ffi::c_char);
    }

    loop {}
}
