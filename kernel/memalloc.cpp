#include "memalloc.h"
#include "stdio.h"
#include "stdlib.h"

#define BLOCK_SIZE 4096

#define USER_MAX_MEM 4294963200

#define BITMAP_BLOCK_COUNT USER_MAX_MEM / BLOCK_SIZE
#define BITMAP_LEN (BITMAP_BLOCK_COUNT / (8 / 2)) + 1

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

void memalloc::page::init(memorymap::SMAP_entry* e620_map, int e620_len) {
    memset((uint8_t*)memoryBitmap, 0xFF, BITMAP_LEN); // set everything as used
    // first set all the areas marked as usable
    for(int i = 0; i < e620_len; i++) {
        uint64_t start = e620_map[i].Base / 4096;
        uint64_t end = (e620_map[i].Base + e620_map[i].Length) / 4096;
        if(end > BITMAP_BLOCK_COUNT) { end = BITMAP_BLOCK_COUNT - 1; }
        if(start > BITMAP_BLOCK_COUNT) { printf("memalloc: entry bigger than memory map\n"); continue; }
        if(e620_map[i].Type == 1) {
            for(uint32_t j = 0; j < (end - start); j++) {
                p_set_memmap_entry(start + j, 0, 0);
            }
        }
    }
    // now set all areas marked as unusable
    for(int i = 0; i < e620_len; i++) {
        uint64_t start = e620_map[i].Base / 4096;
        uint64_t end = (e620_map[i].Base + e620_map[i].Length) / 4096;
        if(end > BITMAP_BLOCK_COUNT) { end = BITMAP_BLOCK_COUNT - 1; }
        if(start > BITMAP_BLOCK_COUNT) { printf("memalloc: entry bigger than memory map\n"); continue; }
        if(e620_map[i].Type > 1) {
            for(uint32_t j = 0; j < (end - start); j++) {
                p_set_memmap_entry(start + j, 1, 1);
            }
        }
    }
    // mark the first 100MB as unusable since they are used by the kernel
    for(uint32_t j = 0; j < 24414; j++) { 
        p_set_memmap_entry(j, 1, 1);
    }
}