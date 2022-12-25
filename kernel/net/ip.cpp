#include <endian.h>
#include <memory_alloc/memalloc.h>
#include <net/icmp.h>
#include <net/ip.h>
#include <stdio.h>
#include <stdlib.h>

static void print_ip(uint8_t *ip) {
    printf("%u.%u.%u.%u", (uint32_t)ip[0], (uint32_t)ip[1], (uint32_t)ip[2], (uint32_t)ip[3]);
}

void net::ip::ipv4_stack::receive_packet(net::networkstack *netstack, struct net::ethernet::ethernet_packet_processed *ethernet_packet, void *data, size_t size) {
    printf("    -> ");
    struct ipv4_packet header;
    struct ipv4_packet *balls = (struct ipv4_packet *)data;
    header = *balls;

    for (int i = 0; i < 40; i++) {
        header.options[i] = 0;
    }
    header.data_start_offset = 21;
    for (int i = 0; i < 40; i++) {
        if (balls->options[i] == 0) {
            header.data_start_offset += +i;
            break;
        }
        printf("option: 0x%p, ", (uint32_t)balls->options[i]);
        header.options[i] = balls->options[i];
    }
    printf("src: ");
    print_ip(header.source_address);
    printf(" dst: ");
    print_ip(header.destination_address);

    header.data_start_offset = 21;

    struct ipv4_packet_processed processed_packet;
    for (int i = 0; i < 4; i++) {
        processed_packet.source_address[i] = header.source_address[i];
        processed_packet.destination_address[i] = header.destination_address[i];
    }
    processed_packet.protocol = header.protocol;

    void *dataptr = ((uint8_t *)data) + header.data_start_offset - 1;
    size_t packetsize = size - (header.data_start_offset - 1);

    if (header.protocol == 0x01) {
        printf(" ICMP\n");
        icmp->receive_packet(netstack, ethernet_packet, &processed_packet, dataptr, packetsize);
        return;
    }
    printf("\n");
}

void net::ip::ipv4_stack::send_packet(net::networkstack *netstack, struct ipv4_packet_processed *packet, void *data, size_t size) {
    struct ipv4_packet packet_n;
    packet_n.version_ihl = 0x45;
    packet_n.TOS = 0x0;
    packet_n.total_length = BE_16((uint16_t)(size + 20));
    packet_n.identification = BE_16((uint16_t)0x81AF);        // only used for fragmentation/reassembly, we don't need it _for now_
    packet_n.flags_fragment_offset = BE_16((uint16_t)0x4000); // no fragmentation
    packet_n.TTL = 0x40;
    packet_n.protocol = packet->protocol;
    packet_n.header_checksum = 0x0000;
    memcpy(packet_n.source_address, ip, 4);
    memcpy(packet_n.destination_address, packet->destination_address, 4);

    // calculate checksum
    uint16_t *packet_u16 = (uint16_t *)&packet_n;
    uint16_t packet_sum = 0;
    for (int i = 0; i < 10; i++) {
        packet_sum += BE_16(packet_u16[i]);
    }
    packet_n.header_checksum = BE_16((uint16_t)(~packet_sum - 3)); // why -3? i have no clue -> maybe carry count???

    void *packetdata = memalloc::single::kmalloc(size + 20);
    memcpy(packetdata, &packet_n, 20);
    memcpy(((uint8_t *)packetdata) + 20, data, size);


    // quick hack
    if(known_ips.size() == 0) {
        known_ips.push_back({{192, 168, 56, 1}, {0xca, 0x85, 0xb4, 0x12, 0xc2, 0x34}});
    }

    int foundindex = -1;
    for(int i = 0; i < known_ips.size(); i++) {
        for(int j = 0; j < 4; j++) {
            if(known_ips[i].ip[j] != packet->destination_address[j]) {
                continue;
            }
        }
        foundindex = i;
        break;
    }
    if(foundindex == -1) {
        printf("could not find MAC address for IP\n");
        return;
    }


    struct net::ethernet::ethernet_packet_processed ethernetpacket;
    ethernetpacket.ethertype = 0x0008; // note: endianness
    for (uint8_t i = 0; i < 6; i++) {
        ethernetpacket.dest_mac[i] = known_ips[foundindex].mac[i];
    }

    netstack->ethernet->send_packet(netstack, &ethernetpacket, packetdata, size + sizeof(ipv4_packet));

    memalloc::single::kfree(packetdata);
}

void net::ip::parse_ipv4_packet(void *data, size_t len) {}

void *net::ip::make_ipv4_packet(const uint8_t *src_address, const uint8_t *dest_address, uint16_t payload_length, uint8_t protocol, size_t *output_len) {
    struct ipv4_packet packet;
    packet.version_ihl = 0x45;
    packet.TOS = 0x0;
    packet.total_length = BE_16((uint16_t)(payload_length + 20));
    packet.identification = BE_16((uint16_t)0x81AF);        // only used for fragmentation/reassembly, we don't need it _for now_
    packet.flags_fragment_offset = BE_16((uint16_t)0x4000); // no fragmentation
    packet.TTL = 0x40;
    packet.protocol = protocol;
    packet.header_checksum = 0x0000;
    memcpy(packet.source_address, src_address, 4);
    memcpy(packet.destination_address, dest_address, 4);

    // calculate checksum
    uint16_t *packet_u16 = (uint16_t *)&packet;
    uint16_t packet_sum = 0;
    for (int i = 0; i < 10; i++) {
        packet_sum += BE_16(packet_u16[i]);
    }
    packet.header_checksum = BE_16((uint16_t)(~packet_sum - 3)); // why -3? i have no clue

    void *ret = new uint8_t[20];
    memcpy(ret, &packet, 20);
    *output_len = 20;
    return ret;
}
