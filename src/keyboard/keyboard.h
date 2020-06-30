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

#define KEYBOARD_INPUT_PORT 0x60



typedef int(*KEYBOARD_INIT_FUNCTION)();
struct keyboard
{
    
    KEYBOARD_INIT_FUNCTION init;
    char name[20];
    struct keyboard* next;
};


bool keyboard_is_function_key(int key);
int keyboard_insert(struct keyboard* keyboard);

/**
 * Pushes the character to the keyboard buffer of the current process, current implementation assumes we only have ascii
 * future implementations will need to be changed to support different character sets.
 */
void keyboard_push(char c);

/**
 * Pops off the first key in the keyboard buffer of the current process. Returned value is character equivilant of the scancode
 */
char keyboard_pop();

void keyboard_init();

#endif