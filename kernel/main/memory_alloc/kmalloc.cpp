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

static void checkList() {
    DEBUG_PRINTF_INSANE("    -> checkList()\n");
    meminfo_t *ptr = heapListStart;
    meminfo_t *lastPtr = heapListStart;
    int counter = 0;
    while (true) {
        if ((ptr->prev == nullptr) && (ptr != heapListStart)) {
            KERNEL_PANIC("freelist corrupted, prev null");
        }
        if (ptr->prev != nullptr) {
            if (ptr->prev != lastPtr) {
                printf("current index: %d\n", counter);
                printf("prev instead: 0x%p what prev should be: 0x%p current pointer: 0x%p\n", ptr->prev, lastPtr, ptr);
                KERNEL_PANIC("freelist corrupted, prev not matching");
            }
            if ((((uint8_t *)ptr->prev) + ptr->prev->size) > (uint8_t *)ptr) {
                printf("current index: %d\n", counter);
                printf("ptr->prev 0x%p ptr->prev->size: %u ptr: 0x%p\n", ptr->prev, ptr->prev->size, ptr);
                KERNEL_PANIC("freelist corrupted, overlap");
            }
        }
        if (ptr->next == nullptr) {
            break;
        }
        lastPtr = ptr;
        ptr = ptr->next;
        counter++;
    }
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

/*
 * finds the list member closest below the specified address
 * returns nullptr if before heap start
 */
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

static void expandHeap(size_t extra_size) {
    size_t pages = (extra_size + sizeof(meminfo_t)) / ARCH_PAGE_SIZE;
    size_t leftover = extra_size % ARCH_PAGE_SIZE;
    if (leftover > 0) {
        pages++;
    }
    meminfo_t *newHeapStart = (meminfo_t *)(((uint8_t *)heapPtr) + (heapPages * ARCH_PAGE_SIZE));
    heapPages += pages;
    heapPtr = memalloc::page::kernel_resize(heapPtr, heapPages);

    meminfo_t *last = getLastListMember();

    newHeapStart->next = nullptr;
    newHeapStart->prev = last;
    newHeapStart->size = (pages * ARCH_PAGE_SIZE) - sizeof(meminfo_t);

    newHeapStart->prev->next = newHeapStart;

    last->next = newHeapStart;

    DEBUG_PRINTF_INSANE("heap expanded\n");
}

/* removes member from linked list */
static void removeListMember(meminfo_t *member) {
    if (member->prev != nullptr) {
        member->prev->next = member->next;
    } else {
        if (member->next == nullptr) {
            expandHeap(ARCH_PAGE_SIZE);
            removeListMember(member);
            return;
        }
        heapListStart = member->next;
    }

    if (member->next != nullptr) {
        member->next->prev = member->prev;
    }
}

/* defragment freelist, returns amount of times list entries were joined */
static size_t joinFreeAreas() {
    DEBUG_PRINTF_INSANE("    -> joinFreeAreas()\n");
    checkList();
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
    checkList();
    return totalCounter;
}

void *memalloc::single::kmalloc(size_t size) {
    DEBUG_PRINTF_INSANE("kmalloc(%u)\n", size);
    if (heapPages == 0) {
        init();
    }
    if (size == 0) {
        DEBUG_PRINTF_INSANE("    -> 0x%p\n", nullptr);
        return nullptr;
    }

    joinFreeAreas();

    size_t neededSize = size + sizeof(meminfo_t);

    meminfo_t *freeArea = findFreeArea(neededSize);

    if (freeArea != nullptr) {
        DEBUG_PRINTF_INSANE("    -> found free area\n");
        // insert a new element into the linked list
        checkList();

        size_t leftoverSize = freeArea->size - neededSize;
        if (leftoverSize > sizeof(meminfo_t)) {
            meminfo_t *oldPrev = freeArea->prev;
            meminfo_t *oldNext = freeArea->next;
            DEBUG_PRINTF_INSANE("enough space for new entry\n");
            DEBUG_PRINTF_INSANE("leftover size: %u\n", leftoverSize);
            DEBUG_PRINTF_INSANE("prev = 0x%p\n", freeArea->prev);
            DEBUG_PRINTF_INSANE("next = 0x%p\n", freeArea->next);

            meminfo_t *newptr = (meminfo_t *)(((uint8_t *)freeArea) + neededSize);

            DEBUG_PRINTF_INSANE("newptr: 0x%p\n", newptr);

            newptr->size = leftoverSize - sizeof(meminfo_t);
            newptr->next = oldNext;
            newptr->prev = oldPrev;

            DEBUG_PRINTF_INSANE("prev = 0x%p\n", freeArea->prev);
            DEBUG_PRINTF_INSANE("next = 0x%p\n", freeArea->next);

            checkList();

            if (newptr->next != nullptr) {
                DEBUG_PRINTF_INSANE("newptr->next != nullptr\n");
                newptr->next->prev = newptr;
            }

            if (newptr->prev != nullptr) {
                DEBUG_PRINTF_INSANE("newptr->prev != nullptr\n");
                newptr->prev->next = newptr;
            } else {
                // this is the start of the heap!
                DEBUG_PRINTF_INSANE("heap start\n");
                heapListStart = newptr;
                if (newptr->next != nullptr) {
                    newptr->next->prev = newptr;
                }
            }

            freeArea->size = size;

            checkList();
        } else {
            DEBUG_PRINTF_INSANE("not creating new entry\n");

            removeListMember(freeArea);

            checkList();
        }

        freeArea->next = nullptr;
        freeArea->prev = nullptr;

        checkList();

        char *ptrChar = (char *)freeArea;
        ptrChar = ptrChar + sizeof(meminfo_t);

        size_t asize = freeArea->size;
        if (asize > 8) {
            asize = 8;
        }
        for (int i = 0; i < asize; i++) {
            ptrChar[i] = 0; // prevent false double free detection
        }

        joinFreeAreas();
        DEBUG_PRINTF_INSANE("    -> 0x%p\n", ptrChar);
        checkList();
        return ptrChar;
    }

    // we couldn't find a free area so we have to allocate more memory
    expandHeap(size);
    joinFreeAreas();
    DEBUG_PRINTF_INSANE("    -> resizing heap, calling kmalloc\n");
    checkList();
    return kmalloc(size);
}

void memalloc::single::kfree(void *ptr) {
    DEBUG_PRINTF_INSANE("kfree(0x%p)\n", ptr);
    checkList();
    if (ptr == nullptr) {
        return;
    }

    meminfo_t *infoPtr = (meminfo_t *)(((uint8_t *)ptr) - sizeof(meminfo_t));
    DEBUG_PRINTF_INSANE("infoPtr -> 0x%p\n", infoPtr);

    if (infoPtr->size >= 8) {
        uint64_t *dfdetect = (uint64_t *)ptr;
        if (*dfdetect == 0xF1CFD26422FDDA53) {
            KERNEL_PANIC("double free!");
        }
        *dfdetect = 0xF1CFD26422FDDA53;
    }

    checkList();

    meminfo_t *llptr = findListMember(ptr);
    if (llptr == nullptr) { // we are at the start of the heap
        DEBUG_PRINTF_INSANE("heap start\n");
        infoPtr->prev = nullptr;
        infoPtr->next = heapListStart;
        heapListStart->prev = infoPtr;
        heapListStart = infoPtr;
        checkList();
        return;
    }

    checkList();

    if (llptr->next != nullptr) {
        DEBUG_PRINTF_INSANE("llptr->next 0x%p\n", llptr->next);
        llptr->next->prev = infoPtr;
    }
    infoPtr->next = llptr->next;
    infoPtr->prev = llptr;
    llptr->next = infoPtr;

    checkList();
    joinFreeAreas();
}

void *memalloc::single::krealloc(void *ptr, size_t size) {
    DEBUG_PRINTF_INSANE("krealloc(0x%p, %u)\n", ptr, size);
    checkList();
    joinFreeAreas();
    
    meminfo_t *infoPtr = (meminfo_t *)(((uint8_t *)ptr) - sizeof(meminfo_t));
    /*
    meminfo_t *llptr = findListMember(ptr);

    

    if (size < infoPtr->size) {
        meminfo_t *newPtr = (meminfo_t *)(((uint8_t *)ptr) + size);
        // is adding a new element to the linked list worth it?
        size_t leftoverSize = infoPtr->size - size;
        if (leftoverSize > sizeof(meminfo_t)) {
            DEBUG_PRINTF_INSANE("    -> resize\n", ptr);
            newPtr->size = leftoverSize - sizeof(meminfo_t);
            if (llptr == nullptr) {
                printf("    -> heapstart\n", ptr);
                newPtr->prev = nullptr;
                newPtr->next = heapListStart;
                heapListStart->prev = newPtr;
                heapListStart = newPtr;
                DEBUG_PRINTF_INSANE("    -> 0x%p\n", ptr);
                checkList();
                return ptr;
            }
            newPtr->prev = llptr;
            newPtr->next = llptr->next;
            llptr->next = newPtr;
            if (newPtr->next != nullptr) {
                newPtr->next->prev = newPtr;
            }
        }
        joinFreeAreas();
        DEBUG_PRINTF_INSANE("    -> 0x%p\n", ptr);
        checkList();
        return ptr;
    }

    if ((llptr != nullptr) && (llptr->next != nullptr)) {
        if (((uint8_t *)llptr->next) == (((uint8_t *)infoPtr) + infoPtr->size + sizeof(meminfo_t))) {
            if ((infoPtr->size + llptr->next->size) >= size + sizeof(meminfo_t)) {
                DEBUG_PRINTF_INSANE("    -> enlarge\n", ptr);
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
                DEBUG_PRINTF_INSANE("    -> 0x%p\n", ptr);
                checkList();
                joinFreeAreas();
                return ptr;
            }
        }
    }

    */

    // trying to resize failed so we'll malloc and copy instead
    DEBUG_PRINTF_INSANE("    -> kmalloc\n");
    void *newarea = kmalloc(size);

    memcpy(newarea, ptr, size);
    DEBUG_PRINTF_INSANE("    -> kfree\n");
    kfree(ptr);
    joinFreeAreas();
    DEBUG_PRINTF_INSANE("    -> 0x%p\n", newarea);
    checkList();
    return newarea;
}
