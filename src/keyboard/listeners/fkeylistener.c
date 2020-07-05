#include "fkeylistener.h"
#include "keyboard/listener.h"
#include "keyboard/keyboard.h"
void fkeylistener_keypress(char c);
void fkeylistener_special(enum SpecialKeys key);

struct keyboard_listener listener = {
    .keypress = fkeylistener_keypress,
    .special = fkeylistener_special};

void fkeylistener_init()
{
    keyboard_listener_register(&listener);
}

void fkeylistener_keypress(char c)
{
    // We don't handle normal keypresses
}

void fkeylistener_special(enum SpecialKeys key)
{
    bool is_down = keyboard_is_special_on(key);
    if (key == F1_PRESSED_OR_RELEASED)
    {
        if(is_down)
        {
            print("F1 Pressed\n");
        } else {
            print("F1 Released\n");
        }
    }
}