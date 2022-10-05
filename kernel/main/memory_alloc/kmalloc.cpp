#include <arch/arch.h>
#include <config.h>
#include <debug.h>
#include <memory_alloc/memalloc.h>
#include <panic.h>
#include <stdio.h>
#include <stdlib.h>
#include <types.h>
#include INCLUDE_ARCH_GENERIC(memory.h)

/*
 * Freelist allocator
 */

typedef struct meminfo_t meminfo_t;
struct __attribute__((packed)) meminfo_t {
    meminfo_t *prev;
    meminfo_t *next;
    size_t size; // holds the size of the area EXCLUDING this struct
};

static uint32_t heapPages = 0;

static void *heapPtr;
static meminfo_t *heapListStart = nullptr;

static void init() {
    heapPtr = memalloc::page::kernel_malloc(1);
    if (heapPtr == nullptr) {
        KERNEL_PANIC("kmalloc initial allocation failed");
    }
    heapPages = 1;

    heapListStart = (meminfo_t *)heapPtr;
    *heapListStart = {.prev = nullptr, .next = nullptr, .size = ARCH_PAGE_SIZE - sizeof(meminfo_t)};
}

static meminfo_t *findFreeArea(size_t size) {
    meminfo_t *ptr = heapListStart;
    while (true) {
        if (size <= ptr->size) {
            return ptr;
        }
        if (ptr->next == nullptr) {
            break;
        }
        ptr = ptr->next;
    }
    return nullptr;
}

static meminfo_t *getLastListMember() {
    meminfo_t *ptr = heapListStart;
    while (true) {
        if (ptr->next == nullptr) {
            return ptr;
        }
        ptr = ptr->next;
    }
}

/* returns nullptr if first list member */
static meminfo_t *findListMember(void *adr) {
    meminfo_t *ptr = heapListStart;

    if (adr < ptr) {
        return nullptr;
    }

    while (true) {
        if (ptr->next == nullptr) {
            return ptr;
        }
        if ((ptr->next > adr) && (ptr < adr)) {
            return ptr;
        }
        ptr = ptr->next;
    }
}

/* defragment freelist, returns amount of times list entries were joined */
static size_t joinFreeAreas() {
    size_t totalCounter = 0;

    size_t counter = 0;
    meminfo_t *ptr = heapListStart;
    while (true) {
        if ((ptr == nullptr) || (ptr->next == nullptr)) {
            if (counter == 0) {
                break;
            }
            totalCounter += counter;
            counter = 0;
            ptr = heapListStart;
            continue;
        }
        if (((uint8_t *)ptr->next) == (((uint8_t *)ptr) + ptr->size + sizeof(meminfo_t))) { // the next pointer is directly after the free area, we can join them together
            counter++;
            ptr->size += ptr->next->size + sizeof(meminfo_t);
            ptr->next = ptr->next->next;
            if (ptr->next != nullptr) {
                ptr->next->prev = ptr;
            }
        }
        ptr = ptr->next;
    }
    return totalCounter;
}

static void expandHeap(size_t extra_size) {
    size_t pages = (extra_size + sizeof(meminfo_t)) / ARCH_PAGE_SIZE;
    size_t leftover = extra_size % ARCH_PAGE_SIZE;
    if (leftover > 0) {
        pages++;
    }

    heapPages += pages;
    heapPtr = memalloc::page::kernel_resize(heapPtr, heapPages);

    meminfo_t *last = getLastListMember(); // the last list member should be at the end of our heap. Otherwise something went wrong badly

    last->size += pages * ARCH_PAGE_SIZE;
}

void *memalloc::single::kmalloc(size_t size) {
    if (heapPages == 0) {
        init();
    }
    if (size == 0) {
        return nullptr;
    }

    joinFreeAreas();

    size_t neededSize = size + sizeof(meminfo_t);

    meminfo_t *freeArea = findFreeArea(neededSize);

    if (freeArea != nullptr) {
        // insert a new element into the linked list
        meminfo_t *oldPrev = freeArea->prev;
        meminfo_t *oldNext = freeArea->next;

        size_t leftoverSize = freeArea->size - neededSize;

        meminfo_t *newptr = (meminfo_t *)(((uint8_t *)freeArea) + neededSize);

        newptr->size = leftoverSize;
        newptr->next = oldNext;
        newptr->prev = oldPrev;

        if (oldNext != nullptr) {
            oldNext->prev = newptr;
        }

        if (oldPrev != nullptr) {
            oldPrev->next = newptr;
        } else {
            // this is the start of the heap!
            heapListStart = newptr;
        }

        freeArea->next = nullptr;
        freeArea->prev = nullptr;
        freeArea->size = size;

        char *ptrChar = (char *)freeArea;
        ptrChar = ptrChar + sizeof(meminfo_t);

        *((uint64_t *)ptrChar) = 0; // prevent false double free detection

        return ptrChar;
    }

    // we couldn't find a free area so we have to allocate more memory
    expandHeap(size);
    return kmalloc(size);
}

void memalloc::single::kfree(void *ptr) {
    if (ptr == nullptr) {
        return;
    }

    uint64_t *dfdetect = (uint64_t *)ptr;
    if (*dfdetect == 0xF1CFD26422FDDA53) {
        KERNEL_PANIC("double free!");
    }
    *dfdetect = 0xF1CFD26422FDDA53;

    meminfo_t *infoPtr = (meminfo_t *)(((uint8_t *)ptr) - sizeof(meminfo_t));

    meminfo_t *llptr = findListMember(ptr);
    if (llptr == nullptr) { // we are at the start of the heap
        infoPtr->prev = nullptr;
        infoPtr->next = heapListStart;
        heapListStart->prev = infoPtr;
        heapListStart = infoPtr;
        return;
    }

    if (llptr->next != nullptr) {
        llptr->next->prev = infoPtr;
    }
    infoPtr->next = llptr->next;
    infoPtr->prev = llptr;
    llptr->next = infoPtr;

    joinFreeAreas();
}

void *memalloc::single::krealloc(void *ptr, size_t size) {
    joinFreeAreas();
    meminfo_t *infoPtr = (meminfo_t *)(((uint8_t *)ptr) - sizeof(meminfo_t));
    meminfo_t *llptr = findListMember(ptr);

    if (size < infoPtr->size) {
        meminfo_t *newPtr = (meminfo_t *)(((uint8_t *)ptr) + size);
        // is adding a new element to the linked list worth it?
        size_t leftoverSize = infoPtr->size - size;
        if (leftoverSize > sizeof(meminfo_t)) {
            newPtr->size = leftoverSize - sizeof(meminfo_t);
            if (llptr == nullptr) {
                newPtr->prev = nullptr;
                newPtr->next = heapListStart;
                heapListStart->prev = newPtr;
                heapListStart = newPtr;
                return ptr;
            }
            newPtr->prev = llptr;
            newPtr->next = llptr->next;
            llptr->next = newPtr;
            if (newPtr->next != nullptr) {
                newPtr->next->prev = newPtr;
            }
        }
        return ptr;
    }

    if ((llptr != nullptr) && (llptr->next != nullptr)) {
        if (((uint8_t *)llptr->next) == (((uint8_t *)infoPtr) + infoPtr->size + sizeof(meminfo_t))) {
            if ((infoPtr->size + llptr->next->size) >= size + sizeof(meminfo_t)) {
                size_t leftoverSize = (infoPtr->size + llptr->next->size) - (size + sizeof(meminfo_t));

                meminfo_t *newPtr = (meminfo_t *)(((uint8_t *)ptr) + size);

                newPtr->size = leftoverSize;
                newPtr->next = llptr->next->next;
                if (newPtr->next != nullptr) {
                    newPtr->next->prev = newPtr;
                }
                newPtr->prev = llptr;
                llptr->next = newPtr;

                infoPtr->size = size;

                return ptr;
            }
        }
    }

    // trying to resize failed so we'll malloc and copy instead
    void *newarea = kmalloc(size);

    stdlib::memcpy(newarea, ptr, infoPtr->size);

    kfree(ptr);
    return newarea;
}
