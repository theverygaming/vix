#include <arch/x86/cpubasics.h>
#include <arch/x86/multitasking.h>
#include <cppstd/vector.h>
#include <log.h>
#include <memory_alloc/memalloc.h>
#include <stdio.h>
#include <stdlib.h>

multitasking::context *current_context = (multitasking::context *)(KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET);

multitasking::process processes[MAX_PROCESSES];

static int currentProcess = 0;
static int processTimeShareCounter = 0;
static bool processSwitchingEnabled = false;

static bool uninitialized = true;

void multitasking::initMultitasking() {
    // set all process space to zero
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i] = {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, false};
        for (int j = 0; j < PROCESS_MAX_PAGE_RANGES; j++) {
            processes[i].pages[j] = {0, 0, 0};
        }
    }
    processes[0] = {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, true};
    memcpy((char *)&processes[0].registerContext, (char *)current_context, sizeof(context));
    createPageRange(processes[0].pages);
    currentProcess = 0;
    log::log_service("multitasking", "enabled");
    processSwitchingEnabled = true;
    uninitialized = false;
}

bool multitasking::isProcessSwitchingEnabled() {
    return processSwitchingEnabled;
}

/* this function gets passed the top of stack, argv[] must be terminated with a null pointer | returns new stack pointer */
static void *init_empty_stack(void *stackadr, void *codeadr, std::vector<char *> *argv) {
    // get argc
    int argc = argv->size();

    if (argc < 1) {
        LOG_DEBUG("issue: argc too small");
    }

    size_t total_size = 6 * 4; // eip, cs, eflags, argc, argv null termination and envp null termination

    for (int i = 0; i < argc; i++) {
        total_size += strlen((*argv)[i]) + 1;
        total_size += 4; // pointer to arg
    }
    stackadr = ((char *)stackadr) - total_size;

    uint32_t *stack = (uint32_t *)stackadr;

    stack[0] = (uint32_t)codeadr; // EIP
    stack[1] = 8;                 // CS?
    stack[2] = 1 << 9;            // EFLAGS, set interrupt bit
    stack[3] = argc;              // argc

    int nextindex = 4;
    size_t string_pos = (6 * 4) + (argc * 4);

    // build up argv
    for (int i = 0; i < argc; i++) {
        memcpy((char *)stackadr + string_pos, (*argv)[i], strlen((*argv)[i]) + 1);
        stack[nextindex++] = (size_t)stackadr + string_pos;
        string_pos += strlen((*argv)[i]) + 1;
    }
    stack[nextindex++] = 0; // argv null termination
    stack[nextindex++] = 0; // envp null termination

    return stackadr;
}

multitasking::process *multitasking::getCurrentProcess() {
    return &processes[currentProcess];
}

void multitasking::waitForProcess(int pid) {
    if (pid < MAX_PROCESSES) {
        while (processes[pid].running) {}
    }
}

void multitasking::refresh_current_process_pagerange() {
    createPageRange(processes[currentProcess].pages);
}

multitasking::process *multitasking::fork_current_process() {
    int freeProcess = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i].running) {
            freeProcess = i;
            break;
        }
    }

    if (freeProcess == -1) {
        return nullptr;
    }

    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if (processes[currentProcess].pages[i].pages != 0) {
            void *physadr = memalloc::page::phys_malloc(processes[currentProcess].pages[i].pages);
            if (!physadr) {
                printf("could not allocate memory for fork!\n");
                return nullptr;
            }
            processes[freeProcess].pages[i].pages = processes[currentProcess].pages[i].pages;
            processes[freeProcess].pages[i].phys_base = (uint32_t)physadr;
            processes[freeProcess].pages[i].virt_base = processes[currentProcess].pages[i].virt_base;
            for (uint32_t j = 0; j < processes[freeProcess].pages[i].pages; j++) {
                paging::copyPhysPage((void *)processes[freeProcess].pages[i].phys_base + (j * 0x1000), (void *)processes[currentProcess].pages[i].phys_base + (j * 0x1000));
            }
        } else {
            processes[freeProcess].pages[i] = {0, 0, 0};
        }
    }
    processes[freeProcess].priority = 0;
    processes[freeProcess].pid = freeProcess;
    memcpy((char *)&processes[freeProcess].registerContext,
           (char *)current_context,
           sizeof(multitasking::context)); // gotta be very careful here to get the current context. The context in the process array is outdated while it is running.
    processes[freeProcess].running = true;
    DEBUG_PRINTF("forked -> new PID: %u\n", processes[freeProcess].pid);
    return &processes[freeProcess];
}

void multitasking::killCurrentProcess() {
    processes[currentProcess].running = false;
    DEBUG_PRINTF("Killed PID %u\n", processes[currentProcess].pid);
    freePageRange(processes[currentProcess].pages);
    interruptTrigger();
}

void multitasking::create_task(void *stackadr, void *codeadr, process_pagerange *pagerange, std::vector<char *> *argv) {
    process_pagerange old_pageranges[PROCESS_MAX_PAGE_RANGES];
    createPageRange(old_pageranges);

    setPageRange(pagerange);
    stackadr = init_empty_stack(stackadr, codeadr, argv);
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i].running) {
            processes[i] = {i, {0, 0, 0, 0, 0, 0, (uint32_t)stackadr, 0}, 0, false};
            memcpy((char *)processes[i].pages, (char *)pagerange, sizeof(process_pagerange) * PROCESS_MAX_PAGE_RANGES);
            processes[i].running = true;
            processes[i].priority = 0;
            break;
        }
    }
    unsetPageRange(pagerange);

    setPageRange(old_pageranges);
}

void multitasking::replace_task(void *stackadr, void *codeadr, process_pagerange *pagerange, std::vector<char *> *argv, int replacePid) {
    /* kill old process */
    processes[replacePid].running = false;
    freePageRange(processes[replacePid].pages);
    interruptTrigger();

    process_pagerange old_pageranges[PROCESS_MAX_PAGE_RANGES];
    createPageRange(old_pageranges);

    setPageRange(pagerange);
    stackadr = init_empty_stack(stackadr, codeadr, argv);
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i].running) {
            processes[i] = {i, {0, 0, 0, 0, 0, 0, (uint32_t)stackadr, 0}, 0, false};
            memcpy((char *)processes[i].pages, (char *)pagerange, sizeof(process_pagerange) * PROCESS_MAX_PAGE_RANGES);
            processes[i].running = true;
            processes[i].priority = 0;
            break;
        }
    }
    unsetPageRange(pagerange);

    setPageRange(old_pageranges);
}

void multitasking::setProcessSwitching(bool state) {
    processSwitchingEnabled = state;
}

void multitasking::interruptTrigger() {
    if (uninitialized) {
        return;
    }

    int runningProcesses = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].running) {
            runningProcesses++;
        }
    }
    if (runningProcesses == 0) {
        printf("PANIK: All processes died. Halting system\n");
        asm volatile("hlt");
    }
    if (!processes[currentProcess].running) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (processes[i].running && currentProcess != i) {
                memcpy((char *)&processes[currentProcess].registerContext, (char *)current_context, sizeof(context)); // Save current process context
                memcpy((char *)current_context, (char *)&processes[i].registerContext, sizeof(context));
                // createPageRange(processes[currentProcess].pages);
                unsetPageRange(processes[currentProcess].pages);
                setPageRange(processes[i].pages);
                currentProcess = i;
                break;
            }
        }
    }
    if (processTimeShareCounter > processes[currentProcess].priority && processSwitchingEnabled) {
        // Priority exceeded, now we have to switch process
        if (runningProcesses > 1) { // is it even possible to switch?
            int start = currentProcess;
            if (currentProcess == runningProcesses - 1) { // is this the last process in the processes array?
                start = 0;
            }
        bruh:
            int oldProcess = currentProcess;
            for (int i = start; i < MAX_PROCESSES; i++) {
                if (processes[i].running && currentProcess != i) {
                    memcpy((char *)&processes[currentProcess].registerContext, (char *)current_context, sizeof(context)); // Save current process context
                    memcpy((char *)current_context, (char *)&processes[i].registerContext, sizeof(context));
                    // createPageRange(processes[currentProcess].pages);
                    unsetPageRange(processes[currentProcess].pages);
                    setPageRange(processes[i].pages);
                    currentProcess = i;
                    break;
                }
            }
            if (oldProcess == currentProcess) {
                start = 0;
                goto bruh; // this will get stuck at some point, temporary fix
            }
        }
        processTimeShareCounter = 0;
    }
    processTimeShareCounter++;
}

bool multitasking::createPageRange(process_pagerange *range, uint32_t max_address) {
    process_pagerange prange[PROCESS_MAX_PAGE_RANGES];
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        prange[i] = {0, 0, 0};
    }

    int prange_counter = 0;

    uint32_t physAddress = 69420;
    uint32_t lastPhysAddress = 0;
    bool invalidated = true;

    uint32_t page = 0;
    for (; page < (max_address / 4096); page++) {
        uint32_t virtadr = page * 4096;
        if (paging::is_readable((void *)virtadr)) {
            physAddress = (uint32_t)paging::get_physaddr((void *)virtadr);
            if ((physAddress - 4096) != lastPhysAddress || invalidated) {
                if (prange[prange_counter].pages != 0) {
                    prange_counter++;
                    if (prange_counter >= PROCESS_MAX_PAGE_RANGES) {
                        return false;
                    }
                }
                prange[prange_counter].phys_base = physAddress;
                prange[prange_counter].virt_base = virtadr;
                invalidated = false;
            }
            prange[prange_counter].pages++;
            lastPhysAddress = physAddress;
        } else {
            invalidated = true;
        }
    }
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if (prange[i].pages == 0) {
            prange[i] = {0, 0, 0};
        }
    }
    memcpy((char *)range, (char *)prange, PROCESS_MAX_PAGE_RANGES * sizeof(process_pagerange));
    return true;
}

void multitasking::setPageRange(process_pagerange *range) {
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if (range[i].pages > 0) {
            // printf("map range v: 0x%p p: 0x%p len: %u\n", range[i].virt_base, range[i].phys_base, range[i].pages);
            for (uint32_t j = 0; j < range[i].pages; j++) {
                paging::map_page((void *)(range[i].phys_base + (j * 0x1000)), (void *)(range[i].virt_base + (j * 0x1000)));
            }
        }
    }
}

void multitasking::unsetPageRange(process_pagerange *range) {
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if (range[i].pages > 0) {
            // printf("clearing 0x%p -> pages: %u\n", (void*)(range[i].virt_base), range[i].pages);
            paging::clearPageTables((void *)range[i].virt_base, range[i].pages);
        }
    }
}

void multitasking::zeroPageRange(process_pagerange *range) {
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        range[i] = {0, 0, 0};
    }
}

void multitasking::freePageRange(process_pagerange *range) {
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if (range[i].pages > 0) {
            memalloc::page::phys_free((void *)range[i].phys_base);
        }
    }
}

void multitasking::printPageRange(process_pagerange *range) {
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if (range[i].pages != 0) {
            printf("pb: 0x%p vb: 0x%p pages: %u\n", range[i].phys_base, range[i].virt_base, range[i].pages);
        }
    }
}
