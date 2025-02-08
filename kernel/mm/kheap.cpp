#include <algorithm>
#include <string.h>
#include <vix/arch/common/paging.h>
#include <vix/arch/generic/memory.h>
#include <vix/config.h>
#include <vix/debug.h>
#include <vix/macros.h>
#include <vix/mm/kheap.h>
#include <vix/mm/pmm.h>
#include <vix/mm/vmm.h>
#include <vix/panic.h>
#include <vix/status.h>
#include <vix/stdio.h>
#include <vix/types.h>

#define PROTECT_ALLOC_STRUCTS

#define DEBUG_PRINTF_INSANE_KHEAP_INSANE(...) \
    while (0) {} // disable debug printf for this file

//#define DEBUG_PRINTF_INSANE_KHEAP_ALLOCS(...) DEBUG_PRINTF_INSANE(__VA_ARGS__)
#define DEBUG_PRINTF_INSANE_KHEAP_ALLOCS(...) DEBUG_PRINTF_INSANE_KHEAP_INSANE(__VA_ARGS__)

/*
 * Page allocator
 */

static void *alloc_pages(size_t pages) {
#ifdef CONFIG_ARCH_HAS_PAGING
    void *area = mm::vmm::kalloc(pages);
    for (size_t i = 0; i < pages; i++) {
        void *phys;
        ASSIGN_OR_PANIC(phys, mm::pmm::alloc_contiguous(1));
        uintptr_t virt = ((uintptr_t)area) + (i * CONFIG_ARCH_PAGE_SIZE);
        arch::vmm::set_page(virt, (uintptr_t)phys, arch::vmm::FLAGS_PRESENT);
        arch::vmm::flush_tlb_single(virt);
    }
#else
    void *area;
    ASSIGN_OR_PANIC(area, mm::pmm::alloc_contiguous(pages));
#endif
    return area;
}

#ifndef CONFIG_KHEAP_BUMP

#ifdef CONFIG_KFREE_CLEANUP
static void free_pages(void *address, size_t count) {
#ifdef CONFIG_ARCH_HAS_PAGING
    unsigned int flags;
    for (size_t i = 0; i < count; i++) {
        uintptr_t phys = arch::vmm::get_page(((uintptr_t)address) + (i * CONFIG_ARCH_PAGE_SIZE), &flags);
        if (unlikely(!(arch::vmm::set_page(((uintptr_t)address) + (i * CONFIG_ARCH_PAGE_SIZE), 0, 0) & arch::vmm::FLAGS_PRESENT))) {
            KERNEL_PANIC("kmalloc page is somehow unmapped");
        }
        mm::pmm::free_contiguous((void *)phys, 1);
    }
#else
    for (size_t i = 0; i < count; i++) {
        mm::pmm::free_contiguous(((uint8_t *)address) + (i * CONFIG_ARCH_PAGE_SIZE), 1);
    }
#endif
}
#endif

/*
 * Freelist allocator
 */

struct __attribute__((packed)) meminfo {
#ifdef PROTECT_ALLOC_STRUCTS
    uint64_t p1;
#endif
    struct meminfo *prev;
    struct meminfo *next;
    size_t size; // holds the size of the area EXCLUDING this struct
#ifdef PROTECT_ALLOC_STRUCTS
    uint64_t p2;
    uintptr_t checksum;
#endif
};

static struct meminfo *heap_start = nullptr;
static void *heap_base_ptr = nullptr;
static size_t heap_base_size = 0;

/* linked list related functions */

static size_t ll_defrag(bool internal = false);

static uintptr_t ll_get_checksum(struct meminfo *ptr) {
    uintptr_t checksum = 0;
#ifdef PROTECT_ALLOC_STRUCTS
    unsigned char *ptr_c = (unsigned char *)ptr;
    for (unsigned int i = 0; i < sizeof(struct meminfo) - sizeof(uintptr_t); i++) {
        checksum += *ptr_c++;
    }
#endif
    return checksum;
}

/* adds protection features to meminfo struct */
static void ll_protect(struct meminfo *ptr) {
#ifdef PROTECT_ALLOC_STRUCTS
    ptr->p1 = 0x2ff8ce70b09deab5;
    ptr->p2 = 0x7f0dc0fea5239da2;
    ptr->checksum = ll_get_checksum(ptr);
#endif
}

static void ll_unprotect(struct meminfo *ptr) {
#ifdef PROTECT_ALLOC_STRUCTS
    ptr->p1 = 0;
    ptr->p2 = 0;
    ptr->checksum = 0;
#endif
}

static bool ll_check_protect(struct meminfo *ptr) {
#ifdef PROTECT_ALLOC_STRUCTS
    if (ptr->p1 != 0x2ff8ce70b09deab5 || ptr->p2 != 0x7f0dc0fea5239da2 || ll_get_checksum(ptr) != ptr->checksum) {
        return false;
    }
#endif
    return true;
}

/*
 * For debugging.
 * Checks linked list and panics if it is not correct.
 */
static void ll_check() {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("ll_check\n");
    struct meminfo *ptr = heap_start;
    if (heap_start->prev != nullptr) {
        KERNEL_PANIC("ll_check failure!!! heap_start->prev != nullptr");
    }
    while (ptr != nullptr) {
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> e: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);
        if (ptr->prev == ptr || ptr->next == ptr) {
            KERNEL_PANIC("ll_check failure!!! ptr->prev == ptr || ptr->next == ptr");
        }
        if (ptr->prev != nullptr) {
            if (ptr->prev->next != ptr) {
                DEBUG_PRINTF_INSANE_KHEAP_INSANE("is: 0x%p should be: 0x%p\n", ptr->prev->next, ptr);
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
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("ll_insert (0x%p, 0x%p)\n", ptr, insert);
    if ((std::max((uintptr_t)ptr, (uintptr_t)insert) - std::min((uintptr_t)ptr, (uintptr_t)insert)) < sizeof(struct meminfo)) {
        KERNEL_PANIC("ll_insert");
    }
    if (after) {
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> after\n");
    } else {
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> before\n");
    }
    ll_check();
    struct meminfo *old_prev = ptr->prev;
    struct meminfo *old_next = ptr->next;

    // TODO: handle heap start

    if (after) {
        if (old_next != nullptr) {
            old_next->prev = insert;
        }
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> ptr: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);
        ptr->next = insert;
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> ptr: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);

        insert->prev = ptr;
        insert->next = old_next;
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> ptr: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> ins: 0x%p p: 0x%p n: 0x%p\n", insert, insert->prev, insert->next);
    } else {
        if (old_prev != nullptr) {
            old_prev->next = insert;
        }
        ptr->prev = insert;
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> ptr: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);

        insert->next = ptr;
        insert->prev = old_prev;
    }
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> ptr: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);

    if (ptr == heap_start && !after) {
        heap_start = insert;
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> ins: 0x%p p: 0x%p n: 0x%p\n", insert, insert->prev, insert->next);
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> ll_insert heap_start\n");
        // KERNEL_PANIC("debug");
    }
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> ins: 0x%p p: 0x%p n: 0x%p\n", insert, insert->prev, insert->next);
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> ptr: 0x%p p: 0x%p n: 0x%p\n", ptr, ptr->prev, ptr->next);
    ll_check();
}

/*
 * Adds block to linked list with size
 */
static void ll_alloc_new_block(size_t required, bool defrag = true) {
    required += sizeof(struct meminfo);
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("ll_alloc_new_block(current: %u want: %u)\n", heap_base_size * CONFIG_ARCH_PAGE_SIZE, required);
    ll_check();
    size_t pages = ALIGN_UP(required, CONFIG_ARCH_PAGE_SIZE) / CONFIG_ARCH_PAGE_SIZE;

    struct meminfo *new_start = (struct meminfo *)alloc_pages(pages);

    heap_base_size += pages;
    new_start->size = (pages * CONFIG_ARCH_PAGE_SIZE) - sizeof(struct meminfo);

    struct meminfo *ptr = heap_start;
    while (ptr->next != nullptr) {
        ptr = ptr->next;
    }
    ll_check();

    ll_insert(ptr, new_start, true);
    if (defrag) {
        ll_defrag();
    }
}

/*
 * removes element from linked list
 */
static void ll_remove(struct meminfo *ptr) {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("ll_remove (0x%p)\n", ptr);
    ll_check();

    if (ptr == heap_start && heap_start->next == nullptr) {
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("adding block");
        ll_alloc_new_block(CONFIG_ARCH_PAGE_SIZE, false);
    }

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

#ifdef CONFIG_KFREE_CLEANUP
/*
 * tries to deallocate block from linked list
 * returns next block (in case linked list has been changed this is important)
 */
static struct meminfo *ll_try_dealloc_block(struct meminfo *ptr) {
    struct meminfo *next = ptr->next;
    struct meminfo *removeptr = ptr;
    size_t size = ptr->size + sizeof(struct meminfo);
    if (size >= CONFIG_ARCH_PAGE_SIZE) {
        if (!PTR_IS_ALIGNED(removeptr, CONFIG_ARCH_PAGE_SIZE)) {
            size_t diff = PTR_ALIGN_UP_DIFF(removeptr, CONFIG_ARCH_PAGE_SIZE);
            if (diff < sizeof(struct meminfo)) {
                return next;
            }
            if ((diff + sizeof(struct meminfo)) > removeptr->size) {
                return next;
            }

            struct meminfo *new_b = (struct meminfo *)((uint8_t *)removeptr + diff);
            new_b->size = removeptr->size - diff;
            removeptr->size = diff - sizeof(struct meminfo);
            ll_insert(removeptr, new_b, true);
            ll_check();
            removeptr = new_b;
        }
        size = removeptr->size + sizeof(struct meminfo);
        size_t diff = ALIGN_DOWN_DIFF(size, CONFIG_ARCH_PAGE_SIZE);
        if (diff != 0) {
            if (diff < sizeof(struct meminfo)) {
                return next;
            }
            if ((size - diff) < sizeof(struct meminfo)) {
                return next;
            }

            struct meminfo *new_b = (struct meminfo *)((uint8_t *)removeptr + (size - diff));
            new_b->size = diff - sizeof(struct meminfo);
            removeptr->size -= diff - sizeof(struct meminfo);
            ll_insert(removeptr, new_b, true);
            ll_check();
            next = new_b;
        }
        ll_remove(removeptr);
        free_pages(removeptr, (removeptr->size + sizeof(struct meminfo)) / CONFIG_ARCH_PAGE_SIZE);
        ll_check();
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("ll_remove: removed 0x%p\n", removeptr);
    }
    return next;
}

/*
 * tries to free as many blocks as possible
 */
static void ll_cleanup() {
    struct meminfo *ptr = heap_start;
    while (ptr != nullptr) {
        ptr = ll_try_dealloc_block(ptr);
    }
}
#endif

/*
 * will replace block already in linked list with a different one
 */
static void ll_replace(struct meminfo *old, struct meminfo *_new) {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("ll_replace\n");
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
#ifdef CONFIG_KHEAP_BEST_SIZE
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
static size_t ll_defrag(bool internal) {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("ll_defrag\n");
    size_t count = 0;
    struct meminfo *ptr = heap_start;
    while (ptr != nullptr) {
        if (ptr->next != nullptr) {
            // DEBUG_PRINTF_INSANE_KHEAP_INSANE("c: 0x%p next: 0x%p\n", ((uintptr_t)ptr + sizeof(struct meminfo) + ptr->size), ptr->next);
            if ((uintptr_t)ptr->next == ((uintptr_t)ptr + sizeof(struct meminfo) + ptr->size)) {
                DEBUG_PRINTF_INSANE_KHEAP_INSANE("can defrag\n");
                DEBUG_PRINTF_INSANE_KHEAP_INSANE("heap frag: %u\n", mm::getHeapFragmentation());
                DEBUG_PRINTF_INSANE_KHEAP_INSANE("heap free: %u\n", mm::getFreeSize());
                ptr->size += ptr->next->size + sizeof(struct meminfo);
                ll_remove(ptr->next);
                count += 1;
                DEBUG_PRINTF_INSANE_KHEAP_INSANE("heap frag: %u\n", mm::getHeapFragmentation());
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
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("ll_defrag -> defragged %u\n", count);
    }
    return count;
}

/*
 * will try to allocate block, possibly inserting an element into the linked list if the size difference is too big
 *
 */
static void ll_allocate_block(struct meminfo *block, size_t wanted_size) {
    size_t leftover_size = block->size - wanted_size;
    if (leftover_size > (sizeof(struct meminfo) + ARCH_ALIGNMENT_REQUIRED)) {
        struct meminfo *_new = (struct meminfo *)((uint8_t *)block + sizeof(struct meminfo) + wanted_size);
        uintptr_t aligndiff = PTR_ALIGN_UP_DIFF(_new, ARCH_ALIGNMENT_REQUIRED);
        _new = PTR_ALIGN_UP(_new, ARCH_ALIGNMENT_REQUIRED);
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> creating new block: 0x%p\n", _new);
        _new->size = (leftover_size - sizeof(struct meminfo)) - aligndiff;
        block->size -= leftover_size - aligndiff;

        ll_replace(block, _new);
    } else {
        ll_remove(block);
    }
}

/* memory allocator functions */

static void init() {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("kmalloc init\n");
    heap_base_ptr = alloc_pages(1);
    heap_base_size = 1;
    heap_start = (struct meminfo *)heap_base_ptr;

    *heap_start = {
#ifdef PROTECT_ALLOC_STRUCTS
        .p1 = 0,
#endif
        .prev = nullptr,
        .next = nullptr,
        .size = CONFIG_ARCH_PAGE_SIZE - sizeof(struct meminfo),
#ifdef PROTECT_ALLOC_STRUCTS
        .p2 = 0,
        .checksum = 0,
#endif
    };
    ll_protect(heap_start);
    ll_check();
}

void *mm::kmalloc(size_t size) {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("kmalloc(%u)\n", size);
    if (unlikely(heap_start == nullptr)) {
        init();
    }
    struct meminfo *found = ll_find_block(size);
    if (found != nullptr) {
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> smallest block found: %u bytes\n", found->size);
        ll_allocate_block(found, size);
        DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> resized block to %u bytes\n", found->size);
        ll_protect(found);
        return ((uint8_t *)found) + sizeof(struct meminfo);
    }

    ll_alloc_new_block(size);
    return kmalloc(size);
}

void mm::kfree(void *ptr) {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("kfree(0x%p)\n", ptr);
    struct meminfo *_blk = (struct meminfo *)((uint8_t *)ptr - sizeof(struct meminfo));
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> 0x%p\n", _blk);
    struct meminfo *closest = ll_find_closest(_blk);
    if (closest == _blk) {
        KERNEL_PANIC("double free!\n");
    }
    if (!ll_check_protect(_blk)) {
        KERNEL_PANIC("internal kmalloc struct corrupted");
    }
    ll_unprotect(_blk);
    ll_insert(closest, _blk, (uintptr_t)closest < (uintptr_t)_blk);
    ll_defrag();
#ifdef CONFIG_KFREE_CLEANUP
    static int cleanup = 0;
    cleanup++;
    if (cleanup > CONFIG_KFREE_CLEANUP_INTERVAL) {
        ll_cleanup();
        cleanup = 0;
    }
#endif
}

void *mm::kmalloc_aligned(size_t size, size_t alignment) {
    // hack level: insane (it's breaks freeing and realloc ENTIRELY)
    void *ptr = kmalloc(size + alignment);
    ptr = PTR_ALIGN_UP(ptr, alignment);
    return ptr;
}

void *mm::krealloc(void *ptr, size_t size, size_t size_old) {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("krealloc(0x%p, %u)\n", ptr, size);

    struct meminfo *_blk = (struct meminfo *)((uint8_t *)ptr - sizeof(struct meminfo));

    size_t copy_size = size;
    if (copy_size > _blk->size) {
        copy_size = _blk->size;
    }

    if (_blk->size < size_old) {
        KERNEL_PANIC("krealloc: invalid old size");
    }

    // trying to resize failed so we'll malloc and copy instead
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> kmalloc\n");
    void *newarea = kmalloc(size);
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> newarea: 0x%p\n", newarea);

    memcpy(newarea, ptr, copy_size);
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> kfree\n");
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

#else // #ifndef CONFIG_KHEAP_BUMP

static void *last_alloc = nullptr;
static size_t last_size = 0;
static size_t last_size_full = 0;

void *mm::kmalloc(size_t size) {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("kmalloc(%u)\n", size);
    return mm::kmalloc_aligned(size, 2);
}

void mm::kfree(void *ptr) {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("kfree(0x%p)\n", ptr);
    DEBUG_PRINTF_INSANE_KHEAP_ALLOCS("KHEAP: invalidate alloc 0x%p\n", ptr);
}

void *mm::kmalloc_aligned(size_t size, size_t alignment) {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("kmalloc_aligned(%u, %u)\n", size, alignment);
    if (alignment > CONFIG_ARCH_PAGE_SIZE) {
        KERNEL_PANIC("Alignment > CONFIG_ARCH_PAGE_SIZE");
    }

    if (last_alloc == nullptr || (last_size + PTR_ALIGN_UP_DIFF((uint8_t *)last_alloc + last_size, alignment) + size) > last_size_full) {
        last_size_full = ALIGN_UP(size, CONFIG_ARCH_PAGE_SIZE);
        last_alloc = alloc_pages(last_size_full / CONFIG_ARCH_PAGE_SIZE);
        last_size = 0;

        DEBUG_PRINTF_INSANE_KHEAP_INSANE("%u %u %u\n", last_size_full, last_alloc, last_size);
    }

    size_t size_to_add = PTR_ALIGN_UP_DIFF((uint8_t *)last_alloc + last_size, alignment) + size;

    void *ptr = PTR_ALIGN_UP((uint8_t *)last_alloc + last_size, alignment);

    last_size += size_to_add;

    DEBUG_PRINTF_INSANE_KHEAP_ALLOCS("KHEAP: validate alloc 0x%p SIZE %u\n", ptr, size);
    return ptr;
}

void *mm::krealloc(void *ptr, size_t size, size_t size_old) {
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("krealloc(0x%p, %u, %u)\n", ptr, size, size_old);

    size_t copy_size = size_old;
    if (copy_size > size) {
        copy_size = size;
    }

    // trying to resize failed so we'll malloc and copy instead
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> kmalloc\n");
    void *newarea = kmalloc(size);
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> newarea: 0x%p\n", newarea);

    memcpy(newarea, ptr, copy_size);
    DEBUG_PRINTF_INSANE_KHEAP_INSANE("    -> kfree\n");
    kfree(ptr);
    return newarea;
}

size_t mm::getFreeSize() {
    return last_size_full - last_size;
}

size_t mm::getHeapFragmentation() {
    return 0;
}

#endif

void *mm::kmalloc_phys_contiguous(size_t size) {
    size_t pages = ALIGN_UP(size, CONFIG_ARCH_PAGE_SIZE) / CONFIG_ARCH_PAGE_SIZE;
#ifdef CONFIG_ARCH_HAS_PAGING
    void *area = mm::vmm::kalloc(pages);
    void *phys;
    ASSIGN_OR_PANIC(phys, mm::pmm::alloc_contiguous(pages));
    for (size_t i = 0; i < pages; i++) {
        uintptr_t virt = ((uintptr_t)area) + (i * CONFIG_ARCH_PAGE_SIZE);
        arch::vmm::set_page(virt, (uintptr_t)phys + (i * CONFIG_ARCH_PAGE_SIZE), arch::vmm::FLAGS_PRESENT);
        arch::vmm::flush_tlb_single(virt);
    }
#else
    void *area;
    ASSIGN_OR_PANIC(area, mm::pmm::alloc_contiguous(pages));
#endif
    return area;
}
