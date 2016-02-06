// timer.h -- Defines the interface for all PIT-related functions.
// Written for JamesM's kernel development tutorials.

#ifndef INCLUDE_TIMER_H
#define INCLUDE_TIMER_H

#include "common.h"

void init_timer(uint32_t frequency);
void timer_callback();

#endif
