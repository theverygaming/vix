#include <net/stack.h>

net::networkstack::networkstack(struct drivers::net::generic_card card) {
    networkcard = card;

    ethernet = new net::ethernet::ethernet_stack;
    ipv4 = new net::ip::ipv4_stack;
    arp = new net::arp::arp_stack;
}

net::networkstack::~networkstack() {
    delete ethernet;
    delete ipv4;
    delete arp;
}

void net::networkstack::receive_packet(void *data, size_t size) {
    ethernet->receive_packet(this, data, size);
}
