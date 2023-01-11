#pragma once
#include <types.h>

// Mailbox channels
#define MBOX_CH_PROPERTY 8

bool mbox_call(int channel, volatile void *buf);
