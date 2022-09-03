#pragma once
#include <types.h>

namespace paging {
    void *get_physaddr(void *virtualaddr);
    void *get_physaddr_unaligned(void *virtualaddr);
    void clearPageTables(void *virtAddress, uint32_t pagecount);
    void map_page(void *physaddr, void *virtualaddr);
    bool is_readable(const void *virtualaddr);
    void copyPhysPage(void *dest, void *src);
}