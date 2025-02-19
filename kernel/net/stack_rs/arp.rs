use core::convert::TryInto;
use core::fmt;

pub struct ARPPacket<ThardwareAddr, TprotocolAddr> {
    pub hw_type: u16,
    pub proto_type: u16,
    pub hw_len: u8,
    pub proto_len: u8,
    pub op: u16,
    pub sender_hw: ThardwareAddr,
    pub sender_proto: TprotocolAddr,
    pub target_hw: ThardwareAddr,
    pub target_proto: TprotocolAddr,
}

impl<ThardwareAddr: core::fmt::Debug, TprotocolAddr: core::fmt::Debug> fmt::Debug for ARPPacket<ThardwareAddr, TprotocolAddr> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "ARPPacket {{ sender_hw: {:?}, sender_proto: {:?}, target_hw: {:?}, target_proto: {:?} }}", self.sender_hw, self.sender_proto, self.target_hw, self.target_proto)
    }
}

#[derive(Debug)]
pub enum ParsedARPPacket {
    EthernetIPv4(ARPPacket<crate::ethernet::EthernetAddress, crate::ipv4::IPv4Address>),
    Unknown,
}


impl ParsedARPPacket {
    pub fn deserialize(data: &[u8]) -> ParsedARPPacket {
        ParsedARPPacket::Unknown
    }
}
