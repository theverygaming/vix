use core::ffi::{c_void};
use alloc::boxed::Box;

pub type Packet = Box<[u8]>;

struct EthernetAddress(pub [u8; 6]);

pub trait EthernetTrx {
    fn transmit(&mut self, packet: Packet) -> crate::Result<()>;
    fn get_mac(&mut self) -> EthernetAddress;
}

// struct for C++ drivers that they statically allocate. Will never change!
#[repr(C)]
struct EthernetCardOps {
    transmit: extern "C" fn(card: *mut EthernetCard, buf: *const u8, size: usize) -> bool,
    get_mac: extern "C" fn(card: *mut EthernetCard, mac: *const u8, size: usize) -> bool,
}

// struct for C++ drivers to interface with - Rust initializes and manages the memory for this struct
#[repr(C)]
struct EthernetCard {
    ops: *const EthernetCardOps,
    ctx: *mut c_void,

    // Rust part of the struct, not included in the C part
}

#[no_mangle]
pub extern "C" fn netstack_ethernet_rx(card: *mut EthernetCard, buf: *const u8, size: usize) {
    kernel::klog!(kernel::klog::KP_INFO, "net: received packet at {:#x} with size {}", buf as usize, size);
}

#[no_mangle]
pub extern "C" fn netstack_ethernet_register_card(ops: *const EthernetCardOps) ->  *mut EthernetCard {
    let mut stack_guard = crate::NET_STACK.lock();
    let mut stack = stack_guard.as_mut().expect("network stack not initialized");

    let card = Box::new(EthernetCard { ops: ops, ctx: core::ptr::null_mut() });
    let card_ptr: *mut EthernetCard = Box::into_raw(card);

    stack.ethernet_cards.push(unsafe { Box::from_raw(card_ptr) });

    kernel::klog!(kernel::klog::KP_INFO, "net: registered new network card");

    card_ptr
}


impl EthernetTrx for EthernetCard {
    fn transmit(&mut self, packet: Packet) -> crate::Result<()> {
        if !unsafe { ((*self.ops).transmit)(self, packet.as_ptr(), packet.len()) } {
            return Err(crate::NetError {});
        }
        Ok(())
    }

    fn get_mac(&mut self) -> EthernetAddress {
        EthernetAddress([0, 0, 0, 0, 0, 0])
    }
}
