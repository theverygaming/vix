#pragma once
#include <type_traits>
#include <vix/endian.h>
#include <vix/types.h>

namespace net::stack::packets {
    namespace ethernet {
        static const uint16_t ETHERTYPE_IPV4 = 0x0800;
        static const uint16_t ETHERTYPE_IPV6 = 0x86DD;
        static const uint16_t ETHERTYPE_ARP = 0x0806;

        // TODO: 802.1Q support
        struct __attribute__((packed)) header {
            uint8_t dest_mac[6];
            uint8_t source_mac[6];
            uint16_t ethertype;

            void swap() {
                ethertype = BE_16(ethertype);
            }
        };

        static_assert(std::is_trivial_v<struct header> == true);
    }
}
