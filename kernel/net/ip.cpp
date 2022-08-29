#include "ip.h"
#include <stdio.h>

void print_ip(uint8_t *ip) {
    printf("%u.%u.%u.%u\n", (uint32_t)ip[0], (uint32_t)ip[1], (uint32_t)ip[2], (uint32_t)ip[3]);
}

void net::ip::parse_ipv4_packet(void *data, size_t len) {
    ipv4_packet_t header;
    ipv4_packet_t *balls = (ipv4_packet_t *)data;
    header = *balls;

    for (int i = 0; i < 40; i++) {
        header.options[i] = 0;
    }
    header.data_start_offset = 21;
    for (int i = 0; i < 40; i++) {
        if (balls->options[i] == 0) {
            header.data_start_offset = 21 + i;
            break;
        }
        printf("option: 0x%p\n", (uint32_t)balls->options[i]);
        header.options[i] = balls->options[i];
    }
    
    printf("src: ");
    print_ip(header.source_address);
    printf("dst: ");
    print_ip(header.destination_address);

}