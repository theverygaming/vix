#pragma once
#include <cppstd/vector.h>
#include <types.h>

namespace schedulers {
    class generic_process {
    public:
        pid_t pid;
        pid_t parent = -1;
        uint8_t cputime = 0; // process will run for cputime + 1 ticks

        // variables
        /*
         * RUNNING -> running
         * RUNNABLE -> will be scheduled
         * SLEEP -> can be interrupted, responds to signals etc.
         * UNINTERRUPTIBLE_SLEEP -> IO sleep etc. does not respond to signals
         * ZOMBIE -> waiting for parent to read exit code
         * STOPPED -> paused process, by CTRL+Z or similar
         * REPLACED -> for internal scheduler use. This process will be removed from the processes vector
         *     -> must have been replaced by another process with the same PID.
         * KILLED -> this process has been killed and any of it's children will be given to PID 1
         */
        enum class state { RUNNING, RUNNABLE, SLEEP, UNINTERRUPTIBLE_SLEEP, ZOMBIE, STOPPED, REPLACED, KILLED} state = state::RUNNABLE;
        uint8_t used_cputime = 0;
    };

    /*
     * This scheduler is not threadsafe. New processes may be popped into the end of the processes vector while running.
     * A process may only be removed from the vector by setting it's state to REPLACED or KILLED
     */
    class generic_scheduler_singlethread {
    public:
        void init(std::vector<generic_process *> *processes);

        /*
         * called every tick(timer interrupt or similar)
         * returns true unless we have a problem(for example ran out of processes to schedule)
         * takes two pointers, switch and switch_index. Switch will be set true when a process switch is needed,
         * then switch_index will be set to the index in the processes vector that shall be switched to.
         */
        bool tick(bool *switch_, size_t *switch_index, bool *old, size_t *old_index);

        pid_t currentProcess = -1;      // current running process PID
        size_t currentProcessIndex = 0; // current running process index in processes vector
    private:
        std::vector<generic_process *> *_processes; // must be a vector of pointers to be able to hold derived classes

        /* round robin-ish starting from oldProcessIndex
         * returns true if new process found, sets currentProcess and currentProcessIndex
         */
        bool reschedule(size_t oldProcessIndex);
        void remove_process(pid_t pid);
        void clearProcessesArray();
    };
}
