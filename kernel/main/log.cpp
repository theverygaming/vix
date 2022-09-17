#include <arch/arch.h>
#include <log.h>
#include INCLUDE_ARCH_GENERIC(textoutput.h)

void log::log(const char *str) {
    printf("%s\n", str);
}

void log::log_service(const char *servicename, const char *str, bool issue) {
    // well this is stupid... arch::generic::textoutput should have a printf...
    arch::generic::textoutput::color color = issue ? arch::generic::textoutput::color::COLOR_RED : arch::generic::textoutput::color::COLOR_GREEN;
    arch::generic::textoutput::puts("[", arch::generic::textoutput::color::COLOR_GREY, arch::generic::textoutput::color::COLOR_BLACK);
    arch::generic::textoutput::puts(servicename, color, arch::generic::textoutput::color::COLOR_BLACK);
    arch::generic::textoutput::puts("] ", arch::generic::textoutput::color::COLOR_GREY, arch::generic::textoutput::color::COLOR_BLACK);
    arch::generic::textoutput::puts(str, arch::generic::textoutput::color::COLOR_GREY, arch::generic::textoutput::color::COLOR_BLACK);
    arch::generic::textoutput::puts("\n", arch::generic::textoutput::color::COLOR_GREY, arch::generic::textoutput::color::COLOR_BLACK);
}
