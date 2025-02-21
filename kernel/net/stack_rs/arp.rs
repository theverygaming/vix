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

impl<ThardwareAddr: AsRef<[u8]>, TprotocolAddr: AsRef<[u8]>> ARPPacket<ThardwareAddr, TprotocolAddr> {
    pub fn serialize(&self, buffer: &mut [u8]) -> Option<usize> {
        let total_len = 8 + (self.hw_len as usize) * 2 + (self.proto_len as usize) * 2;
        if buffer.len() < total_len {
            return None;
        }

        buffer[0..2].copy_from_slice(&self.hw_type.to_be_bytes());
        buffer[2..4].copy_from_slice(&self.proto_type.to_be_bytes());
        buffer[4] = self.hw_len;
        buffer[5] = self.proto_len;
        buffer[6..8].copy_from_slice(&self.operation.to_be_bytes());

        let mut offset = 8;
        let sender_hw_bytes = self.sender_hw.as_ref();
        let sender_proto_bytes = self.sender_proto.as_ref();
        let target_hw_bytes = self.target_hw.as_ref();
        let target_proto_bytes = self.target_proto.as_ref();

        buffer[offset..offset + sender_hw_bytes.len()].copy_from_slice(sender_hw_bytes);
        offset += sender_hw_bytes.len();
        buffer[offset..offset + sender_proto_bytes.len()].copy_from_slice(sender_proto_bytes);
        offset += sender_proto_bytes.len();
        buffer[offset..offset + target_hw_bytes.len()].copy_from_slice(target_hw_bytes);
        offset += target_hw_bytes.len();
        buffer[offset..offset + target_proto_bytes.len()].copy_from_slice(target_proto_bytes);

        Some(total_len)
    }
}
