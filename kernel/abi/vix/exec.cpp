#include <vix/abi/vix/vix.h>
#include <vix/abi/execfmt/detect.h>
#include <vix/status.h>
#include <vix/abi/vix/arch.h>
#include <vix/config.h>

status::StatusOr<sched::tid_t> abi::vix::exec(const char *path, std::vector<std::string> *args) {
    std::pair<mm::mem_areas_t, void *> exec;
    ASSIGN_OR_RETURN(exec, execfmt::load_any(path));

    auto status = abi::vix::arch::start_thread(exec.first, exec.second, args);
    if (!status.status().ok()) {
        mm::mem_areas_deinit(&exec.first);
        return status.status().code();
    } else {
        return status.value();
    }
}
