#include "elfimpl.h"

status::StatusOr<void *> execfmt::elf::load_elf64(arch::vmm::pt_t pt, std::shared_ptr<struct vfs::vnode> elf_file) {
    return load_elf<struct elf64_header, struct elf64_program_header>(pt, elf_file);
}
