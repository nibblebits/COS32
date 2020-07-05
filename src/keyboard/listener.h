#ifndef KEYBOARD_LISTENER
#define KEYBOARD_LISTENER

#include "keyboard.h"
/**
 * Drivers can create their own keyboard listeners to capture key presses
 */
typedef void(*KEYPRESS_FUNCTION)(char key);

typedef void(*KEYPRESS_SPECIAL_FUNCTION)(enum SpecialKeys key);
struct keyboard_listener
{
    KEYPRESS_FUNCTION keypress;
    KEYPRESS_SPECIAL_FUNCTION special;
    struct keyboard_listener* next;
};


/**
 * Initializes all the keyboard listeners
 */
void keyboard_listener_init();

/**
 * Registers a keyboard listener
 */
int keyboard_listener_register(struct keyboard_listener *listener);

/**
 * Invoked to alert all the keyboard listeners that the provided key was pressed
 */
int keyboard_listener_keypressed(char key);

/**
 * Invoked to alert all the keyboard  listeners that a special key was pressed or released
 */
int keyboard_listener_keyspecial(enum SpecialKeys key);
#endif