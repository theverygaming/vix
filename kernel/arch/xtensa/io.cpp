#include <vix/kernel/io.h>
#include <vix/panic.h>

io_handle_t io_pmio_map(uintptr_t base, size_t max_offset) {
    KERNEL_PANIC("pmio not supported on this architecture");
}

io_handle_t io_mmio_map(uintptr_t base, size_t max_offset) {
    KERNEL_PANIC("mmio not implemented on this architecture");
}

void io_unmap(io_handle_t handle) {
    // TODO: mmio unmap
}

uint8_t ioread8(io_handle_t handle, size_t offset) {
    return *((volatile uint8_t *)handle + offset);
}

uint16_t ioread16(io_handle_t handle, size_t offset) {
    return *((volatile uint16_t *)handle + offset);
}

uint32_t ioread32(io_handle_t handle, size_t offset) {
    return *((volatile uint32_t *)handle + offset);
}

uint64_t ioread64(io_handle_t handle, size_t offset) {
    return *((volatile uint64_t *)handle + offset);
}

void iowrite8(io_handle_t handle, size_t offset, uint8_t data) {
    *((volatile uint8_t *)handle + offset) = data;
}

void iowrite16(io_handle_t handle, size_t offset, uint16_t data) {
    *((volatile uint16_t *)handle + offset) = data;
}

void iowrite32(io_handle_t handle, size_t offset, uint32_t data) {
    *((volatile uint32_t *)handle + offset) = data;
}

void iowrite64(io_handle_t handle, size_t offset, uint64_t data) {
    *((volatile uint64_t *)handle + offset) = data;
}
