#pragma once
#include <vix/types.h>

namespace net {
    class networkstack;
}

namespace net {
    class arp {
    public:
        void receive(net::networkstack *netstack, void *data, size_t size);
    };
}
