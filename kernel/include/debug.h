#pragma once
#include <generated/autoconf.h>
#include <panic.h>
#include <stdio.h>
#include <types.h>

// clang-format off
#ifdef CONFIG_LOGLEVEL_INSANE
    #define DEBUG_PRINTF(...) printf_serial(__VA_ARGS__)
    #define DEBUG_PRINTF_INSANE(...) printf_serial(__VA_ARGS__)
    #define assertm(condition, msg) do { if(!(condition)) { KERNEL_PANIC("assertion " #condition " failed - " msg); } } while(0)
#elif CONFIG_LOGLEVEL_DEBUG
    #define DEBUG_PRINTF(...) printf_serial(__VA_ARGS__)
    #define DEBUG_PRINTF_INSANE(...) 
    #define assertm(condition, msg) do { if(!(condition)) { KERNEL_PANIC("assertion " #condition " failed - " msg); } } while(0)
#elif CONFIG_LOGLEVEL_NORMAL
    #define DEBUG_PRINTF(...)
    #define DEBUG_PRINTF_INSANE(...)
    #define assertm(condition, msg)
#elif CONFIG_LOGLEVEL_FATAL
    #define DEBUG_PRINTF(...)
    #define DEBUG_PRINTF_INSANE(...)
    #define assertm(condition, msg)
#elif CONFIG_LOGLEVEL_SILENT
    #define DEBUG_PRINTF(...)
    #define DEBUG_PRINTF_INSANE(...)
    #define assertm(condition, msg)
#endif
// clang-format on
