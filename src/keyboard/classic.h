#ifndef CLASSIC_H
#define CLASSIC_H

#include "keyboard.h"

/**
 * Cant remember the name of the classical keyboards, so our driver will just be called classic
 */



#define ISR_KEYBOARD_INTERRUPT 0x21

struct keyboard* classic_init();

#endif