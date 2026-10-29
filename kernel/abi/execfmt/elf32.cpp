#include "elfimpl.h"

status::StatusOr<void *> execfmt::elf::load_elf32(mm::mem_areas_t *areas, std::shared_ptr<struct vfs::vnode> elf_file) {
    return load_elf<struct elf32_header, struct elf32_program_header>(areas, elf_file);
}
