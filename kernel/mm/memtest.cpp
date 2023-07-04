#include <mm/memtest.h>
#include <types.h>

// https://www.memtest86.com/tech_individual-test-descr.html

/*static bool walking_bit(void *_start, void *_end, bool walkingzero) {
    uint8_t *start = (uint8_t *)_start;
    uintptr_t size = (uintptr_t)_end - (uintptr_t)_start;

    for (int i = 0; i < 8; i++) {
        int n = i;
        for (uintptr_t i = 0; i < size; i++) {
            uint8_t value = walkingzero ? ~(0x1 << n) : 0x1 << n;
            *(start + i) = value;
            n++;
            if (n == 8) {
                n = 0;
            }
        }
        n = i;
        for (uintptr_t i = 0; i < size; i++) {
            uint8_t value = walkingzero ? ~(0x1 << n) : 0x1 << n;
            if (*(start + i) != value) {
                return false;
            }
            n++;
            if (n == 8) {
                n = 0;
            }
        }
    }
    return true;
}*/

static bool address(void *_start, void *_end) {
    uintptr_t *start = (uintptr_t *)_start;
    uintptr_t size = (uintptr_t)_end - (uintptr_t)_start;

    for (uintptr_t i = 0; i < size / sizeof(uintptr_t); i++) {
        *(start + i) = (uintptr_t)(start + i);
    }

    for (uintptr_t i = 0; i < size / sizeof(uintptr_t); i++) {
        if (*(start + i) != (uintptr_t)(start + i)) {
            return false;
        }
    }
    return true;
}

bool mm::memtest(void *_start, void *_end) {
    return address(_start, _end);
}
