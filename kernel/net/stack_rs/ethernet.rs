use core::ffi::{c_void};
use alloc::boxed::Box;
use core::convert::TryInto;
use core::fmt;
use core::slice;

pub type Packet = Box<[u8]>;

#[derive(Clone, Copy)]
struct EthernetAddress(pub [u8; 6]);

impl fmt::Debug for EthernetAddress {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}", 
               self.0[0], self.0[1], self.0[2], self.0[3], self.0[4], self.0[5])
    }
}

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
    let packet = unsafe { slice::from_raw_parts(buf, size) };
    let (eframe, edata) = EthernetFrame::deserialize(packet).expect("invalid ethernet frame");
    let (ipheader, ipdata) = crate::ipv4::IPv4Header::deserialize(edata).expect("invalid IPv4 packet");
    kernel::klog!(kernel::klog::KP_INFO, "net: received ethernet frame -- {:?} -- data size: {}", eframe, edata.len());
    kernel::klog!(kernel::klog::KP_INFO, "net: received IPv4 packet -- {:?} -- data size: {}", ipheader, ipdata.len());
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


struct EthernetFrame {
    pub destination: EthernetAddress,
    pub source: EthernetAddress,
    pub ethertype: u16,
}

impl fmt::Debug for EthernetFrame {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "EthernetFrame {{ source: {:?}, destination: {:?}, ethertype: {:04x} }}", self.source, self.destination, self.ethertype)
    }
}

impl EthernetFrame {
    pub fn deserialize(data: &[u8]) -> Option<(Self, &[u8])> {
        if data.len() <= 14 {
            return None;
        }

        let destination = EthernetAddress(data[0..6].try_into().unwrap());
        let source = EthernetAddress(data[6..12].try_into().unwrap());
        let ethertype = u16::from_be_bytes(data[12..14].try_into().unwrap());

        Some((
            Self {
                destination: destination,
                source: source,
                ethertype: ethertype,
            },
            &data[14..],
        ))
    }
}
