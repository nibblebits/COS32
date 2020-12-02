#include "fkeylistener.h"
#include "keyboard/listener.h"
#include "keyboard/keyboard.h"
#include "task/process.h"
#include "io/io.h"
#include "idt/idt.h"
#include "config.h"
#include "status.h"
#include "kernel.h"

void fkeylistener_keypress(char c);
void fkeylistener_special(enum SpecialKeys key);

static struct keyboard_listener listener = {
    .keypress = fkeylistener_keypress,
    .special = fkeylistener_special};

void fkeylistener_init()
{
    keyboard_listener_register(&listener);
}

void fkeylistener_keypress(__attribute__((unused)) char c)
{
    // We don't handle normal keypresses
}

void fkeylistener_special(enum SpecialKeys key)
{
    bool is_released = !keyboard_is_special_on(key);
    // We only care about function keys
    if (key < F1_PRESSED_OR_RELEASED || key > F12_PRESSED_OR_RELEASED)
    {
        return;
    }

    struct process *process = 0;
    int res = 0;
    if (is_released)
    {
        res = process_load_for_slot("0:/bin/shell.e", &process, key, 0, 0);
        if (res == -EISTKN)
        {
            process = process_get(key);
            process_switch(process);
            return;
        }

        if (res < 0)
        {
            print("Fatal error loading the process\n");
            return;
        }
        process_start(process);
    }
}