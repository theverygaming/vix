#include <gdt.h>
#include <paging.h>

extern "C" void puts_c(const char *str);
extern "C" void startup_cpp() {
    puts_c("loaded C++\n\r");
    paging::initpaging();
    puts_c("paging init\n\r");
    gdt::i686_GDT_Initialize();
    puts_c("GDT init\n\r");
}