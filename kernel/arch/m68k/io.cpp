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

uint8_t ioread8(io_handle_t handle) {
    return *((volatile uint8_t *)handle);
}

uint16_t ioread16(io_handle_t handle) {
    return *((volatile uint16_t *)handle);
}

uint32_t ioread32(io_handle_t handle) {
    return *((volatile uint32_t *)handle);
}

uint64_t ioread64(io_handle_t handle) {
    return *((volatile uint64_t *)handle);
}

void iowrite8(io_handle_t handle, uint8_t data) {
    *((volatile uint8_t *)handle) = data;
}

void iowrite16(io_handle_t handle, uint16_t data) {
    *((volatile uint16_t *)handle) = data;
}

void iowrite32(io_handle_t handle, uint32_t data) {
    *((volatile uint32_t *)handle) = data;
}

void iowrite64(io_handle_t handle, uint64_t data) {
    *((volatile uint64_t *)handle) = data;
}
