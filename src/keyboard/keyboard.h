#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

#define F1_PRESSED 0x3B
#define F2_PRESSED 0x3C
#define F3_PRESSED 0x3D
#define F4_PRESSED  0x3E
#define F5_PRESSED 0x3F
#define F6_PRESSED 0x40
#define F7_PRESSED 0x41
#define F8_PRESSED 0x42
#define F9_PRESSED 0x43
#define F10_PRESSED 0x44
#define F11_PRESSED 0x45
#define F12_PRESSED 0x46
void enable_keyboard();

bool keyboard_is_function_key(int key);

#endif