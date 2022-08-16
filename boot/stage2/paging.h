#pragma once
#include "../../kernel/types.h"

namespace paging {
    void initpaging();

    void *get_physaddr(void *virtualaddr);
}