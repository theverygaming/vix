#include "multitasking.h"
#include "stdio.h"
#include "stdlib.h"
#include "cpubasics.h"
#include "memalloc.h"

multitasking::context *current_context = (multitasking::context*)(KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET);

multitasking::process processes[MAX_PROCESSES]; 

int currentProcess = 0;
int processCounter = 0;
bool processSwitchingEnabled = false;

bool multitasking::isProcessSwitchingEnabled() {
    return processSwitchingEnabled;
}

void init_empty_stack(void* stackadr, void* codeadr) {
    char argstr[] = "/bin/busybox\0sh"; // size: 16 bytes
    memcpy((char*)stackadr + (4 * 10) + sizeof(argstr), argstr, sizeof(argstr));
    
    uint32_t* stack = (uint32_t*)stackadr;

    stack[0] = (uint32_t)codeadr; // EIP
    stack[1] = 8; // CS?
    stack[2] = 1 << 9; // EFLAGS, set interrupt bit
    stack[3] = 2; // argc

    stack[4] = ((uint32_t)stackadr) + (4 * 5); // argv pointer
    stack[5] = ((uint32_t)stackadr) + (4 * 10) + sizeof(argstr); // argv string pointer
    stack[6] = ((uint32_t)stackadr) + (4 * 10) + sizeof(argstr)+13; // argv string pointer
    stack[7] = 0; // argv null termination
    stack[8] = 0; // envp null termination
    stack[9] = 0; // envp null termination
}

bool init = true;

multitasking::process* multitasking::getCurrentProcess() {
    return &processes[currentProcess];
}

multitasking::process* multitasking::fork_current_process() {
    int freeProcess = -1;
    for(int i = 0; i < MAX_PROCESSES; i++) {
        if(!processes[i].running) {
            freeProcess = i;
            break;
        }
    }
    if(freeProcess == -1) { return nullptr; }

    for(int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if(processes[currentProcess].pages[i].pages != 0) {
            void* physadr = memalloc::page::phys_malloc(processes[currentProcess].pages[i].pages);
            processes[freeProcess].pages[i].pages = processes[currentProcess].pages[i].pages;
            processes[freeProcess].pages[i].phys_base = (uint32_t)physadr;
            processes[freeProcess].pages[i].virt_base = processes[currentProcess].pages[i].virt_base;
            for(int j = 0; j < processes[freeProcess].pages[i].pages; j++) {
                paging::copyPhysPage((void*)processes[freeProcess].pages[i].phys_base + (j * 0x1000), (void*)processes[currentProcess].pages[i].phys_base + (j * 0x1000));
            }
        }
        else {
            processes[freeProcess].pages[i] = {0, 0, 0};
        }
    }
    processes[freeProcess].priority = 0;
    processes[freeProcess].pid = freeProcess;
    memcpy((char*)&processes[freeProcess].registerContext, (char*)current_context, sizeof(multitasking::context)); // gotta be very careful here to get the current context. The context in the process array is outdated while it is running.
    processes[freeProcess].running = true;
    printf("forked -> new PID: %u\n", processes[freeProcess].pid);
    return &processes[freeProcess];
}

void multitasking::killCurrentProcess() {
    processes[currentProcess].running = false;
    printf("Killed PID %u\n", processes[currentProcess].pid);
    interruptTrigger();
}

void multitasking::create_task(void* stackadr, void* codeadr, process_pagerange* pagerange) {
    stackadr -= (4 * 13); // init_empty_stack has to build the stack up
    init_empty_stack(stackadr, codeadr);
    for(uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if(!processes[i].running) {
            processes[i] = {i, {0, 0, 0, 0, 0, 0, (uint32_t)stackadr, 0}, 0, false};
            memcpy((char*)processes[i].pages, (char*)pagerange, sizeof(process_pagerange) * PROCESS_MAX_PAGE_RANGES);
            processes[i].running = true;
            processes[i].priority = 100;
            break;
        }
    }
}

void multitasking::setProcessSwitching(bool state) {
    processSwitchingEnabled = state;
}

void multitasking::interruptTrigger() {
    if(init) {
        // set all process space to zero
        for(int i = 0; i < MAX_PROCESSES; i++) {
            processes[i] = {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, false};
            for(int j = 0; j < PROCESS_MAX_PAGE_RANGES; j++) {
                processes[i].pages[j] = {0, 0, 0};
            }
        }
        processes[0] = {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, true};
        memcpy((char*)&processes[0].registerContext, (char*)current_context, sizeof(context));
        createPageRange(processes[0].pages);
        currentProcess = 0;
        init = false;
        printf("---Multitasking enabled---\n");
        processSwitchingEnabled = true;
    }

    
    int runningProcesses = 0;
    for(int i = 0; i < MAX_PROCESSES; i++) {
        if(processes[i].running) {
            runningProcesses++;
        }
    }
    if(runningProcesses == 0) {
        printf("PANIK: All processes died. Halting system\n");
        asm("hlt");
    }
    if(!processes[currentProcess].running) {
        for(int i = 0; i < MAX_PROCESSES; i++) {
                if(processes[i].running && currentProcess != i) {
                    memcpy((char*)&processes[currentProcess].registerContext, (char*)current_context, sizeof(context)); // Save current process context
                    memcpy((char*)current_context, (char*)&processes[i].registerContext, sizeof(context));
                    //createPageRange(processes[currentProcess].pages);
                    unsetPageRange(processes[currentProcess].pages);
                    setPageRange(processes[i].pages);
                    currentProcess = i;
                    break;
                }
            }
    }
    if(processCounter > processes[currentProcess].priority && processSwitchingEnabled) {
        // Priority exceeded, now we have to switch process
        if(runningProcesses > 1) { // is it even possible to switch?
            int start = currentProcess;
            if(currentProcess == runningProcesses - 1) { start = 0; }
            for(int i = start; i < MAX_PROCESSES; i++) {
                if(processes[i].running && currentProcess != i) {
                    memcpy((char*)&processes[currentProcess].registerContext, (char*)current_context, sizeof(context)); // Save current process context
                    memcpy((char*)current_context, (char*)&processes[i].registerContext, sizeof(context));
                    //createPageRange(processes[currentProcess].pages);
                    unsetPageRange(processes[currentProcess].pages);
                    setPageRange(processes[i].pages);
                    currentProcess = i;
                    break;
                }
            }
        }
        processCounter = 0;
    }
    processCounter++;
}

bool multitasking::createPageRange(process_pagerange* range, uint32_t max_address) {
    process_pagerange prange[PROCESS_MAX_PAGE_RANGES];
    for(int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        prange[i] = {0, 0, 0};
    }
    
    int prange_counter = 0;

    uint32_t physAddress = 69420;
    uint32_t lastPhysAddress = 0;
    bool invalidated = true;

    uint32_t page = 0;
    for(; page < (max_address / 4096); page++) {
        uint32_t virtadr = page * 4096;
        if(paging::is_readable((void*)virtadr)) {
            physAddress = (uint32_t)paging::get_physaddr((void*)virtadr);
            if((physAddress - 4096) != lastPhysAddress || invalidated) {
                if(prange[prange_counter].pages != 0) {
                    prange_counter++;
                    if(prange_counter >= PROCESS_MAX_PAGE_RANGES) {
                        return false;
                    }
                }
                prange[prange_counter].phys_base = physAddress;
                prange[prange_counter].virt_base = virtadr;
                invalidated = false;
            }
            prange[prange_counter].pages++;
            lastPhysAddress = physAddress;
        }
        else {
            invalidated = true;
        }
    }
    for(int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if(prange[i].pages == 0) {
            prange[i] = {0, 0, 0};
        }
    }
    memcpy((char*)range, (char*)prange, PROCESS_MAX_PAGE_RANGES * sizeof(process_pagerange));
}

void multitasking::setPageRange(process_pagerange* range) {
    for(int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if(range[i].pages > 0) {
            //printf("map range v: 0x%p p: 0x%p len: %u\n", range[i].virt_base, range[i].phys_base, range[i].pages);
            for(int j = 0; j < range[i].pages; j++) {
                paging::map_page((void*)(range[i].phys_base + (j * 0x1000)), (void*)(range[i].virt_base + (j * 0x1000)));
            }
        }
    }
}

void multitasking::unsetPageRange(process_pagerange* range) {
    for(int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if(range[i].pages > 0) {
            //printf("clearing 0x%p -> pages: %u\n", (void*)(range[i].virt_base), range[i].pages);
            paging::clearPageTables((void*)range[i].virt_base, range[i].pages);
        }
    }
}


void multitasking::zeroPageRange(process_pagerange* range) {
    for(int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        range[i] = {0,0,0};
    }
}