#include <types.h>

static uintptr_t min_addr = 0;
static uintptr_t max_addr = 0;
static uint32_t *bitmap = nullptr;

// TODO: memory map cleanup function (e820 sanetize in linux)
// TODO: generic memory map functionality, ACPI support in memory map
// TODO: memory map align to block size
