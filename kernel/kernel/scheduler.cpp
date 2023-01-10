#include <debug.h>
#include <scheduler.h>

namespace schedulers {
    void generic_scheduler_singlethread::init(std::vector<generic_process *> *processes, void (*load_process)(generic_process *, void *), void (*unload_process)(generic_process *, void *)) {
        _processes = processes;
        currentProcess = -1;
        currentProcessIndex = 0;

        _load_process = load_process;
        _unload_process = unload_process;
    }

    bool generic_scheduler_singlethread::tick(void *ctx) {
        if (_processes->size() <= 0) {
            return false;
        }

        if (currentProcessIndex >= _processes->size()) { // something went horribly wrong...
            currentProcessIndex = 0;
        }

        if (currentProcess < 0) { // first startup, we have to instantly schedule a process
            if (reschedule(0)) {
                _load_process((*_processes)[currentProcessIndex], ctx);
                return true;
            } else {
                DEBUG_PRINTF("sched: no startup process\n");
                return false;
            }
        }

        if ((*_processes)[currentProcessIndex]->state != generic_process::state::RUNNING) {
            if (((*_processes)[currentProcessIndex]->state == generic_process::state::REPLACED) || ((*_processes)[currentProcessIndex]->state == generic_process::state::KILLED)) {
                clearProcessesArray();
            } else {
                _unload_process((*_processes)[currentProcessIndex], ctx);
            }
            if (reschedule(currentProcessIndex)) {
                _load_process((*_processes)[currentProcessIndex], ctx);
                return true;
            } else {
                DEBUG_PRINTF("sched: no runnable (prev not running)\n");
                return false;
            }
        }

        if (_processes->size() == 1) { // we are only running a single process, reduce kernel time by not even trying to switch processes
            return true;
        }

        (*_processes)[currentProcessIndex]->used_cputime++;
        if ((*_processes)[currentProcessIndex]->used_cputime >= (*_processes)[currentProcessIndex]->cputime) {
            (*_processes)[currentProcessIndex]->state = generic_process::state::RUNNABLE;
            (*_processes)[currentProcessIndex]->used_cputime = 0;
            _unload_process((*_processes)[currentProcessIndex], ctx);
            if (reschedule(currentProcessIndex)) {
                _load_process((*_processes)[currentProcessIndex], ctx);
                return true;
            } else {
                DEBUG_PRINTF("sched: no runnable\n");
                return false;
            }
        }
        return true;
    }

    bool generic_scheduler_singlethread::reschedule(size_t oldProcessIndex) {
        size_t index = oldProcessIndex + 1;
        size_t count = 0;
        size_t processArraySize = _processes->size();
        while (count < processArraySize) {
            if (index >= processArraySize) {
                index = 0;
            }

            if ((*_processes)[index]->state == generic_process::state::RUNNABLE) {
                (*_processes)[index]->state = generic_process::state::RUNNING;
                currentProcess = (*_processes)[index]->tgid;
                currentProcessIndex = index;
                return true;
            }

            count++;
            index++;
        }
        return false;
    }

    void generic_scheduler_singlethread::remove_process(pid_t pid) {
        size_t i = 0;
        while (i < _processes->size()) {
            if ((*_processes)[i]->parent == pid) { // child processes are given to PID 1
                (*_processes)[i]->parent = 1;
            }

            if ((*_processes)[i]->tgid == pid) {
                delete (*_processes)[i];
                _processes->erase(i);
                i--;
            }
            i++;
        }
    }

    // this function could be optimized
    void generic_scheduler_singlethread::clearProcessesArray() {
        size_t i = 0;
        while (i < _processes->size()) {
            if ((*_processes)[i]->state == generic_process::state::KILLED) {
                remove_process((*_processes)[i]->tgid);
                clearProcessesArray();
                break;
            }

            if ((*_processes)[i]->state == generic_process::state::REPLACED) {
                delete (*_processes)[i];
                _processes->erase(i);
                i--;
            }
            i++;
        }
    }
}
