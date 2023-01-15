#pragma once
#include <types.h>

namespace paging {
    void *get_physaddr(void *virtualaddr);
    void *get_physaddr_unaligned(void *virtualaddr);
    void clearPageTables(void *virtualaddr, uint32_t pagecount, bool massflush = false);
    void map_page(void *physaddr, void *virtualaddr, size_t count = 1, bool massflush = false, bool global = false);
    bool is_readable(const void *virtualaddr);
    void copyPhysPage(void *dest, void *src);
}
