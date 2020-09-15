#ifndef PIT_H
#define PIT_H

#include <stdint.h>
#include "idt/idt.h"

// The average miliseconds the PIT timer takes to interrupt us
#define PIT_TIMER_AVERAGE_MS 55

void pit_init();
void pit_interrupt(int interrupt);

/**
 * Returns the total miliseconds since the PIT timer has been interrupting us
 */
long pit_get_millis();

#endif