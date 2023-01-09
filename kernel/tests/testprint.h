#pragma once
#include <macro.h>
#include <stdio.h>

#define TEST(group, name, condition)                                                                                            \
    do {                                                                                                                        \
        printf("--TEST--|%s|" group "|%s|" __FILE__ "|" TOSTRING(__LINE__) "|" #condition "\n", (condition) ? "K" : "F", name); \
    } while (0)
