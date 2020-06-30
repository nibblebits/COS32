#include "keyboard.h"
#include "io/io.h"
#include "status.h"
#include "config.h"
#include "classic.h"
#include "task/process.h"
static struct keyboard *keyboard_list_head = 0;
static struct keyboard *keyboard_list_last = 0;

bool keyboard_is_function_key(int key)
{
    return key >= F1_PRESSED && key <= F12_PRESSED;
}

void keyboard_init()
{
    // Initialize the classic old style keyboard driver
    keyboard_insert(classic_init());
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

void keyboard_push(char c)
{
    struct process *process = process_current();
    // Will wrap around, the power of remainder ;)
    int real_index = process->keyboard.tail % sizeof(process->keyboard.buffer);
    process->keyboard.buffer[real_index] = c;
    process->keyboard.tail++;
}

char keyboard_pop()
{
    // Will wrap around, the power of remainder ;)
    struct process *process = process_current();
    int real_index = process->keyboard.head % sizeof(process->keyboard.buffer);
    char c = process->keyboard.buffer[real_index];
    process->keyboard.buffer[real_index] = 0;
    process->keyboard.head++;
    return c;
}