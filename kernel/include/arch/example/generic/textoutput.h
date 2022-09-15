#pragma once

// if this architecture has color
#define ARCH_FEATURE_TEXTOUTPUT_COLOR

namespace arch::generic::textoutput {
    enum class color { COLOR_BLACK, COLOR_WHITE, COLOR_RED, COLOR_LIGHT_RED, COLOR_GREEN, COLOR_LIGHT_GREEN, COLOR_BLUE, COLOR_LIGHT_BLUE };

    void puts(char *str, color foreground, color background); // this function should support \n etc. - colors may be ignored
}
