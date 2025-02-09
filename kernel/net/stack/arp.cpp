#include <vix/kprintf.h>
#include <vix/net/stack/arp.h>
#include <vix/net/stack/ethernet.h>
#include <vix/net/stack/stack.h>

struct __attribute__((packed)) arp_packet {
    uint16_t hardware_type; // usually 0x1 for Ethernet
    uint16_t protocol_type; // should be 0x0800 for IP
    uint8_t hardware_addr_len;
    uint8_t protocol_addr_len;
    uint16_t opcode;              // 0x01 for request, 0x02 for reply
    uint8_t src_hardware_addr[6]; // these sizes are assumed... A proper parser would never do this, but for testing it works i guess?
    uint8_t src_protocol_addr[4];
    uint8_t dest_hardware_addr[6];
    uint8_t dest_protocol_addr[4];
};

void net::arp::receive(net::networkstack *netstack, void *data, size_t size) {
    if (size < sizeof(struct arp_packet)) {
        kprintf(KP_INFO, "    -> packet too short for ARP\n");
        return;
    }
    struct arp_packet *packet = (struct arp_packet *)data;
    if (packet->opcode == 0x100) { // big endian...
        kprintf(KP_INFO, "    -> replying to ARP\n");
        struct arp_packet newpacket;
        memcpy(&newpacket, packet, sizeof(struct arp_packet));

        memcpy(newpacket.dest_protocol_addr, newpacket.src_protocol_addr, sizeof(newpacket.dest_protocol_addr));
        memcpy(newpacket.dest_hardware_addr, newpacket.src_hardware_addr, sizeof(newpacket.dest_hardware_addr));

        newpacket.opcode = 0x200; // reply

        memcpy(newpacket.src_protocol_addr, netstack->ethernet->ipv4.ip, 4);

        for (uint8_t i = 0; i < 6; i++) {
            newpacket.src_hardware_addr[i] = netstack->ethernet->mac[i];
        }

        struct net::stack::packets::ethernet::header ethernetpacket;
        ethernetpacket.ethertype = 0x0608;
        for (uint8_t i = 0; i < 6; i++) {
            ethernetpacket.dest_mac[i] = newpacket.dest_hardware_addr[i];
        }

        netstack->ethernet->send(netstack, &ethernetpacket, &newpacket, sizeof(struct arp_packet));
    }
}
