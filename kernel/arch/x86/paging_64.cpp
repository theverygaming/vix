#include <vix/arch/generic/memory.h>
#include <vix/arch/paging.h>
#include <vix/config.h>
#include <vix/debug.h>
#include <vix/stdio.h>
#include <vix/types.h>

#define AVAILABLE       (1 << 0)
#define WRITE           (1 << 1)
#define SUPERVISOR      (1 << 2)
#define WRITE_THROUGH   (1 << 3)
#define CACHE_DISABLE   (1 << 4)
#define EXECUTE_DISABLE (1 << 63)

#define GLOBAL (1 << 8)

void paging::init() {}
