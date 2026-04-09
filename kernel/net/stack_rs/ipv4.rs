use core::convert::TryInto;
use core::fmt;


#[derive(Clone, Copy)]
pub struct IPv4Address(pub [u8; 4]);

impl AsRef<[u8]> for IPv4Address {
    fn as_ref(&self) -> &[u8] {
        &self.0
    }
}

impl fmt::Debug for IPv4Address {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}.{}.{}.{}", 
               self.0[0], self.0[1], self.0[2], self.0[3])
    }
}

pub struct IPv4Header {
    pub diff_serv: u8,
    pub identification: u16,
    pub flags: u8,
    pub fragment_offset: u16,
    pub ttl: u8,
    pub protocol: u8,
    pub source: IPv4Address,
    pub destination: IPv4Address,
}

impl fmt::Debug for IPv4Header {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "IPv4Header {{ diff_serv: 0x{:x} ID: 0x{:x} flags: 0x{:x} fragment offset: 0x{:x} TTL: {} protocol: {} source: {:?}, destination: {:?} }}",
            self.diff_serv,
            self.identification,
            self.flags,
            self.fragment_offset,
            self.ttl,
            self.protocol,
            self.source,
            self.destination,
        )
    }
}

impl IPv4Header {
    pub fn deserialize(data: &[u8]) -> Option<(Self, usize)> {
        if data.len() < 20 {
            return None;
        }

        let version = (data[0] >> 4) & 0xF;
        if version != 4 {
            return None;
        }
        let header_len = data[0] & 0xF;
        let header_bytes = header_len as u16 * 4;
        if header_bytes < 20 {
            return None;
        }
        let diff_serv = data[1];
        let total_len = u16::from_be_bytes(data[2..4].try_into().unwrap());
        if total_len < header_bytes {
            return None;
        }
        let identification = u16::from_be_bytes(data[4..6].try_into().unwrap());
        let flags = data[6] & 0x7;
        let fragment_offset = u16::from_be_bytes(data[6..8].try_into().unwrap()) & 0x1FFF;
        let ttl = data[8];
        let protocol = data[9];
        let checksum = u16::from_be_bytes(data[10..12].try_into().unwrap());
        let source = IPv4Address(data[12..16].try_into().unwrap());
        let destination = IPv4Address(data[16..20].try_into().unwrap());
        // TODO: handle options?

        let checksum_computed = Self::compute_checksum(data, header_bytes);
        if checksum != checksum_computed {
            return None;
        }

        Some((
            Self {
                diff_serv: diff_serv,
                identification: identification,
                flags: flags,
                fragment_offset: fragment_offset,
                ttl: ttl,
                protocol: protocol,
                source: source,
                destination: destination,
            },
            header_bytes as usize, // data offset
        ))
    }

    fn compute_checksum(data: &[u8], header_bytes: u16) -> u16 {
        // u32 sum will easily fit 15*2*0xFFFF without overflowing
        let mut sum: u32 = 0;
        for i in (0..header_bytes as usize).step_by(2) {
            // the checksum field itself is always zero during the checksum computation
            let val = if i != 10 {
                u16::from_be_bytes(data[i..i+2].try_into().unwrap())
            } else {
                0
            };
            sum += val as u32;
        }
        sum = ((sum >> 16) & 0xF) + (sum & 0xFFFF);
        sum = ((sum >> 16) & 0xF) + (sum & 0xFFFF);
        let mut checksum: u16 = sum.try_into().unwrap();
        checksum = !checksum;
        return checksum;
    }
}
