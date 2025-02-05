#include <string.h>
#include <vix/arch/generic/memory.h>
#include <vix/config.h>
#include <vix/mm/kheap.h>
#include <vix/mm/pmm.h>
#include <vix/panic.h>
#include <vix/stdio.h>
#include <vix/test.h>

static int rand_seed = 420;

static int rand() {
    rand_seed = (42069 * rand_seed + 1) % (0xFFFFFF + 1);
    if (rand_seed < 0) { // make sure the result is always positive
        rand_seed = -rand_seed;
    }
    return rand_seed;
}

struct info {
    void *block;
    size_t size;
};

#ifndef CONFIG_ARCH_M68K
#define ALLOC_COUNT    3000
#define ALLOC_MAX_SIZE 6000
#else
#define ALLOC_COUNT    3000
#define ALLOC_MAX_SIZE 1000
#endif

TEST(malloc_test) {
    printf("free size: %u heap frag: %u\n", mm::getFreeSize(), mm::getHeapFragmentation());
    printf("free phys: %u\n", mm::pmm::get_free_blocks() * ARCH_PAGE_SIZE);
    size_t free_size_start = mm::getFreeSize();
    size_t frag_start = mm::getHeapFragmentation();

    size_t alloc_size = 0;

    static struct info bruh[ALLOC_COUNT];
    for (int i = 0; i < ALLOC_COUNT; i++) {
        bruh[i].size = rand() % ALLOC_MAX_SIZE;
        bruh[i].block = mm::kmalloc(bruh[i].size);
        memset(bruh[i].block, i, bruh[i].size);
        alloc_size += bruh[i].size;
    }

    printf("allocated %u\n", alloc_size);
    alloc_size = 0;

    printf("free size: %u heap frag: %u\n", mm::getFreeSize(), mm::getHeapFragmentation());
    printf("free phys: %u\n", mm::pmm::get_free_blocks() * ARCH_PAGE_SIZE);

    bool malloc_corruption = false;

    // check for things bumping into each other
    for (int i = 0; i < ALLOC_COUNT; i++) {
        uint8_t *blk = (uint8_t *)bruh[i].block;
        for (size_t j = 0; j < bruh[i].size; j++) {
            if (blk[j] != (uint8_t)i) {
                printf("expected: %u got: %u @ %u\n", (uint32_t)i, blk[j], (uint32_t)j);
                malloc_corruption = true;
            }
        }
    }
    test::test_section("no malloc corruption", !malloc_corruption);

    printf("free size: %u heap frag: %u\n", mm::getFreeSize(), mm::getHeapFragmentation());
    printf("free phys: %u\n", mm::pmm::get_free_blocks() * ARCH_PAGE_SIZE);

    bool realloc_corruption = false;
    // realloc
    for (int i = 0; i < ALLOC_COUNT; i++) {
        size_t oldsize = bruh[i].size;

        bruh[i].size = rand() % ALLOC_MAX_SIZE;
        bruh[i].block = mm::krealloc(bruh[i].block, bruh[i].size, oldsize);

        if (bruh[i].size < oldsize) {
            oldsize = bruh[i].size;
        }

        uint8_t *blk = (uint8_t *)bruh[i].block;
        for (size_t j = 0; j < oldsize; j++) {
            if (blk[j] != (uint8_t)i) {
                printf("expected: %u got: %u @ %u\n", (uint32_t)i, blk[j], (uint32_t)j);
                realloc_corruption = true;
            }
        }
        memset(bruh[i].block, i, bruh[i].size);
    }
    test::test_section("no realloc corruption", !realloc_corruption);

    printf("free size: %u heap frag: %u\n", mm::getFreeSize(), mm::getHeapFragmentation());
    printf("free phys: %u\n", mm::pmm::get_free_blocks() * ARCH_PAGE_SIZE);

    bool realloc_bump_corruption = false;
    // check for things bumping into each other
    for (int i = 0; i < ALLOC_COUNT; i++) {
        uint8_t *blk = (uint8_t *)bruh[i].block;
        for (size_t j = 0; j < bruh[i].size; j++) {
            if (blk[j] != (uint8_t)i) {
                printf("blk 0x%p -- expected: %u got: %u @ %u\n", bruh[i].block, (uint32_t)i, blk[j], (uint32_t)j);
                realloc_bump_corruption = true;
            }
        }
    }
    test::test_section("no realloc bump corruption", !realloc_bump_corruption);
    // free
    for (int i = 0; i < ALLOC_COUNT; i++) {
        alloc_size += bruh[i].size;
        mm::kfree(bruh[i].block);
    }

    printf("freed %u\n", alloc_size);
    alloc_size = 0;

    printf("free size: %u heap frag: %u\n", mm::getFreeSize(), mm::getHeapFragmentation());
    printf("free phys: %u\n", mm::pmm::get_free_blocks() * ARCH_PAGE_SIZE);

    test::test_section("kmalloc free size", free_size_start <= mm::getFreeSize());

    return !malloc_corruption && !realloc_corruption && !realloc_bump_corruption && (free_size_start <= mm::getFreeSize());
}
