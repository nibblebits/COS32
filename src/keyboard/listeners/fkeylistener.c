#include "fkeylistener.h"
#include "keyboard/listener.h"
#include "keyboard/keyboard.h"
#include "task/process.h"
#include "io/io.h"
#include "memory/idt/idt.h"
#include "config.h"
#include "status.h"
#include "kernel.h"

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
        res = process_load_for_slot("0:/start.r", &process, key);
        if (res == -EISTKN)
        {
            process = process_get(key);
            print("Switching to process\n");

            // Critical operation
            disable_interrupts();
            process_switch(process);
            // Interrupts will be re-enabled after task_return
           // Acknowledge the interrupt
            outb(PIC1, PIC_EOI);
            // Now we have switched the process let's execute where we left off. Multitasking/Task switching :D
            task_return(&process->task->registers);
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