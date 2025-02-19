#![no_std]
extern crate alloc;
extern crate kernel;

use kernel::mutex::Mutex;
use alloc::boxed::Box;

mod ethernet;
mod ipv4;
mod arp;

#[derive(Debug, Clone)]
struct NetError;

type Result<T> = core::result::Result<T, NetError>;

struct Stack {
    ethernet_cards: alloc::vec::Vec<Box<dyn ethernet::EthernetTrx>>,
}

impl Stack {

}

static NET_STACK: Mutex<Option<Stack>> = Mutex::new(None);

extern "C" fn netstack_init() -> core::ffi::c_int {
    kernel::klog!(kernel::klog::KP_INFO, "Initializing Rust Network Stack");
    let mut stk = NET_STACK.lock();
    *stk = Some(Stack {
        ethernet_cards: alloc::vec!(),
    });
    return 0;
}
kernel::initcall!(0, netstack_init, __INITCALL_NETSTACK_INIT);
