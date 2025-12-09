#![no_std]
#![feature(macro_metavar_expr_concat)]
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

extern "C" fn netstack_init() {
    kernel::klog!(kernel::klog::KP_INFO, "Initializing Rust Network Stack");
    let mut stk = NET_STACK.lock();
    *stk = Some(Stack {
        ethernet_cards: alloc::vec!(),
    });
}
kernel::initfn!(netstack, kernel::INITFN_EARLY_DRIVER_INIT!(), 0, netstack_init);
