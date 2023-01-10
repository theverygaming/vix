#include <mm/memalloc.h>
#include <panic.h>
#include <stdio.h>
#include <stdlib.h>
#include <tests/testprint.h>

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

#define ALLOC_COUNT    200 // max 256
#define ALLOC_MAX_SIZE 50

void malloc_test() {
    printf("free size: %u heap frag: %u\n", mm::getFreeSize(), mm::getHeapFragmentation());
    size_t free_size_start = mm::getFreeSize();
    size_t frag_start = mm::getHeapFragmentation();

    size_t alloc_size = 0;

    struct info bruh[ALLOC_COUNT];
    for (int i = 0; i < ALLOC_COUNT; i++) {
        bruh[i].size = rand() % ALLOC_MAX_SIZE;
        bruh[i].block = mm::kmalloc(bruh[i].size);
        memset(bruh[i].block, i, bruh[i].size);
        alloc_size += bruh[i].size;
    }

    printf("allocated %u\n", alloc_size);
    alloc_size = 0;

    printf("free size: %u heap frag: %u\n", mm::getFreeSize(), mm::getHeapFragmentation());
    TEST("kmalloc", "kmalloc no additional frag", frag_start == mm::getHeapFragmentation());

    bool malloc_corruption = false;

    // check for things bumping into each other
    for (int i = 0; i < ALLOC_COUNT; i++) {
        uint8_t *blk = (uint8_t *)bruh[i].block;
        for (size_t j = 0; j < bruh[i].size; j++) {
            if (blk[j] != i) {
                printf("expected: %u got: %u @ %u\n", (uint32_t)i, blk[j], (uint32_t)j);
                malloc_corruption = true;
            }
        }
    }
    TEST("kmalloc", "no malloc corruption", !malloc_corruption);

    printf("free size: %u heap frag: %u\n", mm::getFreeSize(), mm::getHeapFragmentation());

    bool realloc_corruption = false;
    // realloc
    for (int i = 0; i < ALLOC_COUNT; i++) {
        size_t oldsize = bruh[i].size;

        bruh[i].size = rand() % ALLOC_MAX_SIZE;
        bruh[i].block = mm::krealloc(bruh[i].block, bruh[i].size);

        if (bruh[i].size < oldsize) {
            oldsize = bruh[i].size;
        }

        uint8_t *blk = (uint8_t *)bruh[i].block;
        for (size_t j = 0; j < oldsize; j++) {
            if (blk[j] != i) {
                printf("expected: %u got: %u @ %u\n", (uint32_t)i, blk[j], (uint32_t)j);
                realloc_corruption = true;
            }
        }

        memset(bruh[i].block, i, bruh[i].size);
    }

    TEST("kmalloc", "no realloc corruption", !realloc_corruption);

    printf("free size: %u heap frag: %u\n", mm::getFreeSize(), mm::getHeapFragmentation());

    bool realloc_bump_corruption = false;
    // check for things bumping into each other
    for (int i = 0; i < ALLOC_COUNT; i++) {
        uint8_t *blk = (uint8_t *)bruh[i].block;
        for (size_t j = 0; j < bruh[i].size; j++) {
            if (blk[j] != i) {
                printf("blk 0x%p -- expected: %u got: %u @ %u\n", bruh[i].block, (uint32_t)i, blk[j], (uint32_t)j);
                realloc_bump_corruption = true;
            }
        }
    }
    TEST("kmalloc", "no realloc bump corruption", !realloc_bump_corruption);

    // free
    for (int i = 0; i < ALLOC_COUNT; i++) {
        alloc_size += bruh[i].size;
        mm::kfree(bruh[i].block);
    }

    printf("freed %u\n", alloc_size);
    alloc_size = 0;

    TEST("kmalloc", "kmalloc frag", frag_start == mm::getHeapFragmentation());
    TEST("kmalloc", "kmalloc free size", free_size_start <= mm::getFreeSize());

    printf("free size: %u heap frag: %u\n", mm::getFreeSize(), mm::getHeapFragmentation());
}
