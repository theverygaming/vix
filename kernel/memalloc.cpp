#include "memalloc.h"
#include "stdio.h"

#define BLOCK_SIZE 4096

#define USER_MAX_MEM 4294959104
#define USER_MIN_MEM 99999744 // lower 100MB of physical memory for kernel

#define BITMAP_LEN ((USER_MAX_MEM - USER_MIN_MEM) / BLOCK_SIZE) / (8 / 2)

uint8_t memoryBitmap[BITMAP_LEN] = {0}; // each block is two bits, the first bit marks if the block is allocated or not, the second bit marks the end of a block

uint8_t bitset(uint8_t* byte, uint8_t bitnum, uint8_t value) {
    *byte ^= (-value ^ *byte) & (1 << bitnum); 
}

uint8_t bitget(uint8_t byte, uint8_t bitnum) {
    return (byte >> bitnum) & 1;
}

void* memmap_to_adr(uint32_t index, uint32_t offset) {
    return (void*)((index << 14) | (offset << 12));
}

void adr_to_memmap(uint32_t* index, uint32_t* offset, void* address) {
    *index = (uint32_t)address >> 14;
    *offset = ((uint32_t)address & 0x3FFF) >> 12;
}

void set_memmap_entry(uint8_t allocated, uint8_t marker, uint32_t offset, uint8_t* entry) {
    bitset(entry, offset * 2, allocated);
    bitset(entry, (offset * 2) + 1, marker);
}

void get_memmap_entry(uint8_t* allocated, uint8_t* marker, uint32_t offset, uint8_t entry) {
    *allocated = bitget(entry, offset * 2);
    *marker = bitget(entry, (offset * 2) + 1);
}

bool check_blocks_free(uint32_t index, uint32_t offset, uint32_t blocks) {
    for(uint32_t i = 0; i < blocks; i++) {
        uint8_t allocated, marker;
        get_memmap_entry(&allocated, &marker, offset, memoryBitmap[index]);
        if(allocated) {
            return false;
        }
    }
    printf("found a free block @i->%u o->%u\n", index, offset);
    return true;
}

void allocate_blocks(uint32_t index, uint32_t offset, uint32_t blocks) {
    while(blocks) {
        for(; offset < 4; offset++) {
            set_memmap_entry(1, 0, offset, &memoryBitmap[index]);
            if(!--blocks) { break; }
        }
        if(!blocks) { break; }
        offset = 0;
        index++;
    }
    set_memmap_entry(1, 1, offset, &memoryBitmap[index]);
}

bool memmap_find_empty_block(uint32_t* index, uint32_t* offset, uint32_t blocks) {
    for(uint32_t i = 0; i < BITMAP_LEN; i++) { // this is bad, make it better
        for(uint32_t j = 0; j < 4; j++) {
            if(check_blocks_free(i, j, blocks)) {
                *index = i;
                *offset = j;
                return true;
            }
        }
    }
    return false;
}

void* memalloc::malloc(uint32_t blocks) {
    uint32_t index, offset;
    memmap_find_empty_block(&index, &offset, blocks);
    allocate_blocks(index, offset, blocks);
    return memmap_to_adr(index, offset);
}

void memalloc::free(void* adr) {
    uint32_t index, offset;
    adr_to_memmap(&index, &offset, adr);
    int counter = 0;
    uint8_t allocated, marker;
    while(1) {
        for(; offset < 4; offset++) {
            counter++;
            get_memmap_entry(&allocated, &marker, offset, memoryBitmap[index]);
            set_memmap_entry(0, 0, offset, &memoryBitmap[index]);
            if(marker) { printf("freed %d blocks\n", counter); return; }
        }
        offset = 0;
        index++;
    }
}