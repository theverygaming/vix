#include <scheduler.h>

namespace schedulers {
    void generic_scheduler_singlethread::init(std::vector<generic_process *> *processes) {
        _processes = processes;
        currentProcess = -1;
        currentProcessIndex = 0;
    }

    bool generic_scheduler_singlethread::tick(bool *switch_, size_t *switch_index, bool *old, size_t *old_index) {
        *switch_ = false;
        *old = true;

        if (_processes->size() <= 0) {
            return false;
        }

        if (currentProcessIndex >= _processes->size()) { // something went horribly wrong...
            currentProcessIndex = 0;
        }

        if (currentProcess < 0) { // first startup, we have to instantly schedule a process
            if (reschedule(0)) {
                *switch_ = true;
                *switch_index = currentProcessIndex;
                *old = false;
                return true;
            } else {
                return false;
            }
        }

        if ((*_processes)[currentProcessIndex]->state != generic_process::state::RUNNING) {
            clearProcessesArray();
            if (reschedule(currentProcessIndex)) {
                *switch_ = true;
                *switch_index = currentProcessIndex;
                *old = false;
                return true;
            } else {
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
            *old_index = currentProcessIndex;
            if (reschedule(currentProcessIndex)) {
                *switch_ = true;
                *switch_index = currentProcessIndex;
                return true;
            } else {
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
                currentProcess = (*_processes)[index]->pid;
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

            if ((*_processes)[i]->pid == pid) {
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
                remove_process((*_processes)[i]->pid);
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
