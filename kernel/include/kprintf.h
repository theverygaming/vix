#pragma once

#define KP_EMERG   0
#define KP_ALERT   1
#define KP_CRIT    2
#define KP_ERR     3
#define KP_WARNING 4
#define KP_NOTICE  5
#define KP_INFO    6
#define KP_DEBUG   7 // !!! this should only be used though the DEBUG_PRINTF macro in debug.h

void kprintf(int loglevel, const char *fmt, ...);
