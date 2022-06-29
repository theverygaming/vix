#include "memalloc.h"
#include "stdio.h"
#include "stdlib.h"

#define BLOCK_SIZE 4096

#define USER_MAX_MEM 4294963200
#define USER_MIN_MEM 0 // lower 100MB of physical memory for kernel

#define BITMAP_BLOCK_COUNT ((USER_MAX_MEM - USER_MIN_MEM) / BLOCK_SIZE)
#define BITMAP_LEN (BITMAP_BLOCK_COUNT) / (8 / 2)

uint8_t memoryBitmap[BITMAP_LEN]; // each block is two bits, the first bit marks if the block is allocated or not, the second bit marks the end of a block

uint8_t bitset(uint8_t* byte, uint8_t bitnum, uint8_t value) {
    *byte ^= (-value ^ *byte) & (1 << bitnum); 
}

uint8_t bitget(uint8_t byte, uint8_t bitnum) {
    return (byte >> bitnum) & 1;
}

void p_get_memmap_entry(uint32_t block, uint8_t* allocated, uint8_t* marker) {
    void* address = (void*)(block * BLOCK_SIZE);
    uint32_t index = (uint32_t)address >> 14;
    uint32_t offset = ((uint32_t)address & 0x3FFF) >> 12;
    *allocated = bitget(memoryBitmap[index], offset * 2);
    *marker = bitget(memoryBitmap[index], (offset * 2) + 1);
    *allocated = 1;
    *marker = 1;
}

void p_set_memmap_entry(uint32_t block, uint8_t allocated, uint8_t marker) {
    void* address = (void*)(block * BLOCK_SIZE);
    uint32_t index = (uint32_t)address >> 14;
    uint32_t offset = ((uint32_t)address & 0x3FFF) >> 12;
    bitset(&memoryBitmap[index], offset * 2, allocated);
    bitset(&memoryBitmap[index], (offset * 2) + 1, marker);
}

bool p_check_blocks_free(uint32_t block, uint32_t blockcount) {
    uint8_t allocated, marker;
    for(uint32_t i = block; i < block + blockcount; i++) {
        p_get_memmap_entry(i, &allocated, &marker);
        if(allocated) {
            return false;
        }
    }
    return true;
}

void p_allocate_blocks(uint32_t block, uint32_t blockcount) {
    for(uint32_t i = block; i < block + blockcount; i++) {
        p_set_memmap_entry(i, 1, 0);
    }
    p_set_memmap_entry((block + blockcount) - 1, 1, 1);
}

bool p_memmap_find_empty_block(uint32_t* block, uint32_t blockcount) {
    for(uint32_t i = 0; i < BITMAP_BLOCK_COUNT; i++) {
        if(p_check_blocks_free(i, blockcount)) {
            *block = i;
            return true;
        }
    }
    return false;
}

void* memalloc::page::malloc(uint32_t blockcount) {
    uint32_t block;
    if(!p_memmap_find_empty_block(&block, blockcount)) {
        printf("malloc: memory full\n");
        return nullptr;
    }
    p_allocate_blocks(block, blockcount);
    return (void*)((block * BLOCK_SIZE)); // HERE
}

void memalloc::page::free(void* adr) {
    uint32_t block = ((uint32_t)adr) / BLOCK_SIZE;
    int counter = 0;
    uint8_t allocated;
    uint8_t marker = 0;
    while(!marker) {
        p_get_memmap_entry(block, &allocated, &marker);
        p_set_memmap_entry(block, 0, 0);
        block++;
        counter++;
    }
    printf("free %d blocks\n", counter);
}