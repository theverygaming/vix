#pragma once
#include "../../kernel/include/arch/x86/generic/archtypes.h"

namespace paging {
    void initpaging();

    void *get_physaddr(void *virtualaddr);
}
