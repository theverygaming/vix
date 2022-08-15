#pragma once
#include <types.h>
#include <stdlib.h>

namespace memalloc::allocators {
    template <uint32_t max_block_count, size_t block_size> class block_alloc {
    public:
        void init() {
            clear();
        }

        void clear() {
            memset(allocator_bitmap, 0, (max_block_count / (8 / 2)) + 1);
        }

        void markAllUsed() {
            memset(allocator_bitmap, 0xFF, (max_block_count / (8 / 2)) + 1);
        }

        void alloc(void *offset_adr, uint32_t blockcount) {
            uint32_t block = (uint32_t)offset_adr / block_size;
            p_allocate_blocks(block, blockcount);
        }

        void dealloc(void *offset_adr, uint32_t blockcount) {
            uint32_t block = (uint32_t)offset_adr / block_size;
            p_deallocate_blocks(block, blockcount);
        }

        void *malloc(uint32_t blockcount, bool *successful) {
            uint32_t block;
            if (!p_memmap_find_empty_block(&block, blockcount)) {
                *successful = false;
                return nullptr;
            }
            p_allocate_blocks(block, blockcount);
            *successful = true;
            return (void *)((block * block_size));
        }

        void free(void *address) {
            uint32_t block = ((uint32_t)address) / block_size;
            int counter = 0;
            uint8_t allocated;
            uint8_t marker = 0;
            while (!marker) {
                p_get_memmap_entry(block, &allocated, &marker);
                p_set_memmap_entry(block, 0, 0);
                if (!allocated) {
                    break;
                }
                block++;
                counter++;
            }
        }

    private:
        /* each block is two bits, the first bit marks if the block is allocated or not, the second bit marks the end of a block */
        uint8_t allocator_bitmap[(max_block_count / (8 / 2)) + 1]; // + 1 in case max_block_count is not divisible by 4, could otherwise end up with a too small bitmap

        void p_get_memmap_entry(uint32_t block, uint8_t *allocated, uint8_t *marker) {
            void *address = (void *)(block * block_size);
            uint32_t index = (uint32_t)address / block_size;
            uint32_t offset = ((uint32_t)address & ((block_size * (8 / 2)) - 1)) >> 12;
            *allocated = bitget(allocator_bitmap[index], offset * 2);
            *marker = bitget(allocator_bitmap[index], (offset * 2) + 1);
        }

        void p_set_memmap_entry(uint32_t block, uint8_t allocated, uint8_t marker) {
            void *address = (void *)(block * block_size);
            uint32_t index = (uint32_t)address / block_size;
            uint32_t offset = ((uint32_t)address & ((block_size * (8 / 2)) - 1)) >> 12;
            bitset(&allocator_bitmap[index], offset * 2, allocated);
            bitset(&allocator_bitmap[index], (offset * 2) + 1, marker);
        }

        bool p_check_blocks_free(uint32_t block, uint32_t blockcount) {
            uint8_t allocated, marker;
            for (uint32_t i = block; i < block + blockcount; i++) {
                p_get_memmap_entry(i, &allocated, &marker);
                if (allocated) {
                    return false;
                }
            }
            return true;
        }

        void p_allocate_blocks(uint32_t block, uint32_t blockcount) {
            for (uint32_t i = block; i < block + blockcount; i++) {
                p_set_memmap_entry(i, 1, 0);
            }
            p_set_memmap_entry((block + blockcount) - 1, 1, 1);
        }

        void p_deallocate_blocks(uint32_t block, uint32_t blockcount) {
            for (uint32_t i = block; i < block + blockcount; i++) {
                p_set_memmap_entry(i, 0, 0);
            }
        }

        bool p_memmap_find_empty_block(uint32_t *block, uint32_t blockcount) {
            for (uint32_t i = 0; i < max_block_count; i++) {
                if (p_check_blocks_free(i, blockcount)) {
                    *block = i;
                    return true;
                }
            }
            return false;
        }

        /* helper functions */
        void bitset(uint8_t *byte, uint8_t bitnum, uint8_t value) {
            *byte ^= (-value ^ *byte) & (1 << bitnum);
        }

        uint8_t bitget(uint8_t byte, uint8_t bitnum) {
            return (byte >> bitnum) & 1;
        }
    };
}