use core::convert::TryInto;
use core::fmt;

pub struct ARPPacket<ThardwareAddr, TprotocolAddr> {
    pub hw_type: u16,
    pub proto_type: u16,
    pub hw_len: u8,
    pub proto_len: u8,
    pub operation: u16,
    pub sender_hw: ThardwareAddr,
    pub sender_proto: TprotocolAddr,
    pub target_hw: ThardwareAddr,
    pub target_proto: TprotocolAddr,
}

impl<ThardwareAddr: core::fmt::Debug, TprotocolAddr: core::fmt::Debug> fmt::Debug for ARPPacket<ThardwareAddr, TprotocolAddr> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "ARPPacket {{ operation: {:?} sender_hw: {:?}, sender_proto: {:?}, target_hw: {:?}, target_proto: {:?} }}",
            self.operation,
            self.sender_hw,
            self.sender_proto,
            self.target_hw,
            self.target_proto
        )
    }
}

#[derive(Debug)]
pub enum ParsedARPPacket {
    EthernetIPv4(ARPPacket<crate::ethernet::EthernetAddress, crate::ipv4::IPv4Address>),
    Unknown,
}


impl ParsedARPPacket {
    pub fn deserialize(data: &[u8]) -> ParsedARPPacket {
        if data.len() <= 8 {
            // definitely a invalid packet
            return ParsedARPPacket::Unknown;
        }
        let hw_type = u16::from_be_bytes(data[0..2].try_into().unwrap());
        let proto_type = u16::from_be_bytes(data[2..4].try_into().unwrap());
        let hw_len = data[4];
        let proto_len = data[5];
        let operation = u16::from_be_bytes(data[6..8].try_into().unwrap());
        // will the addresses fit?
        if (hw_len as usize) * 2 + (proto_len as usize) * 2 > data.len() - 8 {
            // invalid packet again!
            return ParsedARPPacket::Unknown;
        }
        let mut offset: usize = 8;
        let sender_hw = &data[offset..offset+(hw_len as usize)];
        offset += hw_len as usize;
        let sender_proto = &data[offset..offset+(proto_len as usize)];
        offset += proto_len as usize;
        let target_hw = &data[offset..offset+(hw_len as usize)];
        offset += hw_len as usize;
        let target_proto = &data[offset..offset+(proto_len as usize)];
        
        match (hw_type, proto_type, hw_len, proto_len) {
            (1, 0x0800, 6, 4) => ParsedARPPacket::EthernetIPv4({
                ARPPacket::<crate::ethernet::EthernetAddress, crate::ipv4::IPv4Address> {
                    hw_type: hw_type,
                    proto_type: proto_type,
                    hw_len: hw_len,
                    proto_len: proto_len,
                    operation: operation,
                    sender_hw: crate::ethernet::EthernetAddress(sender_hw.try_into().unwrap()),
                    sender_proto: crate::ipv4::IPv4Address(sender_proto.try_into().unwrap()),
                    target_hw: crate::ethernet::EthernetAddress(target_hw.try_into().unwrap()),
                    target_proto: crate::ipv4::IPv4Address(target_proto.try_into().unwrap())
                }
            }),
            _ => ParsedARPPacket::Unknown,
        }
    }
}
