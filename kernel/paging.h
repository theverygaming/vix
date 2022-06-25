#pragma once
#include "types.h"

namespace paging {
    void initpaging();

    enum page_size {FOUR_KiB, FOUR_MB};
    enum page_priv {SUPERVISOR, USER};
    enum page_perms {R, RW};

    void create_directory_entry(int tablenum, void* address, enum page_size pagesize, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present);

    void create_pagetable_entry(int tablenum, int entrynum, void* address, bool global, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present);
    void delete_pagetable_entry(int tablenum, int entrynum);

    void* get_physaddr(void* virtualaddr);
    void loadApplicationMemory(void* appPhysAddress, int pagecount);
    void clearPageTables(void* virtAddress, uint32_t pagecount);
    void map_page(void *physaddr, void *virtualaddr);

    void set_pagetable_entry(int tablenum, int entrynum, void* address, bool global, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present);
    void set_directory_entry(int tablenum, void* address, enum page_size pagesize, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present);
    bool check_directory_entry_present(int tablenum);
    bool check_pagetable_entry_present(int tablenum, int entrynum);
}