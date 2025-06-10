#pragma once
#include <vix/types.h>

namespace paging {
    void init();
    void *get_physaddr(void *virtualaddr);
    void *get_physaddr_unaligned(void *virtualaddr);
    void clearPageTables(void *virtualaddr, uint32_t pagecount, bool massflush = false);
    bool is_readable(const void *virtualaddr);
}
