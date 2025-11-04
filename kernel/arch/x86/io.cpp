#include <vix/kernel/io.h>
#include <vix/panic.h>

io_handle_t io_pmio_map(uintptr_t base) {
    if (base > 0xFFFF) {
        KERNEL_PANIC("invalid io_pmio_map address 0x%p", base);
    }
    return base;
}

io_handle_t io_mmio_map(uintptr_t base, size_t max_offset) {
    return 0; // FIXME: well that isn't gonna work very well
}

void io_unmap(io_handle_t handle) {}

uint8_t ioread8(io_handle_t handle) {
    // IO space
    if (handle <= 0xFFFF) {
        uint8_t value;
        asm volatile("inb %%dx, %%al" : "=a"(value) : "d"(handle));
        return value;
    }
    // MMIO space
    return *((volatile uint8_t *)handle);
}

uint16_t ioread16(io_handle_t handle) {
    // IO space
    if (handle <= 0xFFFF) {
        uint16_t value;
        asm volatile("inw %%dx, %%ax" : "=a"(value) : "d"(handle));
        return value;
    }
    // MMIO space
    return *((volatile uint16_t *)handle);
}

uint32_t ioread32(io_handle_t handle) {
    // IO space
    if (handle <= 0xFFFF) {
        uint32_t value;
        asm volatile("inl %%dx, %%eax" : "=a"(value) : "d"(handle));
        return value;
    }
    // MMIO space
    return *((volatile uint32_t *)handle);
}

uint64_t ioread64(io_handle_t handle) {
    // IO space
    if (handle <= 0xFFFF) {
        return 0; // ??? doesn't exist lmao
    }
    // MMIO space
    return *((volatile uint64_t *)handle);
}

void iowrite8(io_handle_t handle, uint8_t data) {
    // IO space
    if (handle <= 0xFFFF) {
        asm volatile("outb %%al, %%dx" ::"d"(handle), "a"(data));
        return;
    }
    // MMIO space
    *((volatile uint8_t *)handle) = data;
}

void iowrite16(io_handle_t handle, uint16_t data) {
    // IO space
    if (handle <= 0xFFFF) {
        asm volatile("outw %%ax, %%dx" ::"d"(handle), "a"(data));
        return;
    }
    // MMIO space
    *((volatile uint16_t *)handle) = data;
}

void iowrite32(io_handle_t handle, uint32_t data) {
    // IO space
    if (handle <= 0xFFFF) {
        asm volatile("outl %%eax, %%dx" ::"d"(handle), "a"(data));
        return;
    }
    // MMIO space
    *((volatile uint32_t *)handle) = data;
}

void iowrite64(io_handle_t handle, uint64_t data) {
    // IO space
    if (handle <= 0xFFFF) {
        return; // ??? doesn't exist lmao
    }
    // MMIO space
    *((volatile uint64_t *)handle) = data;
}
