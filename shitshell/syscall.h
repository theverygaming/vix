#pragma once
#include "types.h"

typedef int pid_t;

void sys_write(uint32_t fd, const char* buf, size_t count);
uint32_t sys_read(uint32_t fd, char* buf, size_t count);

