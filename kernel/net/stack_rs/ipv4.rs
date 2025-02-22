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
        write!(f, "IPv4Header {{ source: {:?}, destination: {:?} }}", self.source, self.destination)
    }
}

impl IPv4Header {
    pub fn deserialize(data: &[u8]) -> Option<(Self, usize)> {
        if data.len() <= 20 {
            return None;
        }

        // TODO: checksum!

        let source = IPv4Address(data[12..16].try_into().unwrap());
        let destination = IPv4Address(data[16..20].try_into().unwrap());

        Some((
            Self {
                diff_serv: 0,
                identification: 0,
                flags: 0,
                fragment_offset: 0,
                ttl: 0,
                protocol: 0,
                source: source,
                destination: destination,
            },
            20, // data offset
        ))
    }
}
