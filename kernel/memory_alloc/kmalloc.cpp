#include "memalloc.h"
#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <types.h>

#define PAGE_SIZE 4096

static uint32_t heapPages = 0;

static void *heapPtr;

typedef struct meminfo_t meminfo_t;
struct __attribute__((packed)) meminfo_t {
    meminfo_t *next;
    size_t size;
    bool free;
};

void init() {
    heapPtr = memalloc::page::kernel_malloc(1); // ignore if this fails, if it does we will get a pagefault
    heapPages = 1;
    *((meminfo_t *)heapPtr) = {.next = 0, .size = PAGE_SIZE - sizeof(meminfo_t), .free = true};
}

meminfo_t *findFreeArea(size_t size) {
    meminfo_t *ptr = (meminfo_t *)heapPtr;
    while (true) {
        if (ptr->free && size <= ptr->size) {
            return ptr;
        }
        if (ptr->next == nullptr) {
            break;
        }
        ptr = ptr->next;
    }
    return 0;
}

void createNewFreeAreas() {
    meminfo_t *ptr = (meminfo_t *)heapPtr;
    while (true) {
        if (ptr->next == nullptr) {
            // check leftover space
            size_t offset = ((((char *)ptr) + sizeof(meminfo_t)) - ((char *)heapPtr)) + ptr->size;
            size_t leftover = (heapPages * PAGE_SIZE) - offset;
            meminfo_t *newptr = (meminfo_t *)(((char *)heapPtr) + offset);

            if (leftover > 0) {
                *newptr = {.next = 0, .size = leftover - sizeof(meminfo_t), .free = true};
                ptr->next = newptr;
            }
            break;
        }
        ptr = ptr->next;
    }
}

/* joins multiple small fragments after each other back together into one big one */
int joinFreeAreas() {
    int counter = 0;
    meminfo_t *ptr = (meminfo_t *)heapPtr;
    while (true) {
        if (ptr->free) {
            if (ptr->next != nullptr) {
                if (ptr->next->free) {
                    // the next pointer is also free, lets join the two together into one
                    counter++;
                    ptr->size += ptr->next->size + sizeof(meminfo_t);
                    ptr->next = ptr->next->next;
                }
            }
        }
        if (ptr->next == nullptr) {
            break;
        }
        ptr = ptr->next;
    }
    return counter;
}

void *memalloc::single::kmalloc(size_t size) {
    if (heapPages == 0) {
        init();
    }
    createNewFreeAreas();
    while (joinFreeAreas() > 0) {}
    meminfo_t *freeArea = findFreeArea(size);
    if (freeArea != nullptr) {
        freeArea->size = size;
        freeArea->free = false;
        char *ptrChar = (char *)freeArea;
        return ptrChar + sizeof(meminfo_t);
    } else {
        // so we have to allocate more memory
        size_t pages = size / PAGE_SIZE;
        size_t leftover = size % PAGE_SIZE;
        if (pages != 0 && leftover > 0) {
            pages += 1;
        }
        if (pages == 0) {
            pages += 1;
        }
        heapPages += pages;
        heapPtr = memalloc::page::kernel_realloc(heapPtr, heapPages);

        createNewFreeAreas();
        while (joinFreeAreas() > 0) {}
        freeArea = findFreeArea(size);
        if (freeArea != nullptr) {
            freeArea->size = size;
            freeArea->free = false;
            char *ptrChar = (char *)freeArea;
            return ptrChar + sizeof(meminfo_t);
        }
    }
    return nullptr;
}

void memalloc::single::kfree(void *ptr) {
    meminfo_t *info_ptr = (meminfo_t *)(((char *)ptr) - sizeof(meminfo_t));
    if (!info_ptr->free) {
        info_ptr->free = true;
    } else {
        printf("waddafuck double free?\n");
        debug::debug_loop();
    }
}

void *memalloc::single::krealloc(void *ptr, size_t size) {
    createNewFreeAreas();
    while (joinFreeAreas() > 0) {}

    /* try to realloc */
    meminfo_t *info_ptr = (meminfo_t *)(((char *)ptr) - sizeof(meminfo_t));
    size_t oldSize = info_ptr->size;

    if (info_ptr->size == size) {
        return ptr;
    }

    if (info_ptr->size > size) { // size decrease
        info_ptr->size = size;
        return ptr;
    }

    // TODO: actually try to realloc

    /* if realloc fails, use malloc */

    meminfo_t *freeArea = findFreeArea(size);
    if (freeArea != nullptr) {
        freeArea->size = size;
        freeArea->free = false;
        char *ptrChar = (char *)freeArea;

        void *newptr = ptrChar + sizeof(meminfo_t);
        if (newptr != ptr) {
            memcpy((char *)newptr, (char *)ptr, oldSize);
        }
        memalloc::single::kfree(ptr);

        return ptrChar + sizeof(meminfo_t);
    } else {
        // so we have to allocate more memory
        size_t pages = size / PAGE_SIZE;
        size_t leftover = size % PAGE_SIZE;
        if (pages != 0 && leftover > 0) {
            pages += 1;
        }
        if (pages == 0) {
            pages += 1;
        }
        heapPages += pages;
        heapPtr = memalloc::page::kernel_realloc(heapPtr, heapPages);

        createNewFreeAreas();
        while (joinFreeAreas() > 0) {}
        freeArea = findFreeArea(size);
        if (freeArea != nullptr) {
            freeArea->size = size;
            freeArea->free = false;
            char *ptrChar = (char *)freeArea;

            void *newptr = ptrChar + sizeof(meminfo_t);
            if (newptr != ptr) {
                memcpy((char *)newptr, (char *)ptr, oldSize);
            }
            memalloc::single::kfree(ptr);

            return newptr;
        }
    }
    return nullptr;
}