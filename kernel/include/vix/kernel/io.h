#pragma once
#include <vix/types.h>

// typedef so it can be changed easier later on
typedef uintptr_t io_handle_t;

uint8_t ioread8(io_handle_t handle);
uint16_t ioread16(io_handle_t handle);
uint32_t ioread32(io_handle_t handle);
uint64_t ioread64(io_handle_t handle);
void iowrite8(io_handle_t handle, uint8_t data);
void iowrite16(io_handle_t handle, uint16_t data);
void iowrite32(io_handle_t handle, uint32_t data);
void iowrite64(io_handle_t handle, uint64_t data);

io_handle_t io_pio_map(uintptr_t base);
io_handle_t io_mmio_map(uintptr_t base, size_t max_offset);
