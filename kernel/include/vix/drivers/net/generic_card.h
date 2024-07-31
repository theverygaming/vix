#pragma once
#include <vix/types.h>

namespace drivers::net {
    struct generic_card {
        void (*send_packet)(void *data, size_t size);
        uint8_t (*get_mac_byte)(int n);
    };
}
