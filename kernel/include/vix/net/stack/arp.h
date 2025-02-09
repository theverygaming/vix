#pragma once
#include <vix/types.h>

namespace net {
    class networkstack;
}

namespace net {
    namespace arp {
        void receive(net::networkstack *netstack, void *data, size_t size);
    };
}
