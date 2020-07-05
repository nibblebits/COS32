#include "listener.h"
#include "status.h"
#include "listeners/fkeylistener.h"

static struct keyboard_listener *keyboard_listener_head = 0;
static struct keyboard_listener *keyboard_listener_last = 0;

void keyboard_listener_init()
{
    keyboard_listener_head = 0;
    keyboard_listener_last = 0;
    fkeylistener_init();
}

int keyboard_listener_register(struct keyboard_listener *listener)
{
    int res = COS32_ALL_OK;
    if (keyboard_listener_last)
    {
        keyboard_listener_last->next = listener;
        keyboard_listener_last = listener;
        goto out;
    }

    keyboard_listener_head = listener;
    keyboard_listener_last = listener;

out:
    return res;
}

int keyboard_listener_keypressed(char key)
{
    int res = COS32_ALL_OK;
    struct keyboard_listener *current = keyboard_listener_head;
    while (current)
    {
        // Not all keyboard listeners want to implement a keypress
        if (!current->keypress)
        {
            continue;
        }
        current->keypress(key);
        current = current->next;
    }

    return res;
}

int keyboard_listener_keyspecial(enum SpecialKeys key)
{
    int res = COS32_ALL_OK;
    struct keyboard_listener *current = keyboard_listener_head;
    while (current)
    {
        // Not all keyboard listeners want to implement a special key listener
        if (!current->special)
        {
            continue;
        }
        current->special(key);
        current = current->next;
    }

    return res;
}