#include <vix/kernel/io.h>
#include <vix/panic.h>

#define PMIO_MAX (0xFFFF)

io_handle_t io_pmio_map(uintptr_t base, size_t max_offset) {
    if (base > PMIO_MAX) {
        KERNEL_PANIC("invalid io_pmio_map address 0x%p", base);
    }
    if ((base + max_offset) > PMIO_MAX) {
        KERNEL_PANIC(
            "io_pmio_map offset too large address: 0x%p max offset: 0x%p",
            base,
            max_offset
        );
    }
    return base;
}

io_handle_t io_mmio_map(uintptr_t base, size_t max_offset) {
    if (base <= PMIO_MAX) {
        KERNEL_PANIC("invalid io_mmio_map address 0x%p", base);
    }
    return 0; // FIXME: well that isn't gonna work very well
}

void io_unmap(io_handle_t handle) {
    if (handle > PMIO_MAX) {
        // MMIO space
        // FIXME: well that isn't gonna work very well
        return;
    }
}

uint8_t ioread8(io_handle_t handle) {
    // IO space
    if (handle <= PMIO_MAX) {
        uint8_t value;
        asm volatile("inb %%dx, %%al" : "=a"(value) : "d"(handle));
        return value;
    }
    // MMIO space
    return *((volatile uint8_t *)handle);
}

uint16_t ioread16(io_handle_t handle) {
    // IO space
    if (handle <= PMIO_MAX) {
        uint16_t value;
        asm volatile("inw %%dx, %%ax" : "=a"(value) : "d"(handle));
        return value;
    }
    // MMIO space
    return *((volatile uint16_t *)handle);
}

uint32_t ioread32(io_handle_t handle) {
    // IO space
    if (handle <= PMIO_MAX) {
        uint32_t value;
        asm volatile("inl %%dx, %%eax" : "=a"(value) : "d"(handle));
        return value;
    }
    // MMIO space
    return *((volatile uint32_t *)handle);
}

uint64_t ioread64(io_handle_t handle) {
    // IO space
    if (handle <= PMIO_MAX) {
        return 0; // ??? doesn't exist lmao
    }
    // MMIO space
    return *((volatile uint64_t *)handle);
}

void iowrite8(io_handle_t handle, uint8_t data) {
    // IO space
    if (handle <= PMIO_MAX) {
        asm volatile("outb %%al, %%dx" ::"d"(handle), "a"(data));
        return;
    }
    // MMIO space
    *((volatile uint8_t *)handle) = data;
}

void iowrite16(io_handle_t handle, uint16_t data) {
    // IO space
    if (handle <= PMIO_MAX) {
        asm volatile("outw %%ax, %%dx" ::"d"(handle), "a"(data));
        return;
    }
    // MMIO space
    *((volatile uint16_t *)handle) = data;
}

void iowrite32(io_handle_t handle, uint32_t data) {
    // IO space
    if (handle <= PMIO_MAX) {
        asm volatile("outl %%eax, %%dx" ::"d"(handle), "a"(data));
        return;
    }
    // MMIO space
    *((volatile uint32_t *)handle) = data;
}

void iowrite64(io_handle_t handle, uint64_t data) {
    // IO space
    if (handle <= PMIO_MAX) {
        return; // ??? doesn't exist lmao
    }
    // MMIO space
    *((volatile uint64_t *)handle) = data;
}
