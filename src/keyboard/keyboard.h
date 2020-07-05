#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

enum SpecialKeys
{
    F1_PRESSED_OR_RELEASED,
    F2_PRESSED_OR_RELEASED,
    F3_PRESSED_OR_RELEASED,
    F4_PRESSED_OR_RELEASED,
    F5_PRESSED_OR_RELEASED,
    F6_PRESSED_OR_RELEASED,
    F7_PRESSED_OR_RELEASED,
    F8_PRESSED_OR_RELEASED,
    F9_PRESSED_OR_RELEASED,
    F10_PRESSED_OR_RELEASED,
    F11_PRESSED_OR_RELEASED,
    F12_PRESSED_OR_RELEASED,
    CTRL_PRESSED_OR_RELEASED,
    ALT_PRESSED_OR_RELEASED
};

#define KEYBOARD_TOTAL_SPECIAL_KEYS 14
#define KEYBOARD_INPUT_PORT 0x60

typedef int (*KEYBOARD_INIT_FUNCTION)();
struct keyboard
{

    KEYBOARD_INIT_FUNCTION init;
    char name[20];
    struct keyboard *next;
};

/**
 * Adds a new keyboard to our system
 */
int keyboard_insert(struct keyboard *keyboard);

/**
 * Pushes the character to the keyboard buffer of the current process, current implementation assumes we only have ascii
 * future implementations will need to be changed to support different character sets.
 */
void keyboard_push(char c);

/**
 * Turns on a special character on the keyboard such as CTRL or the function keys for the current process
 */

void keyboard_special_on(enum SpecialKeys c);

/**
 * Turns off a special character on the keyboard for the current process
 */
void keyboard_special_off(enum SpecialKeys c);

/**
 * Returns true if the special key is on for the current process
*/
bool keyboard_is_special_on(enum SpecialKeys c);

/**
 * Pops off the first key in the keyboard buffer of the current process. Returned value is character equivilant of the scancode
 */
char keyboard_pop();

void keyboard_init();

#endif