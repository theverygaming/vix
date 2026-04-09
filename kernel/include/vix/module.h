#pragma once

// HACK: all of these are weak to avoid multiple definition error when having multiple modules in-tree.. We need a new solution for this

#define MODULE_AUTHOR(value)      __attribute__((section(".moduleinfo"), used, weak)) const char *__MODULE_AUTHOR = value
#define MODULE_DESCRIPTION(value) __attribute__((section(".moduleinfo"), used, weak)) const char *__MODULE_DESCRIPTION = value
#define MODULE_VERSION(value)     __attribute__((section(".moduleinfo"), used, weak)) const char *__MODULE_VERSION = value
// clang-format off
#define MODULE_INIT(func) extern "C" int __attribute__((weak)) __MODULE_INIT() { return func(); }
#define MODULE_EXIT(func) extern "C" void __attribute__((weak)) __MODULE_EXIT() { func(); }
// clang-format on
