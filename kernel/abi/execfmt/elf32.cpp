#include "elfimpl.h"

status::StatusOr<void *> execfmt::elf::load_elf32(arch::vmm::pt_t pt, std::shared_ptr<struct vfs::vnode> elf_file) {
    return load_elf<struct elf32_header, struct elf32_program_header>(pt, elf_file);
}
