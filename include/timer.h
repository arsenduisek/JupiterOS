#ifndef JUPITER_TIMER_H
#define JUPITER_TIMER_H

#include "types.h"

void     timer_init(uint32_t hz);
uint32_t timer_ticks(void);

#endif
