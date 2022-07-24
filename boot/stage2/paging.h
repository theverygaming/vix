#pragma once
#include "../../kernel/types.h"

namespace paging {
    void initpaging();

    enum page_size { FOUR_KiB, FOUR_MB };
    enum page_priv { SUPERVISOR, USER };
    enum page_perms { R, RW };

    void create_directory_entry(int tablenum, void *address, enum page_size pagesize, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present);

    void create_pagetable_entry(int tablenum, int entrynum, void *address, bool global, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present);
    void delete_pagetable_entry(int tablenum, int entrynum);

    void *get_physaddr(void *virtualaddr);

}