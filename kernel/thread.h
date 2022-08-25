#pragma once
#include <memory_alloc/memalloc.h>
#include <multitasking.h>
#include <stdlib.h>

class thread {
public:
    /* threads are responsible for calling sys_exit themselves. */
    thread(void (*func)(), size_t stack_size_bytes) {
        stack_pointer = (size_t *)memalloc::single::kmalloc(stack_size_bytes + (40 * sizeof(size_t)));
        memset((uint8_t *)pagerange, 0, sizeof(multitasking::process_pagerange) * PROCESS_MAX_PAGE_RANGES);
        multitasking::create_task(stack_pointer, (void *)func, pagerange);
    }

    ~thread() {
        memalloc::single::kfree(stack_pointer);
    }

private:
    size_t *stack_pointer = nullptr;
    multitasking::process_pagerange pagerange[PROCESS_MAX_PAGE_RANGES];
};