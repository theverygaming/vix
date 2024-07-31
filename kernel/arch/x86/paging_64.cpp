#include <vix/arch/generic/memory.h>
#include <vix/arch/paging.h>
#include <vix/config.h>
#include <vix/debug.h>
#include <vix/stdio.h>
#include <vix/types.h>

#define AVAILABLE       (1 << 0)
#define WRITE           (1 << 1)
#define SUPERVISOR      (1 << 2)
#define WRITE_THROUGH   (1 << 3)
#define CACHE_DISABLE   (1 << 4)
#define EXECUTE_DISABLE (1 << 63)

#define GLOBAL (1 << 8)

static uint64_t table_l4[512] __attribute__((aligned(16)));
static uint64_t table_l3[512] __attribute__((aligned(16)));
static uint64_t table_l2[512] __attribute__((aligned(16)));
static uint64_t table_l1[512] __attribute__((aligned(16)));

void paging::init() {}

void *paging::get_physaddr(void *virtualaddr) {
    return virtualaddr;
}

void *paging::get_physaddr_unaligned(void *virtualaddr) {
    return virtualaddr;
}

// Both addresses have to be page-aligned!
void paging::map_page(void *physaddr, void *virtualaddr, size_t count, bool massflush, bool global) {
    printf("map_page 0x%p 0x%p\n", physaddr, virtualaddr);
}

void paging::clearPageTables(void *virtualaddr, uint32_t pagecount, bool massflush) {}

bool paging::is_readable(const void *virtualaddr) {
    return true;
}

void paging::copyPhysPage(void *dest, void *src) {}
