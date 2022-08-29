#pragma once
#include <types.h>

namespace net::ip {
    typedef struct __attribute__((packed)) {
        uint8_t version_ihl;
        uint8_t TOS;
        uint16_t total_length;
        uint16_t identification;
        uint16_t flags_fragment_offset;
        uint8_t TTL;
        uint8_t Protocol;
        uint16_t header_checksum;
        uint32_t source_address;
        uint32_t destination_address;
        uint8_t options[40];
        uint8_t data_len;
        uint8_t data[];
    } ipv4_header;
}