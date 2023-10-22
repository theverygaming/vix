#pragma once
#include <sched.h>

#define START_THREAD_ARGS(function, ...)                    \
    do {                                                    \
        sched::start_thread([] { function(__VA_ARGS__); }); \
    } while (0);
