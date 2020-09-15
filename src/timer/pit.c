
#include "pit.h"
#include "task/task.h"
#include "io/io.h"
static long ticks_since_initialized = 0;

void pit_init()
{
    idt_register_interrupt_callback(ISR_TIMER_INTERRUPT, pit_interrupt);
}

void pit_interrupt(int interrupt)
{
    ticks_since_initialized += PIT_TIMER_AVERAGE_MS;

    // Let the task mechnism process some important things
    task_process();

    // Acknowledge the interrupt
    outb(PIC1, PIC_EOI);
    task_next();
}

/**
 * Returns the total miliseconds since the PIT timer has been interrupting us
 */
long pit_get_millis()
{
    return ticks_since_initialized;
}