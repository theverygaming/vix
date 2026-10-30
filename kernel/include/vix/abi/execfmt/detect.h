#pragma once
#include <vix/mm/mm.h>
#include <vix/status.h>
#include <utility>
#include <memory>
#include <vix/fs/vfs.h>
#include <vix/config.h>

namespace execfmt {
    status::StatusOr<std::pair<mm::mem_areas_t, void *>> load_any(const char *path);
    status::StatusOr<std::pair<mm::mem_areas_t, void *>> load_any(std::shared_ptr<struct vfs::vnode> exec_file);
}
