#include <arch.h>
#include <config.h>
#include <debug.h>
#include <macros.h>
#include <mm/memalloc.h>
#include <panic.h>
#include <stdio.h>
#include <stdlib.h>
#include <types.h>
#include INCLUDE_ARCH_GENERIC(memory.h)

#define DEBUG_PRINTF_INSANE(...) \
    while (0) {} // disable debug printf for this file

/*
 * Freelist allocator
 */

struct __attribute__((packed)) meminfo {
    struct meminfo *prev;
    struct meminfo *next;
    size_t size; // holds the size of the area EXCLUDING this struct
};

static struct meminfo *heap_start = nullptr;
static void *heap_base_ptr = nullptr;
static size_t heap_base_size = 0;

/* linked list related functions */

/*
 * For debugging.
 * Checks linked list and panics if it is not correct.
 */
static void ll_check() {
    DEBUG_PRINTF_INSANE("sizeof(struct meminfo) = 0x%p\n", (uint32_t)sizeof(struct meminfo));
    DEBUG_PRINTF_INSANE("ll_check\n");
    struct meminfo *ptr = heap_start;
    while (ptr != nullptr) {
        DEBUG_PRINTF_INSANE("    -> e: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);
        if (ptr->prev == ptr || ptr->next == ptr) {
            KERNEL_PANIC("ll_check failure!!! ptr->prev == ptr || ptr->next == ptr");
        }
        if (ptr->prev != nullptr) {
            if (ptr->prev->next != ptr) {
                DEBUG_PRINTF_INSANE("is: 0x%p should be: 0x%p\n", ptr->prev->next, ptr);
                KERNEL_PANIC("ll_check failure!!! ptr->prev->next != ptr");
            }
        } else if (ptr != heap_start) {
            KERNEL_PANIC("ll_check failure!!! ptr != heap_start");
        }
        ptr = ptr->next;
    }
}

/*
 * inserts element into linked list
 * if after it will insert after the specified element, default is false
 * otherwise it will insert before it
 *
 * ptr -> element to insert before/after
 * insert -> element to insert
 */
static void ll_insert(struct meminfo *ptr, struct meminfo *insert, bool after = false) {
    DEBUG_PRINTF_INSANE("ll_insert (0x%p, 0x%p)\n", ptr, insert);
    if (after) {
        DEBUG_PRINTF_INSANE("    -> after\n");
    } else {
        DEBUG_PRINTF_INSANE("    -> before\n");
    }
    ll_check();
    struct meminfo *old_prev = ptr->prev;
    struct meminfo *old_next = ptr->next;

    // TODO: handle heap start

    if (after) {
        if (old_next != nullptr) {
            old_next->prev = insert;
        }
        ptr->next = insert;

        insert->prev = ptr;
        insert->next = old_next;
    } else {
        if (old_prev != nullptr) {
            old_prev->next = insert;
        }
        ptr->prev = insert;
        DEBUG_PRINTF_INSANE("    -> s: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);

        insert->next = ptr;
        insert->prev = old_prev;
    }
    DEBUG_PRINTF_INSANE("    -> s: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);

    if (ptr == heap_start && !after) {
        heap_start = insert;
        DEBUG_PRINTF_INSANE("    -> s: 0x%p p: 0x%p n: 0x%p\n", insert, insert->prev, insert->next);
        DEBUG_PRINTF_INSANE("    -> ll_insert heap_start\n");
        // KERNEL_PANIC("debug");
    }
    DEBUG_PRINTF_INSANE("    -> s: 0x%p p: 0x%p n: 0x%p\n", insert, insert->prev, insert->next);
    DEBUG_PRINTF_INSANE("    -> s: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);
    ll_check();
}

/*
 * Adds block to linked list with size
 */
static void ll_alloc_new_block(size_t required) {
    size_t pages = required / ARCH_PAGE_SIZE;
    if (required % ARCH_PAGE_SIZE != 0) {
        pages += 1;
    }
    struct meminfo *new_start = (struct meminfo *)((uint8_t *)heap_base_ptr + (heap_base_size * ARCH_PAGE_SIZE));

    heap_base_size += pages;
    memalloc::page::kernel_resize(heap_base_ptr, heap_base_size);
    new_start->size = pages * ARCH_PAGE_SIZE;

    struct meminfo *ptr = heap_start;
    while (ptr->next != nullptr) {
        ptr = ptr->next;
    }

    ll_insert(ptr, new_start, true);
}

/*
 * removes element from linked list
 */
static void ll_remove(struct meminfo *ptr) {
    DEBUG_PRINTF_INSANE("ll_remove (0x%p)\n", ptr);
    ll_check();

    if (ptr == heap_start && heap_start->next == nullptr) {
        DEBUG_PRINTF_INSANE("adding block");
        ll_alloc_new_block(ARCH_PAGE_SIZE);
    }

    struct meminfo *old_prev = ptr->prev;
    struct meminfo *old_next = ptr->next;

    // TODO: check if this is heap_start and act accordingly
    if (ptr->prev != nullptr) {
        ptr->prev->next = ptr->next;
    }
    if (ptr->next != nullptr) {
        ptr->next->prev = ptr->prev;
    }

    ptr->next = nullptr;
    ptr->prev = nullptr;

    if (ptr == heap_start) {
        if (old_next != nullptr) {
            heap_start = old_next;
        } else {
            KERNEL_PANIC("ll_remove ptr == heap_start -- no new");
        }
    }
    ll_check();
}

/*
 * will replace block already in linked list with a different one
 */
static void ll_replace(struct meminfo *old, struct meminfo *_new) {
    DEBUG_PRINTF_INSANE("ll_replace\n");
    if (old->prev != nullptr) {
        old->prev->next = _new;
    }
    if (old->next != nullptr) {
        old->next->prev = _new;
    }
    _new->next = old->next;
    _new->prev = old->prev;

    old->next = nullptr;
    old->prev = nullptr;

    if (old == heap_start) {
        heap_start = _new;
    }
    ll_check();
}

/*
 * tries to find the block closest to the current one in memory
 */
static struct meminfo *ll_find_closest(struct meminfo *block) {
    uintptr_t closest_dist = UINTPTR_MAX;
    struct meminfo *closest = nullptr;
    struct meminfo *ptr = heap_start;
    while (ptr != nullptr) {
        uintptr_t dist;
        if ((uintptr_t)ptr < (uintptr_t)block) {
            dist = (uintptr_t)block - (uintptr_t)ptr;
        } else {
            dist = (uintptr_t)ptr - (uintptr_t)block;
        }
        if (dist < closest_dist) {
            closest_dist = dist;
            closest = ptr;
        }
        ptr = ptr->next;
    }
    return closest;
}
/*
 * tries to find the smallest possible block or the first big enough block
 */
static struct meminfo *ll_find_block(size_t minsize) {
    struct meminfo *smallest = nullptr;
    size_t smallest_size = SIZE_MAX;

    struct meminfo *ptr = heap_start;
    while (ptr != nullptr) {
        if (ptr->size < minsize) {
            ptr = ptr->next;
            continue;
        }
#ifdef CONFIG_KMALLOC_BEST_SIZE
        if (ptr->size < smallest_size) {
            smallest = ptr;
            smallest_size = ptr->size;
        }
#else
        return ptr;
#endif
        ptr = ptr->next;
    }
    return smallest;
}

/*
 * Will try to defragment, very slow function.
 * Do not use the internal argument
 */
static size_t ll_defrag(bool internal = false) {
    DEBUG_PRINTF_INSANE("ll_defrag\n");
    size_t count = 0;
    struct meminfo *ptr = heap_start;
    while (ptr != nullptr) {
        if (ptr->next != nullptr) {
            // DEBUG_PRINTF_INSANE("c: 0x%p next: 0x%p\n", ((uintptr_t)ptr + sizeof(struct meminfo) + ptr->size), ptr->next);
            if ((uintptr_t)ptr->next == ((uintptr_t)ptr + sizeof(struct meminfo) + ptr->size)) {
                DEBUG_PRINTF_INSANE("can defrag\n");
                DEBUG_PRINTF_INSANE("heap frag: %u\n", mm::getHeapFragmentation());
                DEBUG_PRINTF_INSANE("heap free: %u\n", mm::getFreeSize());
                ptr->size += ptr->next->size + sizeof(struct meminfo);
                ll_remove(ptr->next);
                count += 1;
                DEBUG_PRINTF_INSANE("heap frag: %u\n", mm::getHeapFragmentation());
                break;
            }
        }
        ptr = ptr->next;
    }
    if (!internal) {
        size_t defrag;
        do {
            defrag = ll_defrag(true);
            count += defrag;
        } while (defrag != 0);
    }
    if (!internal) {
        DEBUG_PRINTF_INSANE("ll_defrag -> defragged %u\n", count);
    }
    return count;
}

/*
 * will try to allocate block, possibly inserting an element into the linked list if the size different is too big
 *
 */
static void ll_allocate_block(struct meminfo *block, size_t wanted_size) {
    size_t leftover_size = block->size - wanted_size;
    if (leftover_size > sizeof(struct meminfo)) {
        struct meminfo *_new = (struct meminfo *)((uint8_t *)block + sizeof(struct meminfo) + wanted_size);
        DEBUG_PRINTF_INSANE("    -> creating new block: 0x%p\n", _new);
        _new->size = leftover_size - sizeof(struct meminfo);
        block->size -= leftover_size;
        ll_replace(block, _new);
    } else {
        ll_remove(block);
    }
}

/* memory allocator functions */

static void init() {
    DEBUG_PRINTF_INSANE("kmalloc init\n");
    heap_base_ptr = memalloc::page::kernel_malloc(1);
    heap_base_size = 1;
    heap_start = (struct meminfo *)heap_base_ptr;
    *heap_start = {.prev = nullptr, .next = nullptr, .size = ARCH_PAGE_SIZE - sizeof(struct meminfo)};
}

void *mm::kmalloc(size_t size) {
    DEBUG_PRINTF_INSANE("kmalloc(%u)\n", size);
    if (unlikely(heap_start == nullptr)) {
        init();
    }
    struct meminfo *found = ll_find_block(size);
    if (found != nullptr) {
        DEBUG_PRINTF_INSANE("    -> smallest block found: %u bytes\n", found->size);
        ll_allocate_block(found, size);
        DEBUG_PRINTF_INSANE("    -> resized block to %u bytes\n", found->size);
        return ((uint8_t *)found) + sizeof(struct meminfo);
    }

    ll_alloc_new_block(size);
    return kmalloc(size);
}

void mm::kfree(void *ptr) {
    DEBUG_PRINTF_INSANE("kfree(0x%p)\n", ptr);
    struct meminfo *_blk = (struct meminfo *)((uint8_t *)ptr - sizeof(struct meminfo));
    DEBUG_PRINTF_INSANE("    -> 0x%p\n", _blk);
    struct meminfo *closest = ll_find_closest(_blk);
    if (closest == _blk) {
        KERNEL_PANIC("double free!\n");
        DEBUG_PRINTF_INSANE("--------------------- IGNORED DOUBLE FREE\n");
        return;
    }
    if ((uintptr_t)closest < (uintptr_t)_blk) {
        ll_insert(closest, _blk, true);
    } else {
        ll_insert(closest, _blk, false);
    }
    ll_defrag();
}

void *mm::kmalloc_aligned(size_t size, size_t alignment) {
    // hack level: insane
    void *ptr = kmalloc(size + alignment);
    uintptr_t misalign = (uintptr_t)ptr % alignment;
    if (misalign != 0) {
        ptr = (void *)((uintptr_t)ptr + (alignment - misalign));
    }
    return ptr;
}

void *mm::krealloc(void *ptr, size_t size) {
    DEBUG_PRINTF_INSANE("krealloc(0x%p, %u)\n", ptr, size);

    struct meminfo *_blk = (struct meminfo *)((uint8_t *)ptr - sizeof(struct meminfo));

    size_t copy_size = size;
    if (copy_size > _blk->size) {
        copy_size = _blk->size;
    }

    // trying to resize failed so we'll malloc and copy instead
    DEBUG_PRINTF_INSANE("    -> kmalloc\n");
    void *newarea = kmalloc(size);
    DEBUG_PRINTF_INSANE("    -> newarea: 0x%p\n", newarea);

    memcpy(newarea, ptr, copy_size);
    DEBUG_PRINTF_INSANE("    -> kfree\n");
    kfree(ptr);
    return newarea;
}

size_t mm::getFreeSize() {
    size_t free = 0;
    struct meminfo *ptr = heap_start;
    while (ptr != nullptr) {
        free += ptr->size;
        ptr = ptr->next;
    }
    return free;
}

size_t mm::getHeapFragmentation() {
    size_t count = 0;
    struct meminfo *ptr = heap_start;
    while (ptr != nullptr) {
        count++;
        ptr = ptr->next;
    }
    return count;
}