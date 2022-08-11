#pragma once

#include <types.h>
#include <arch/x86/cpubasics.h>


namespace drivers::serial {
    void init();
    void putc(char c);
    char getc();
}