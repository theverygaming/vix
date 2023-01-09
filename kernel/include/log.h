#pragma once
#include <debug.h>
#include <generated/autoconf.h>
#include <macro.h>

// clang-format off
#ifdef CONFIG_LOGLEVEL_INSANE
    #define LOG_INSANE(str) log::log(__FILE__ ":" TOSTRING(__LINE__) " -> " str)
    #define LOG_DEBUG(str) log::log(__FILE__ ":" TOSTRING(__LINE__) " -> " str)
    #define LOG_NORMAL(str) log::log(__FILE__ ":" TOSTRING(__LINE__) " -> " str)
    #define LOG_FATAL(str) log::log(__FILE__ ":" TOSTRING(__LINE__) " -> " str)
#elif CONFIG_LOGLEVEL_DEBUG
    #define LOG_INSANE(str)
    #define LOG_DEBUG(str) log::log(__FILE__ ":" TOSTRING(__LINE__) " -> " str)
    #define LOG_NORMAL(str) log::log(__FILE__ ":" TOSTRING(__LINE__) " -> " str)
    #define LOG_FATAL(str) log::log(__FILE__ ":" TOSTRING(__LINE__) " -> " str)
#elif CONFIG_LOGLEVEL_NORMAL
    #define LOG_INSANE(str)
    #define LOG_DEBUG(str)
    #define LOG_NORMAL(str) log::log(__FILE__ ":" TOSTRING(__LINE__) " -> " str)
    #define LOG_FATAL(str) log::log(__FILE__ ":" TOSTRING(__LINE__) " -> " str)
#elif CONFIG_LOGLEVEL_FATAL
    #define LOG_INSANE(str)
    #define LOG_DEBUG(str)
    #define LOG_NORMAL(str)
    #define LOG_FATAL(str) log::log(__FILE__ ":" TOSTRING(__LINE__) " -> " str)
#elif CONFIG_LOGLEVEL_SILENT
    #define LOG_INSANE(str)
    #define LOG_DEBUG(str)
    #define LOG_NORMAL(str)
    #define LOG_FATAL(str)
#endif
// clang-format on

namespace log {
    void log(const char *str);
    void log_service(const char *servicename, const char *str, bool issue = false);
}
