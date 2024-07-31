#pragma once

namespace mm {
    // tests given range of memory and returns if it seems okay
    bool memtest(void *start, void *end);
}
