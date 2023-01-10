#pragma once
#include "../../kernel/arch/x86/include/arch/generic/types.h"

namespace paging {
    void initpaging();

    void *get_physaddr(void *virtualaddr);
}
