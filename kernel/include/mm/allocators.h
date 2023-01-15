#pragma once
#include <stdlib.h>
#include <types.h>

namespace mm::allocators {
    template <size_t max_block_count, size_t block_size> class block_alloc {
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

        void alloc(void *offset_adr, size_t blockcount) {
            size_t block = (size_t)offset_adr / block_size;
            p_allocate_blocks(block, blockcount);
        }

        void dealloc(void *offset_adr, size_t blockcount) {
            size_t block = (size_t)offset_adr / block_size;
            p_deallocate_blocks(block, blockcount);
        }

        void *malloc(size_t blockcount, bool *successful) {
            size_t block;
            if (!p_memmap_find_empty_block(&block, blockcount)) {
                *successful = false;
                return nullptr;
            }
            p_allocate_blocks(block, blockcount);
            *successful = true;
            return (void *)((block * block_size));
        }

        /* if this returns a different address the user is expected to copy the data if it returns unsuccessful the allocated memory is kept as it was before */
        void *realloc(void *offset_adr, size_t blockcount, bool *successful) {
            size_t old_block_size;
            p_memmap_check_allocated_block_size(((size_t)offset_adr) / block_size, &old_block_size);
            free(offset_adr);
            void *newptr = malloc(blockcount, successful);
            if (!(*successful)) {
                alloc(offset_adr, old_block_size);
                return offset_adr;
            }
            return newptr;
        }

        void free(void *offset_adr) {
            size_t block = ((size_t)offset_adr) / block_size;
            uint8_t allocated;
            uint8_t marker = 0;
            while (!marker) {
                p_get_memmap_entry(block, &allocated, &marker);
                p_set_memmap_entry(block, 0, 0);
                if (!allocated) {
                    break;
                }
                block++;
            }
        }

        size_t count_free_blocks() {
            size_t count = 0;
            for (size_t i = 0; i < max_block_count; i++) {
                uint8_t allocated;
                uint8_t marker;
                p_get_memmap_entry(i, &allocated, &marker);
                if (allocated == 0) {
                    count++;
                }
            }
            return count;
        }

    private:
        /* each block is two bits, the first bit marks if the block is allocated or not, the second bit marks the end of a block */
        uint8_t allocator_bitmap[(max_block_count / (8 / 2)) + 1]; // + 1 in case max_block_count is not divisible by 4, could otherwise end up with a too small bitmap

        void p_get_memmap_entry(size_t block, uint8_t *allocated, uint8_t *marker) {
            if (block >= max_block_count) {
                // mark as allocated
                *allocated = 1;
                *marker = 1;
                return;
            }
            size_t index = block / 4;
            size_t offset = block % 4;
            *allocated = bitget(allocator_bitmap[index], offset * 2);
            *marker = bitget(allocator_bitmap[index], (offset * 2) + 1);
        }

        void p_set_memmap_entry(size_t block, uint8_t allocated, uint8_t marker) {
            if (block >= max_block_count) {
                return;
            }
            size_t index = block / 4;
            size_t offset = block % 4;
            bitset(&allocator_bitmap[index], offset * 2, allocated);
            bitset(&allocator_bitmap[index], (offset * 2) + 1, marker);
        }

        bool p_check_blocks_free(size_t block, size_t blockcount) {
            uint8_t allocated, marker;
            for (size_t i = block; i < block + blockcount; i++) {
                if (i >= max_block_count) {
                    return false;
                }
                p_get_memmap_entry(i, &allocated, &marker);
                if (allocated) {
                    return false;
                }
            }
            return true;
        }

        void p_allocate_blocks(size_t block, size_t blockcount) {
            for (size_t i = block; i < block + blockcount; i++) {
                p_set_memmap_entry(i, 1, 0);
            }
            p_set_memmap_entry((block + blockcount) - 1, 1, 1);
        }

        void p_deallocate_blocks(size_t block, size_t blockcount) {
            for (size_t i = block; i < block + blockcount; i++) {
                p_set_memmap_entry(i, 0, 0);
            }
        }

        bool p_memmap_find_empty_block(size_t *block, size_t blockcount) {
            for (size_t i = 0; i < max_block_count; i++) {
                if (p_check_blocks_free(i, blockcount)) {
                    *block = i;
                    return true;
                }
            }
            return false;
        }

        void p_memmap_check_allocated_block_size(size_t block, size_t *blocksize) {
            size_t counter = 0;
            uint8_t allocated;
            uint8_t marker = 0;
            while (!marker) {
                if (block >= max_block_count) {
                    *blocksize = counter;
                    return;
                }
                p_get_memmap_entry(block, &allocated, &marker);
                if (!allocated) {
                    break;
                }
                block++;
                counter++;
            }
            *blocksize = counter;
        }

        /* helper functions */
        void bitset(uint8_t *byte, uint8_t bitnum, uint8_t value) {
            *byte ^= (-value ^ *byte) & (1 << bitnum);
        }

        uint8_t bitget(uint8_t byte, uint8_t bitnum) {
            return (byte >> bitnum) & 1;
        }
    };
    template <size_t max_block_count, size_t block_size> class block_alloc_single {
    public:
        void init() {
            clear();
        }

        void clear() {
            memset(allocator_bitmap, 0, (max_block_count / 8) + 1);
        }

        void markAllUsed() {
            memset(allocator_bitmap, 0xFF, (max_block_count / 8) + 1);
        }

        void alloc(void *offset_adr, size_t blockcount) {
            size_t block = (size_t)offset_adr / block_size;
            p_allocate_blocks(block, blockcount);
        }

        void dealloc(void *offset_adr, size_t blockcount) {
            size_t block = (size_t)offset_adr / block_size;
            p_deallocate_blocks(block, blockcount);
        }

        void *malloc(size_t blockcount, bool *successful) {
            size_t block;
            if (!p_memmap_find_empty_block(&block, blockcount)) {
                *successful = false;
                return nullptr;
            }
            p_allocate_blocks(block, blockcount);
            *successful = true;
            return (void *)((block * block_size));
        }

        size_t count_free_blocks() {
            size_t count = 0;
            for (size_t i = 0; i < max_block_count; i++) {
                uint8_t allocated;
                p_get_memmap_entry(i, &allocated);
                if (allocated == 0) {
                    count++;
                }
            }
            return count;
        }

    private:
        uint8_t allocator_bitmap[(max_block_count / 8) + 1]; // + 1 in case max_block_count is not divisible by 4, could otherwise end up with a too small bitmap

        void p_get_memmap_entry(size_t block, uint8_t *allocated) {
            if (block >= max_block_count) {
                // mark as allocated
                *allocated = 1;
                return;
            }
            size_t index = block / 8;
            size_t offset = block % 8;
            *allocated = bitget(allocator_bitmap[index], offset);
        }

        void p_set_memmap_entry(size_t block, uint8_t allocated) {
            if (block >= max_block_count) {
                return;
            }
            size_t index = block / 8;
            size_t offset = block % 8;
            bitset(&allocator_bitmap[index], offset, allocated);
        }

        bool p_check_blocks_free(size_t block, size_t blockcount) {
            uint8_t allocated;
            for (size_t i = block; i < block + blockcount; i++) {
                if (i >= max_block_count) {
                    return false;
                }
                p_get_memmap_entry(i, &allocated);
                if (allocated) {
                    return false;
                }
            }
            return true;
        }

        void p_allocate_blocks(size_t block, size_t blockcount) {
            for (size_t i = block; i <= block + blockcount; i++) {
                p_set_memmap_entry(i, 1);
            }
        }

        void p_deallocate_blocks(size_t block, size_t blockcount) {
            for (size_t i = block; i < block + blockcount; i++) {
                p_set_memmap_entry(i, 0);
            }
        }

        bool p_memmap_find_empty_block(size_t *block, size_t blockcount) {
            for (size_t i = 0; i < max_block_count; i++) {
                if (p_check_blocks_free(i, blockcount)) {
                    *block = i;
                    return true;
                }
            }
            return false;
        }

        void p_memmap_check_allocated_block_size(size_t block, size_t *blocksize) {
            size_t counter = 0;
            uint8_t allocated;
            uint8_t marker = 0;
            while (!marker) {
                if (block >= max_block_count) {
                    *blocksize = counter;
                    return;
                }
                p_get_memmap_entry(block, &allocated, &marker);
                if (!allocated) {
                    break;
                }
                block++;
                counter++;
            }
            *blocksize = counter;
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
