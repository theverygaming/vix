#include <vix/abi/linux/calls.h>
#include <vix/config.h>
#include <vix/kprintf.h>
#include <vix/mm/memmap.h>
#include <vix/mm/pmm.h>
#include <vix/sched.h>
#include <vix/time.h>

struct sysinfo {
    long uptime;             /* Seconds since boot */
    unsigned long loads[3];  /* 1, 5, and 15 minute load averages */
    unsigned long totalram;  /* Total usable main memory size */
    unsigned long freeram;   /* Available memory size */
    unsigned long sharedram; /* Amount of shared memory */
    unsigned long bufferram; /* Memory used by buffers */
    unsigned long totalswap; /* Total swap space size */
    unsigned long freeswap;  /* Swap space still available */
    unsigned short procs;    /* Number of current processes */
    unsigned long totalhigh; /* Total high memory size */
    unsigned long freehigh;  /* Available high memory size */
    unsigned int mem_unit;   /* Memory unit size in bytes */
    char _f[20 - 2 * sizeof(long) - sizeof(int)];
    /* Padding to 64 bytes */
};

__DEF_LINUX_SYSCALL(sys_sysinfo) {
    struct sysinfo *info = (struct sysinfo *)sysarg0;

    // TODO: return EFUALT on invalid address
    // FIXME: we need something like copy_to_user

    info->uptime = time::getUptimeSeconds();
    info->loads[0] = 0; //  1-minute load average
    info->loads[1] = 0; //  5-minute load average
    info->loads[2] = 0; // 15-minute load average
    info->totalram = mm::mem_map_get_total_usable_bytes() / CONFIG_ARCH_PAGE_SIZE;
    info->freeram = mm::pmm::get_free_blocks();
    info->sharedram = 0;
    info->bufferram = 0;
    info->totalswap = 0;
    info->freeswap = 0;
    info->procs = 0;
    info->totalhigh = 0;
    info->freehigh = 0;
    info->mem_unit = CONFIG_ARCH_PAGE_SIZE;
    return 0;
}
