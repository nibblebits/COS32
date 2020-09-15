#include "keyboard.h"
#include "kernel.h"
#include "io/io.h"
#include "status.h"
#include "config.h"
#include "classic.h"
#include "listener.h"
#include "task/process.h"
#include "listeners/fkeylistener.h"
static struct keyboard *keyboard_list_head = 0;
static struct keyboard *keyboard_list_last = 0;

/**
* These are special keys such as CTRL, F1, F2 and so on. True means they are on, False means they are off.
* Someone might think that this should be in the process, however holding CTRL should obviously be applied toa ll processes right?
* What about caps lock?
* Maybe at least for the function keys theirs a use case for a per process basis but for now i dont think it matters,
* this feels like the correct design.
*/
static bool special_keys[KEYBOARD_TOTAL_SPECIAL_KEYS];

void keyboard_init()
{

    // Initialize the classic old style keyboard driver
    keyboard_insert(classic_init());

    // Initialize the fkey listener
    fkeylistener_init();
}

int keyboard_insert(struct keyboard *keyboard)
{
    int res = COS32_ALL_OK;
    // Keyboard drivers are  equired to have the functions implemented
    if (keyboard->init == 0)
    {
        res = -EINVARG;
        goto out;
    }

    if (keyboard_list_last)
    {
        keyboard_list_last->next = keyboard;
        keyboard_list_last = keyboard;
        goto init_keyboard;
    }

    keyboard_list_head = keyboard;
    keyboard_list_last = keyboard;

init_keyboard:
    res = keyboard->init();
out:

    return res;
}

static void keyboard_assert_special_key_valid(enum SpecialKeys c)
{
    ASSERT(c >= 0 && c < KEYBOARD_TOTAL_SPECIAL_KEYS);
}

/**
 * Turns on a special character on the keyboard such as CTRL or the function keys
 */

void keyboard_special_on(enum SpecialKeys c)
{
    keyboard_assert_special_key_valid(c);
    if (special_keys[c])
    {
        // It's already on so we don't need to do anything else here, no point alerting the listeners again either
        return;
    }

    special_keys[c] = true;

    // Let all the special keyboard listeners know
    keyboard_listener_keyspecial(c);
}

bool keyboard_is_special_on(enum SpecialKeys c)
{
    keyboard_assert_special_key_valid(c);

    return special_keys[c];
}

/**
 * Turns off a special character on the keyboard
 */
void keyboard_special_off(enum SpecialKeys c)
{
    keyboard_assert_special_key_valid(c);
    if (!special_keys[c])
    {
        // It's already off so we don't need to do anything else here, no point alerting the listeners again either
        return;
    }

    special_keys[c] = false;
    // Let all the special keyboard listeners know
    keyboard_listener_keyspecial(c);
}

static int keyboard_get_tail_index(struct process *process)
{
    return process->keyboard.tail % sizeof(process->keyboard.buffer);
}


static void keyboard_backspace(struct process* process)
{
    process->keyboard.tail -= 1;
    int real_index = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index] = 0x00;
}


void keyboard_push(char c)
{
    struct process *process = process_current();
    // We don't allow keyboard access when no process is running
    if (!process)
    {
        return;
    }


    // Will wrap around, the power of remainder ;)
    int real_index = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index] = c;
    process->keyboard.tail++;

    // Let all the keyboard listeners know about this key press
    keyboard_listener_keypressed(c);
}

char keyboard_pop()
{
    // Will wrap around, the power of remainder ;)
    struct process *process = process_current();
    // We don't allow keyboard access when no process is running
    if (!process)
    {
        return 0;
    }

    int real_index = process->keyboard.head % sizeof(process->keyboard.buffer);
    char c = process->keyboard.buffer[real_index];
    if (c == 0x00)
    {
        // Nothing to pop? Then no need to increase the head
        return 0;
    }
    process->keyboard.buffer[real_index] = 0;
    process->keyboard.head++;
    return c;
}