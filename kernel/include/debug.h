#pragma once
#include <generated/config.h>
#include <panic.h>
#include <stdio.h>
#include <types.h>

#if CONFIG_LOGLEVEL == 'I'
    #define DEBUG_PRINTF(...) printf_serial(__VA_ARGS__)
    #define DEBUG_PRINTF_INSANE(...) printf_serial(__VA_ARGS__)
    #define DEBUG_BREAKPOINT debug::breakpoint(__LINE__, __FILE__)
    #define assertm(condition, msg) do { if(!(condition)) { KERNEL_PANIC("assertion " #condition " failed - " msg); } } while(0)
#elif CONFIG_LOGLEVEL == 'D'
    #define DEBUG_PRINTF(...) printf_serial(__VA_ARGS__)
    #define DEBUG_PRINTF_INSANE(...) 
    #define DEBUG_BREAKPOINT debug::breakpoint(__LINE__, __FILE__)
    #define assertm(condition, msg) do { if(!(condition)) { KERNEL_PANIC("assertion " #condition " failed - " msg); } } while(0)
#elif CONFIG_LOGLEVEL == 'N'
    #define DEBUG_PRINTF(...)
    #define DEBUG_PRINTF_INSANE(...)
    #define DEBUG_BREAKPOINT
    #define assertm(condition, msg)
#else
    #define DEBUG_PRINTF(...)
    #define DEBUG_PRINTF_INSANE(...)
    #define DEBUG_BREAKPOINT
    #define assertm(condition, msg)
#endif


namespace debug {
    void debug_loop();
    void breakpoint(int line, char *file);
}
