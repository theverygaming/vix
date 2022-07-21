#include "memalloc.h"
#include "stdio.h"
#include "stdlib.h"
#include "../config.h"

#define PAGE_SIZE 4096

#define PHYS_MAX_MEM 4294963200

#define PHYS_BITMAP_BLOCK_COUNT PHYS_MAX_MEM / PAGE_SIZE
#define PHYS_BITMAP_LEN (PHYS_BITMAP_BLOCK_COUNT / (8 / 2)) + 1

uint8_t phys_memoryBitmap[PHYS_BITMAP_LEN]; // each block is two bits, the first bit marks if the block is allocated or not, the second bit marks the end of a block

#define KERNEL_PAGES KERNEL_MEMORY_END_OFFSET / 4096
#define KERNEL_BITMAP_LEN (KERNEL_PAGES / (8 / 2)) + 1

uint8_t kernel_memoryBitmap[KERNEL_BITMAP_LEN];


void bitset(uint8_t* byte, uint8_t bitnum, uint8_t value) {
    *byte ^= (-value ^ *byte) & (1 << bitnum); 
}

uint8_t bitget(uint8_t byte, uint8_t bitnum) {
    return (byte >> bitnum) & 1;
}

void p_get_memmap_entry(uint32_t block, uint8_t* allocated, uint8_t* marker, uint8_t* memoryBitmap) {
    void* address = (void*)(block * PAGE_SIZE);
    uint32_t index = (uint32_t)address >> 14;
    uint32_t offset = ((uint32_t)address & 0x3FFF) >> 12;
    *allocated = bitget(memoryBitmap[index], offset * 2);
    *marker = bitget(memoryBitmap[index], (offset * 2) + 1);
}

void p_set_memmap_entry(uint32_t block, uint8_t allocated, uint8_t marker, uint8_t* memoryBitmap) {
    void* address = (void*)(block * PAGE_SIZE);
    uint32_t index = (uint32_t)address >> 14;
    uint32_t offset = ((uint32_t)address & 0x3FFF) >> 12;
    bitset(&memoryBitmap[index], offset * 2, allocated);
    bitset(&memoryBitmap[index], (offset * 2) + 1, marker);
}

bool p_check_blocks_free(uint32_t block, uint32_t blockcount, uint8_t* memoryBitmap) {
    uint8_t allocated, marker;
    for(uint32_t i = block; i < block + blockcount; i++) {
        p_get_memmap_entry(i, &allocated, &marker, memoryBitmap);
        if(allocated) {
            return false;
        }
    }
    return true;
}

void p_allocate_blocks(uint32_t block, uint32_t blockcount, uint8_t* memoryBitmap) {
    for(uint32_t i = block; i < block + blockcount; i++) {
        p_set_memmap_entry(i, 1, 0, memoryBitmap);
    }
    p_set_memmap_entry((block + blockcount) - 1, 1, 1, memoryBitmap);
}

bool p_memmap_find_empty_block(uint32_t* block, uint32_t blockcount, uint8_t* memoryBitmap) {
    for(uint32_t i = 0; i < PHYS_BITMAP_BLOCK_COUNT; i++) {
        if(p_check_blocks_free(i, blockcount, memoryBitmap)) {
            *block = i;
            return true;
        }
    }
    return false;
}

void* memalloc::page::phys_malloc(uint32_t blockcount) {
    uint32_t block;
    if(!p_memmap_find_empty_block(&block, blockcount, phys_memoryBitmap)) {
        printf("malloc: memory full\n");
        return nullptr;
    }
    p_allocate_blocks(block, blockcount, phys_memoryBitmap);
    return (void*)((block * PAGE_SIZE));
}

void memalloc::page::phys_free(void* adr) {
    uint32_t block = ((uint32_t)adr) / PAGE_SIZE;
    int counter = 0;
    uint8_t allocated;
    uint8_t marker = 0;
    while(!marker) {
        p_get_memmap_entry(block, &allocated, &marker, phys_memoryBitmap);
        p_set_memmap_entry(block, 0, 0, phys_memoryBitmap);
        if(!allocated) { break; }
        block++;
        counter++;
    }
    printf("free %d blocks\n", counter);
}

void memalloc::page::phys_init(memorymap::SMAP_entry* e620_map, int e620_len) {
    memset((uint8_t*)phys_memoryBitmap, 0xFF, PHYS_BITMAP_LEN); // set everything as used
    // first set all the areas marked as usable
    for(int i = 0; i < e620_len; i++) {
        uint64_t start = e620_map[i].Base / 4096;
        uint64_t end = (e620_map[i].Base + e620_map[i].Length) / 4096;
        if(end > PHYS_BITMAP_BLOCK_COUNT) { end = PHYS_BITMAP_BLOCK_COUNT - 1; }
        if(start > PHYS_BITMAP_BLOCK_COUNT) { printf("memalloc: entry bigger than memory map\n"); continue; }
        if(e620_map[i].Type == 1) {
            for(uint32_t j = 0; j < (end - start); j++) {
                p_set_memmap_entry(start + j, 0, 0, phys_memoryBitmap);
            }
        }
    }
    // now set all areas marked as unusable
    for(int i = 0; i < e620_len; i++) {
        uint64_t start = e620_map[i].Base / 4096;
        uint64_t end = (e620_map[i].Base + e620_map[i].Length) / 4096;
        if(end > PHYS_BITMAP_BLOCK_COUNT) { end = PHYS_BITMAP_BLOCK_COUNT - 1; }
        if(start > PHYS_BITMAP_BLOCK_COUNT) { printf("memalloc: entry bigger than memory map\n"); continue; }
        if(e620_map[i].Type > 1) {
            for(uint32_t j = 0; j < (end - start); j++) {
                p_set_memmap_entry(start + j, 1, 1, phys_memoryBitmap);
            }
        }
    }
    // mark the first 100MB as unusable since they are used by the kernel
    for(uint32_t j = 0; j < KERNEL_PAGES; j++) { 
        p_set_memmap_entry(j, 1, 1, phys_memoryBitmap);
    }
}



void* memalloc::page::kernel_malloc(uint32_t blockcount) {
    uint32_t block;
    if(!p_memmap_find_empty_block(&block, blockcount, kernel_memoryBitmap)) {
        printf("malloc: memory full\n");
        for(;;);
        return nullptr;
    }
    p_allocate_blocks(block, blockcount, kernel_memoryBitmap);
    return (void*)((block * PAGE_SIZE) + KERNEL_VIRT_ADDRESS);
}

void memalloc::page::kernel_alloc(void* adr, uint32_t blockcount) {
    adr -= KERNEL_VIRT_ADDRESS;
    uint32_t block = (uint32_t)adr / 4096;
    p_allocate_blocks(block, blockcount, kernel_memoryBitmap);
}

void memalloc::page::kernel_free(void* adr) {
    adr -= KERNEL_VIRT_ADDRESS;
    uint32_t block = ((uint32_t)adr) / PAGE_SIZE;
    int counter = 0;
    uint8_t allocated;
    uint8_t marker = 0;
    while(!marker) {
        p_get_memmap_entry(block, &allocated, &marker, kernel_memoryBitmap);
        p_set_memmap_entry(block, 0, 0, kernel_memoryBitmap);
        if(!allocated) { break; }
        block++;
        counter++;
    }
}

void memalloc::page::kernel_init() {
    memset((uint8_t*)kernel_memoryBitmap, 0, KERNEL_BITMAP_LEN); // set everything as free
    for(int i = 0; i < (KERNEL_FREE_AREA_BEGIN_OFFSET / 4096); i++) {
        p_set_memmap_entry(i, 1, 1, kernel_memoryBitmap);
    }
}