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
        uint8_t source_address[4];
        uint8_t dest_address[4];

        uint8_t options[40];

        uint32_t data_len;
        uint8_t data[65515]; // is this correct?
    } ipv4_header;

    ipv4_header parse_ipv4_packet(void *data, size_t len);
}