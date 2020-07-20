#include "fkeylistener.h"
#include "keyboard/listener.h"
#include "keyboard/keyboard.h"
#include "task/process.h"
#include "status.h"
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
    // We only care about function keys
    if (key < F1_PRESSED_OR_RELEASED || key > F12_PRESSED_OR_RELEASED)
    {
        return;
    }

    struct process* process = 0;
    int res = 0;
    if (is_down)
    {
        print("Starting new process\n");
        res = process_load_for_slot("0:/start.r", &process, key);
        if (res == -EISTKN)
        {
            process = process_get(key);
            print("Switching to process\n");
            process_switch(process);
            //user_mode_enter()
            return;
        }

        if (res < 0)
        {
            print("Fatal error loading the process\n");
            return;
        }

        print("Starting process\n");
        process_start(process);
    }
   
}