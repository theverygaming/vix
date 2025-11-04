#pragma once
#include <vix/arch/io.h>

// io_handle_t is a typedef (the architecture may abstract it away)

uint8_t ioread8(io_handle_t handle);
uint16_t ioread16(io_handle_t handle);
uint32_t ioread32(io_handle_t handle);
uint64_t ioread64(io_handle_t handle);
void iowrite8(io_handle_t handle, uint8_t data);
void iowrite16(io_handle_t handle, uint16_t data);
void iowrite32(io_handle_t handle, uint32_t data);
void iowrite64(io_handle_t handle, uint64_t data);

// port-mapped IO
io_handle_t io_pmio_map(uintptr_t base, size_t max_offset);

// memory-mapped IO
io_handle_t io_mmio_map(uintptr_t base, size_t max_offset);
void io_unmap(io_handle_t handle);
