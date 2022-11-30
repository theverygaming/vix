#pragma once

#define MODULE_AUTHOR(value) __attribute__((section(".moduleinfo"), used)) const char *__MODULE_AUTHOR = value
#define MODULE_DESCRIPTION(value) __attribute__((section(".moduleinfo"), used)) const char *__MODULE_DESCRIPTION = value
#define MODULE_VERSION(value) __attribute__((section(".moduleinfo"), used)) const char *__MODULE_VERSION = value
#define MODULE_INIT(func) \
    extern "C" int __MODULE_INIT() { return func(); }
#define MODULE_EXIT(func) \
    extern "C" void __MODULE_EXIT() { func(); }
