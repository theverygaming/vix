#pragma once

#define likely(cond)   __builtin_expect((cond), 1)
#define unlikely(cond) __builtin_expect((cond), 0)
