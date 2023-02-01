#include <arch/generic/memory.h>
#include <arch/paging.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>

void paging::init() {}

void *paging::get_physaddr(void *virtualaddr) {
    return virtualaddr;
}

void *paging::get_physaddr_unaligned(void *virtualaddr) {
    return virtualaddr;
}

// Both addresses have to be page-aligned!
void paging::map_page(void *physaddr, void *virtualaddr, size_t count, bool massflush, bool global) {}

void paging::clearPageTables(void *virtualaddr, uint32_t pagecount, bool massflush) {}

bool paging::is_readable(const void *virtualaddr) {
    return true;
}

void paging::copyPhysPage(void *dest, void *src) {}
