#include <vix/abi/linux/linux.h>
#include <vix/abi/execfmt/detect.h>
#include <vix/status.h>
#include <vix/abi/linux/arch.h>
#include <vix/config.h>

status::StatusOr<sched::tid_t> abi::linux::exec(const char *path, std::vector<std::string> *args) {
#ifdef CONFIG_ARCH_HAS_PAGING
    std::pair<mm::mem_areas_t, void *> exec;
    ASSIGN_OR_RETURN(exec, execfmt::load_any(path));

    auto status = abi::linux::arch::start_process(exec.first, exec.second, args);
    if (!status.status().ok()) {
        mm::mem_areas_deinit(&exec.first);
        return status.status().code();
    } else {
        return status.value();
    }
#else
    return status::StatusCode::EGENERIC;
#endif
}
