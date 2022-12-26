#include <net/stack.h>

net::networkstack::networkstack(struct drivers::net::generic_card card) {
    networkcard = card;
}

void net::networkstack::init() {
    uint8_t mac[6];
    for (int i = 0; i < 6; i++) {
        mac[i] = networkcard.get_mac_byte(i);
    }
    ethernet = new net::ethernet(mac);
    _init = true;
}

net::networkstack::~networkstack() {
    delete ethernet;
}

void net::networkstack::receive(void *data, size_t size) {
    if (!_init) {
        return;
    }
    ethernet->receive(this, data, size);
}
